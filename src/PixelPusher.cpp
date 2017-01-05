/*
 *  PixelPusher.cpp
 *
 *  Created by Andrea Cuius
 *  The MIT License (MIT)
 *  Copyright (c) 2015 Nocte Studio Ltd.
 *
 *  www.nocte.co.uk
 *
 */

#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "PixelPusher.h"
#include "PusherDiscoveryService.h"

using namespace ci;
using namespace ci::app;
using namespace std;

uint8_t Pixel::sLinearExp[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12,
      13, 13, 13, 14, 14, 14, 14, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27,
      27, 28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57,
      59, 60, 61, 63, 64, 65, 67, 68, 70, 72, 73, 75, 76, 78, 80, 82, 83, 85, 87, 89, 91, 93, 95, 97, 99, 102, 104, 106, 109, 111, 114, 116,
      119, 121, 124, 127, 129, 132, 135, 138, 141, 144, 148, 151, 154, 158,
      161, 165, 168, 172, 176, 180, 184, 188, 192, 196, 201, 205,
      209, 214, 219, 224, 229, 234, 239, 244, 249, 255 };

uint8_t Strip::sLinearExp[256] = {  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4,
        4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10,
        10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 17, 17, 17, 18,
        18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27,
        28, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 36, 36, 37, 37, 38, 38, 39,
        40, 40, 41, 42, 42, 43, 44, 44, 45, 46, 46, 47, 48, 48, 49, 50, 51, 51, 52, 53, 54, 54,
        55, 56, 57, 57, 58, 59, 60, 61, 62, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 89, 90, 91, 92, 93, 94, 96, 97, 98,
        99, 101, 102, 103, 105, 106, 107, 109, 110, 111, 113, 114, 116, 117, 119, 120, 121, 123, 125, 126,
        128, 129, 131, 132, 134, 136, 137, 139, 141, 142, 144, 146, 148, 150, 151, 153, 155, 157, 159, 161,
        163, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 186, 188, 190, 192, 195, 197, 199, 202, 204,
        206, 209, 211, 214, 216, 219, 221, 224, 227, 229, 232, 235, 237, 240, 243, 246, 249, 252 };


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
    
    if ( swRev < (uint32_t)ACCEPTABLE_LOWEST_SW_REV )
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
}


PixelPusher::~PixelPusher()
{
    if (  mSendDataThread.joinable() )
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
    
    setPixelMap( ci::vec2(0,0), Strip::MAP_LEFT_RIGHT );
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
void PixelPusher::createCardThread( asio::io_service& ioService )
{
    createStrips();
    
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
    if ( mSendDataThread.joinable() )
    {
        // terminate in 200ms to ensure it sends out the black pixels
        mTerminateThreadAt = getElapsedSeconds() + 0.2;

        mSendDataThread.join();
    }
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
    ci::BufferRef       packetBuffer        = Buffer::create( 4 + ( ( 1 + 3 * getPixelsPerStrip() ) * getMaxStripsPerPacket() ) );
    uint8_t             *packetData         = (uint8_t*)packetBuffer->getData();

    // reset command packet buffer
    int                 cmdPacketLength     = ( ( getPusherFlags() & PFLAG_FIXEDSIZE ) != 0 ) ? 4 + ( ( 1 + 3 * getPixelsPerStrip() ) * stripPerPacket ) :  PP_CMD_RESET_SIZE + 4;
    ci::BufferRef       resetCmdBuffer      = Buffer::create(cmdPacketLength);
    uint8_t             *resetCmdData       = (uint8_t*)resetCmdBuffer->getData();
    memcpy( &resetCmdData[4], &PP_CMD_RESET[0], PP_CMD_RESET_SIZE );
   
    while( mRunThread )
    {
        // Terminated the thread using a delay to ensure it send out the latest data(black pixels)
        
        if ( mTerminateThreadAt > 0 && getElapsedSeconds() > mTerminateThreadAt )
        {
            console() << "PixelPusher::sendPacketToPusher() Thread is set to expire." << endl;

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
            
            while( (size_t)stripIdx < touchedStrips.size() )
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


void PixelPusher::setPixels( ci::Surface8u *image )
{
    Strip::PixelMap map;
    ColorA          col;
    StripRef        strip;
    vec2            stepVec;
    vec2            pixelPos;
    
    for( size_t k=0; k < mStrips.size(); k++ )
    {
        strip   = mStrips[k];
        map     = strip->getPixelMap();
        
        pixelPos    = map.from;
        stepVec     = ( map.to - map.from ) / (float)strip->getNumPixels();

        for( size_t j=0; j < strip->getNumPixels(); j++ )
        {
            if ( pixelPos.y < 0 || pixelPos.y >= image->getHeight() || pixelPos.x < 0 || pixelPos.x >= image->getWidth() )
                col = Color::black();
            else
                col = image->getPixel( pixelPos );
            
            pixelPos += stepVec;
            strip->setPixel( j, (uint8_t)(col.r * 255), (uint8_t)(col.g * 255), (uint8_t)(col.b * 255) );
        }
    }
}


void PixelPusher::setPixelMap( ci::vec2 offset, Strip::PixelMapOrientation orientation )
{
    if ( orientation == Strip::MAP_LEFT_RIGHT || orientation == Strip::MAP_RIGHT_LEFT )
    {
        for( size_t k=0; k < mStrips.size(); k++ )
        {
            mStrips[k]->setPixelMap( offset, orientation );
            offset.y++;
        }
    }
    
    else if ( orientation == Strip::MAP_TOP_DOWN || orientation == Strip::MAP_BOTTOM_UP )
    {
        for( size_t k=0; k < mStrips.size(); k++ )
        {
            mStrips[k]->setPixelMap( offset, orientation );
            offset.x++;
        }
    }
}

