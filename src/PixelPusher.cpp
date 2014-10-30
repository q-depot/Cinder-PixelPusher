/*
 *  PixelPusher.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2014 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "cinder/Utilities.h"
#include "PixelPusher.h"
#include "Strip.h"
#include "PusherDiscoveryService.h"

using namespace ci;
using namespace ci::app;
using namespace std;


PixelPusher::PixelPusher( DeviceHeader header ) : mDeviceHeader(header)
{
    mArtnetUniverse   	= 0;
    mArtnetChannel    	= 0;
    mPort               = 9798;
    mStripsAttached     = 0;
    mPixelsPerStrip     = 0;
    mExtraDelayMsec     = 0;
    mMulticast          = false;
    mMulticastPrimary   = false;
    mSegments           = 0;
    mPowerDomain        = 0;
    mLastPingAt         = getElapsedSeconds();
    mResetSentAt        = getElapsedSeconds();
    
    setPusherFlags(0);
    
    uint32_t swRev      = header.getSoftwareRevision();
    
    if ( swRev < ACCEPTABLE_LOWEST_SW_REV )
    {
        console() << "WARNING!  This PixelPusher Library requires firmware revision " + to_string( ACCEPTABLE_LOWEST_SW_REV / 100.0 ) << endl;
        console() << "WARNING!  This PixelPusher is using " << to_string( swRev / 100.0 ) << endl;
        console() << "WARNING!  This is not expected to work.  Please update your PixelPusher." << endl;
    }
    
    shared_ptr<uint8_t>    packet      = mDeviceHeader.getPacketReminder();
    int                         packetSize  = mDeviceHeader.getPacketReminderSize();
    
    if ( packetSize < 28 )
        throw invalid_argument( "Packet size < 28" );
    
    memcpy( &mStripsAttached,       &packet.get()[0],   1 );
    memcpy( &mMaxStripsPerPacket,   &packet.get()[1],   1 );
    memcpy( &mPixelsPerStrip,       &packet.get()[2],   2 );
    memcpy( &mUpdatePeriod,         &packet.get()[4],   4 );
    memcpy( &mPowerTotal,           &packet.get()[8],   4 );
    memcpy( &mDeltaSequence,        &packet.get()[12],  4 );
    memcpy( &mControllerId,         &packet.get()[16],  4 );
    memcpy( &mGroupId,              &packet.get()[20],  4 );
    memcpy( &mArtnetUniverse,       &packet.get()[24],  2 );
    memcpy( &mArtnetChannel,        &packet.get()[26],  2 );
    
    if ( packetSize > 28 && swRev > 100)
        memcpy( &mPort, &packet.get()[28],    2 );
    else
        mPort = 9798;
    
    // A minor complication here.  The PixelPusher firmware generates announce packets from
    // a static structure, so the size of stripFlags is always 8;  even if there are fewer
    // strips configured.  So we have a wart. - jls.

    int stripFlagSize = 8;
    if ( mStripsAttached > 8 )
        stripFlagSize = mStripsAttached;
    
    mStripFlags.resize( stripFlagSize );
    
    if ( packetSize > 30 && swRev > 108 )
        memcpy( &mStripFlags[0], &packet.get()[30], stripFlagSize );

    else
        for (int i=0; i<stripFlagSize; i++)
            mStripFlags[i] = 0;
    
    if ( packetSize > 30 + stripFlagSize && swRev > 116 )
    {
        // set Pusher flags
        uint32_t pusherFlags;
        memcpy( &pusherFlags,   &packet.get()[32+stripFlagSize], 4 );
        setPusherFlags( pusherFlags );
        
        memcpy( &mSegments,     &packet.get()[36+stripFlagSize], 4 );
        memcpy( &mPowerDomain,  &packet.get()[40+stripFlagSize], 4 );
    }
    
    createStrips();
}


PixelPusher::~PixelPusher()
{
    if (  mSendDataThread.get_id() != thread::id() )
    {
        for( size_t k=0; k < mStrips.size(); k++ )
            mStrips[k]->setPixelsBlack();
        
        reset();
        
        destroyCardThread();
    }
}


vector<StripRef> PixelPusher::getStrips()
{
    // Devices that are members of a multicast group,
    // but which are not the primary member of that group,
    // do not return strips.
    if ( mMulticast && !mMulticastPrimary )
        return vector<StripRef>();

    return mStrips;
    // Ensure callers can't modify the returned list
    // return Collections.unmodifiableList(strips);
}


StripRef PixelPusher::getStrip( int stripNumber )
{
    if ( stripNumber > mStripsAttached )
        return StripRef();

    return mStrips[stripNumber];
}


void PixelPusher::updateVariables( PixelPusherRef device )
{
    mDeltaSequence      = device->mDeltaSequence;
    mMaxStripsPerPacket = device->mMaxStripsPerPacket;
    mPowerTotal         = device->mPowerTotal;
    mUpdatePeriod       = device->mUpdatePeriod;
}
  

void PixelPusher::copyHeader( PixelPusherRef device )
{
    mControllerId       = device->mControllerId;
    mDeltaSequence      = device->mDeltaSequence;
    mGroupId            = device->mGroupId;
    mMaxStripsPerPacket = device->mMaxStripsPerPacket;
    mPowerTotal         = device->mPowerTotal;
    mUpdatePeriod       = device->mUpdatePeriod;
    mArtnetChannel      = device->mArtnetChannel;
    mArtnetUniverse     = device->mArtnetUniverse;
    mPort               = device->mPort;
    mPowerDomain        = device->mPowerDomain;
    
    setPusherFlags( device->getPusherFlags() );
    
    // if the number of strips we have doesn't match,
    // we'll need to make a fresh set.
    if ( mStripsAttached != device->mStripsAttached )
    {
        mStrips.clear();
        mStripsAttached = device->mStripsAttached;
    }
    
    // likewise, if the length of each strip differs,
    // we will need to make a new set.
    if ( mPixelsPerStrip != device->mPixelsPerStrip )
    {
        mStrips.clear();
        mPixelsPerStrip = device->mPixelsPerStrip;
    }
    
    // and it's the same for segments
    if ( mSegments != device->mSegments )
    {
        mStrips.clear();
        mSegments = device->mSegments;
    }
}


vector<StripRef> PixelPusher::getTouchedStrips()
{
    vector<StripRef> touchedStrips;
    
    for( size_t k=0; k < mStrips.size(); k++ )
        if ( mStrips[k]->isTouched() )
            touchedStrips.push_back( mStrips[k] );
    
      return touchedStrips;
}


bool PixelPusher::hasRGBOW()
{
    if ( mStrips.empty() )
        return false;
    
    for( size_t k=0; k < mStrips.size(); k++ )
        if ( mStrips[k]->getRGBOW() )
            return true;
    
    return false;
}


string PixelPusher::formattedStripFlags()
{
    string s;
    
    for( int i = 0; i < mStripsAttached; i++ )
        s += "[" + to_string(mStripFlags[i]) + "]";
    
    return s;
}


void PixelPusher::createStrips()
{
    for( uint8_t k = 0; k < mStripsAttached; k++ )
    {
        StripRef strip = Strip::create( k, mPixelsPerStrip );

        if ( ( mStripFlags[k] & SFLAG_MOTION ) != 0 )
            strip->setMotion(true);
        else
            strip->setMotion(false);
        
        if ( ( mStripFlags[k] & SFLAG_NOTIDEMPOTENT ) != 0 )
            strip->setNotIdempotent(true);
        else
            strip->setNotIdempotent(false);
        
        strip->setRGBOW( ( mStripFlags[k] & SFLAG_RGBOW ) == 1 );
        
        strip->setPixelsBlack();
        
        mStrips.push_back( strip );
    }
}


bool PixelPusher::isEqual( PixelPusherRef otherDevice )
{
    // if it differs by less than half a msec, it has no effect on our timing
    int updatePeriod = getUpdatePeriod() - otherDevice->getUpdatePeriod();     // update period is uint32_t
    if ( abs( updatePeriod ) > 500 )
        return false;
    
    // some fudging to cope with the fact that pushers don't know they have RGBOW
    if ( ( hasRGBOW() & !otherDevice->hasRGBOW() ) && ( getPixelsPerStrip() != otherDevice->getPixelsPerStrip() / 3 ) )
            return false;
    
    if ( ( !hasRGBOW() & otherDevice->hasRGBOW() ) && ( getPixelsPerStrip() / 3 != otherDevice->getPixelsPerStrip() ) )
            return false;
    
    if ( ! ( hasRGBOW() || otherDevice->hasRGBOW() ) )
        if (getPixelsPerStrip() != otherDevice->getPixelsPerStrip())
            return false;
    
    if ( getNumStrips() != otherDevice->getNumStrips() )
        return false;
    
    // handle the case where someone changed the config during library runtime
    if ( mArtnetChannel != otherDevice->getArtnetChannel() || mArtnetUniverse != otherDevice->getArtnetUniverse() )
        return false;
    
    // if the port's been changed, we need to update
    if ( mPort != otherDevice->getPort() )
        return false;
    
    // we should update every time the power total changes significantly
    int powerTotal = mPowerTotal - otherDevice->getPowerTotal();     // power total is uint32_t
    if ( abs( powerTotal ) > 10000 )
        return false;
    
    // handle the case where our power domain changed
    if ( mPowerDomain != otherDevice->getPowerDomain() )
        return false;
    
    // ditto for number of segments and pusherFlags
    if ( mSegments != otherDevice->getSegments() )
        return false;
    
    if ( getPusherFlags() != otherDevice->getPusherFlags() )
        return false;
    
    return true;
}


bool PixelPusher::hasTouchedStrips()
{
    for( size_t k=0; k < mStrips.size(); k++ )
        if ( mStrips[k]->isTouched() )
            return true;

    return false;
}


// Thread stuff
void PixelPusher::createCardThread( boost::asio::io_service& ioService )
{
    mThreadExtraDelayMsec   = 0;
    mPacketNumber           = 0;
    
    mClient = UdpClient::create( ioService );
    mClient->connectConnectEventHandler( &PixelPusher::onConnect, this );
    mClient->connectErrorEventHandler( &PixelPusher::onError, this );
    mClient->connect( getIp(), getPort() );
    
    mSendDataThread = thread( &PixelPusher::sendPacketToPusher, this );
}


void PixelPusher::destroyCardThread()
{
    // terminate in 100ms to ensure it sends out the black pixels
    mTerminateThreadAt = getElapsedSeconds() + 0.2;
    
    if ( mSendDataThread.joinable() )
        mSendDataThread.join();
}


void PixelPusher::onConnect( UdpSessionRef session )
{
    console() << "PixelPusher Connected: " << getIp() << endl;
    
    mSession = session;
    mSession->connectErrorEventHandler( &PixelPusher::onError, this );
}


void PixelPusher::onError( string err, size_t bytesTransferred )
{
    console() << "PixelPusher Socket error: " << err << endl;
}


void PixelPusher::sendPacketToPusher()
{
// TODO: add multicast stuff
//    if (pusher.isMulticast()) {
//        if (!pusher.isMulticastPrimary()) {
//            try {
//                Thread.sleep(1000);
//            } catch (InterruptedException ie) {
//                // we don't care.
//            }
//            continue; // we just sleep until we're primary
//        }
//    }
//
//    long estimatedSleep = (System.nanoTime() - lastWorkTime)/1000000;
//    estimatedSleep = Math.min(estimatedSleep, ((1000/registry.getFrameLimit())
//                                               / (pusher.stripsAttached / stripPerPacket)));

    mRunThread          = true;
    mTerminateThreadAt  = -1.0;
    mThreadSleepMsec    = 16;
    
    StripRef            strip;
    vector<StripRef>    touchedStrips;
    vector<PixelRef>    pixels;
    int                 packetLength;
    size_t              stripDataSize;
    long                totalDelay;
    bool                payload;
    int                 stripIdx;
    int                 maxStripsPerPacket  = getMaxStripsPerPacket();
    int                 stripPerPacket      = min( (uint8_t)maxStripsPerPacket, getStripsAttached() );
    uint8_t             *stripData;
    
    // packet buffer
    ci::Buffer          packetBuffer        = Buffer( 4 + ( ( 1 + 3 * getPixelsPerStrip() ) * getMaxStripsPerPacket() ) );
    uint8_t             *packetData         = (uint8_t*)packetBuffer.getData();

    // reset command packet buffer
    int                 cmdPacketLength     = ( ( getPusherFlags() & PFLAG_FIXEDSIZE ) != 0 ) ? 4 + ( ( 1 + 3 * getPixelsPerStrip() ) * stripPerPacket ) :  PP_CMD_RESET_SIZE + 4;
    ci::Buffer          resetCmdBuffer      = Buffer(cmdPacketLength);
    uint8_t             *resetCmdData       = (uint8_t*)resetCmdBuffer.getData();
    memcpy( &resetCmdData[4], &PP_CMD_RESET[0], PP_CMD_RESET_SIZE );
   
    while( mRunThread )
    {
        // Terminated the thread using a delay to ensure it send out the latest data(black pixels)
        
        if ( mTerminateThreadAt > 0 && getElapsedSeconds() > mTerminateThreadAt )
        {
            console() << "STOP fucking thread" << endl;
            mRunThread = false;
            continue;
        }
        
        if ( mSession && mSession->getSocket()->is_open() )
        {
            touchedStrips = getTouchedStrips();
            
            // adjust thread speed, getUpdatePeriod in uSec
            if ( getUpdatePeriod() > 100000 )
                mThreadSleepMsec = ( 16 / ( getStripsAttached() / stripPerPacket ) );       // Handle errant delay calculation in the firmware.
            
            else if ( getUpdatePeriod() > 1000 )
                mThreadSleepMsec = ( getUpdatePeriod() / 1000 ) + 1;
            
            else
                mThreadSleepMsec = ( ( 1000 / PusherDiscoveryService::getFrameLimit() ) / ( getStripsAttached() / stripPerPacket ) );
            
            totalDelay = mThreadSleepMsec + mThreadExtraDelayMsec + getExtraDelay();
            
            // no commands or touched strips, nothing to do
            if ( !mSendReset && touchedStrips.empty() )
            {
                this_thread::sleep_for( chrono::milliseconds( totalDelay ) );
                continue;
            }
            
            if ( mSendReset )
            {
                // update Packet number
                memcpy( &resetCmdData[0], &mPacketNumber, 4 );
                
                // send packet
                mSession->write( resetCmdBuffer );
                mPacketNumber++;
                
                mSendReset = false;
                
                console() << getElapsedSeconds() << " PixelPusher reset device: " << getIp() << endl;
  
                this_thread::sleep_for( chrono::milliseconds( totalDelay ) );

                continue;
            }
            
            // send strip data
            stripIdx = 0;
            
            while( stripIdx < touchedStrips.size() )
            {
                packetLength    = 0;
                payload         = false;
                
                // Packet number
                memcpy( &packetData[0], &mPacketNumber, 4 );
                packetLength += 4;
               
                // Now loop over remaining strips.
                for ( int k = 0; k < stripPerPacket; k++ )
                {
                    if ( stripIdx >= touchedStrips.size() )
                        break;
                    
                    strip = touchedStrips[stripIdx++];
                    
                    // add strip number
                    packetData[packetLength++] = strip->getStripNumber();
                    
                    // update pixels buffer
                    strip->updatePixelsBuffer();
                    
                    stripData       = strip->getPixelsData();
                    stripDataSize   = strip->getPixelsDataSize();
                    
                    // add pixels data
                    memcpy( &packetData[packetLength], stripData, stripDataSize );
                    packetLength += stripDataSize;
                    
                    strip->markTouched(false);
                    
                    payload = true;
                }
                
                if ( payload )
                {
                    // send packet
                    mSession->write( packetBuffer );
                    mPacketNumber++;
                    payload = false;
                    this_thread::sleep_for( chrono::milliseconds( totalDelay ) );
                }
            }
        }
        // the socket is not ready, wait a bit
        else
            this_thread::sleep_for( chrono::milliseconds( 16 ) );
    }
    
    console() << "PixelPusher::sendPacketToPusher() thread exited!" << endl;
}


void PixelPusher::increaseExtraDelay( uint32_t i )
{
    if ( PusherDiscoveryService::getAutoThrottle() )
        mExtraDelayMsec += i;
}


void PixelPusher::decreaseExtraDelay( uint32_t i )
{
    if ( !PusherDiscoveryService::getAutoThrottle() )
        return;
    
    if (  mExtraDelayMsec >= i )
        mExtraDelayMsec = mExtraDelayMsec - i;
    else
        mExtraDelayMsec = 0;
}

