/**
   * uint8_t strips_attached;
   * uint8_t max_strips_per_packet;
   * uint16_t pixels_per_strip; // uint16_t used to make alignment work
   * uint32_t update_period; // in microseconds
   * uint32_t power_total; // in PWM units
   * uint32_t delta_sequence; // difference between received and expected
   * sequence numbers
   * int32_t controller_ordinal;  // configured order number for controller
   * int32_t group_ordinal;  // configured group number for this controller
   * int16_t artnet_universe;
   * int16_t artnet_channel;
   * int16_t my_port;
   */

#include "cinder/Utilities.h"
#include "PixelPusher.h"
#include "Strip.h"
#include "DeviceRegistry.h"


PixelPusher::PixelPusher( DeviceHeader header ) : mDeviceHeader(header)
{
    mArtnetUniverse   	= 0;
    mArtnetChannel    	= 0;
    mPort               = 9798;
    mStripsAttached     = 0;
    mPixelsPerStrip     = 0;
  
    mExtraDelayMsec     = 0;
    mAutoThrottle       = false;
  
    mMulticast          = false;
    mMulticastPrimary   = false;
    
    mSegments           = 0;
    mPowerDomain        = 0;
    
    setPusherFlags(0);
    
    uint32_t swRev      = header.getSoftwareRevision();
    
    if ( swRev < ACCEPTABLE_LOWEST_SW_REV )
    {
        ci::app::console() << "WARNING!  This PixelPusher Library requires firmware revision " + std::to_string( ACCEPTABLE_LOWEST_SW_REV / 100.0 ) << std::endl;
        ci::app::console() << "WARNING!  This PixelPusher is using " << std::to_string( swRev / 100.0 ) << std::endl;
        ci::app::console() << "WARNING!  This is not expected to work.  Please update your PixelPusher." << std::endl;
    }
    
    std::shared_ptr<uint8_t>    packet      = mDeviceHeader.getPacketReminder();
    int                         packetSize  = mDeviceHeader.getPacketReminderSize();
    
    if ( packetSize < 28 )
        throw std::invalid_argument("Packet size < 28");
    
    memcpy( &mStripsAttached,       &packet.get()[0],   1 );
    memcpy( &mMaxStripsPerPacket,   &packet.get()[1],   1 );
    memcpy( &mPixelsPerStrip,       &packet.get()[2],   2 );
    memcpy( &mUpdatePeriod,         &packet.get()[4],   4 );
    memcpy( &mPowerTotal,           &packet.get()[8],   4 );
    memcpy( &mDeltaSequence,        &packet.get()[12],  4 );
    memcpy( &mControllerOrdinal,    &packet.get()[16],  4 );
    memcpy( &mGroupOrdinal,         &packet.get()[20],  4 );
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

    /*
     * We have some entries that come after the per-strip flag array.
     * We represent these as longs so that the entire range of a uint may be preserved;
     * why on earth Java doesn't have unsigned ints I have no idea. - jls
     * 
     * uint32_t pusher_flags;      // flags for the whole pusher
     * uint32_t segments;          // number of segments in each strip
     * uint32_t power_domain;      // power domain of this pusher
     */
    
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
    for( size_t k=0; k < mStrips.size(); k++ )
        mStrips[k]->setPixelsBlack();
    
    ci::sleep( 200 );
    
    destroyCardThread();
}


std::vector<StripRef> PixelPusher::getStrips()
{
    // Devices that are members of a multicast group,
    // but which are not the primary member of that group,
    // do not return strips.
    if ( mMulticast && !mMulticastPrimary )
        return std::vector<StripRef>();

    if ( mStrips.empty() )      // TODO: create strips here doesn't make any fucking sense!
        createStrips();

    return mStrips;
    // Ensure callers can't modify the returned list
    // return Collections.unmodifiableList(strips);
}


StripRef PixelPusher::getStrip( int stripNumber )
{
    if ( stripNumber > mStripsAttached )
        return StripRef();
    
    
    if ( mStrips.empty() )
        createStrips();

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
    mControllerOrdinal  = device->mControllerOrdinal;
    mDeltaSequence      = device->mDeltaSequence;
    mGroupOrdinal       = device->mGroupOrdinal;
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


void PixelPusher::setAntiLog( bool antiLog )
{
    mUseAntiLog = antiLog;

    if ( !mStrips.empty() )
        return;
    
    createStrips();
        
    for( size_t k=0; k < mStrips.size(); k++ )
        mStrips[k]->useAntiLog( mUseAntiLog );
}


std::vector<StripRef> PixelPusher::getTouchedStrips()
{
    if ( mStrips.empty() )
        createStrips();

    std::vector<StripRef> touchedStrips;
    
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


std::string PixelPusher::formattedStripFlags()
{
    std::string s;
    
    for( int i = 0; i < mStripsAttached; i++ )
        s += "[" + std::to_string(mStripFlags[i]) + "]";
    
    return s;
}


void PixelPusher::createStrips()
{
    for( uint8_t k = 0; k < mStripsAttached; k++ )
    {
        StripRef strip = Strip::create( k, mPixelsPerStrip );

        if ( ( mStripFlags[k] & SFLAG_LOGARITHMIC ) != 0 )
            strip->useAntiLog(false);
        else
            strip->useAntiLog( mUseAntiLog );
        
        if ( ( mStripFlags[k] & SFLAG_MOTION ) != 0 )
            strip->setMotion(true);
        else
            strip->setMotion(false);
        
        if ( ( mStripFlags[k] & SFLAG_NOTIDEMPOTENT ) != 0 )
            strip->setNotIdempotent(true);
        else
            strip->setNotIdempotent(false);
        
        strip->setRGBOW( ( mStripFlags[k] & SFLAG_RGBOW ) == 1 );
        
        mStrips.push_back( strip );
    }
}


bool PixelPusher::isEqual( PixelPusherRef otherDevice )
{
    // if it differs by less than half a msec, it has no effect on our timing
    int updatePeriod = getUpdatePeriod() - otherDevice->getUpdatePeriod();     // update period is uint32_t
    if ( std::abs( updatePeriod ) > 500 )
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
    if ( std::abs( powerTotal ) > 10000 )
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
    //    mBandwidthEstimate      = 0;
    
    //    mLastWorkTime   = ci::app::getElapsedSeconds();
    
    
    int maxPacketSize  = 4 +  ( ( 1 + 3 * getPixelsPerStrip() ) * getMaxStripsPerPacket() );
    mPacketBuffer       = ci::Buffer( maxPacketSize );
    
    mPacketNumber   = 0;
    
    mClient = UdpClient::create( ioService );
    mClient->connectConnectEventHandler( &PixelPusher::onConnect, this );
    mClient->connectErrorEventHandler( &PixelPusher::onError, this );
    
    mClient->connect( getIp(), getPort() );
    
//    if ( getUpdatePeriod() > 100 && getUpdatePeriod() < 1000000 )
//        mThreadSleepMsec = ( getUpdatePeriod() / 1000 ) + 1;
    
    mSendDataThread = std::thread( &PixelPusher::sendPacketToPusher, this );
}


void PixelPusher::destroyCardThread()
{
    mRunThread = false;
    if ( mSendDataThread.joinable() )
        mSendDataThread.join();
}


void PixelPusher::onConnect( UdpSessionRef session )
{
    ci::app::console() << "PixelPusher Connected" << std::endl;
    
    mSession = session;
    mSession->connectErrorEventHandler( &PixelPusher::onError, this );
}


void PixelPusher::onError( std::string err, size_t bytesTransferred )
{
    ci::app::console() << "PixelPusher Socket ERROR: " << err << std::endl;
}


void PixelPusher::sendPacketToPusher()
{
    mRunThread          = true;
    mThreadSleepMsec    = 16;
    
    StripRef                strip;
    std::vector<StripRef>   touchedStrips;
    std::vector<PixelRef>   pixels;
    int                     packetLength;
    int                     maxStripsPerPacket;
    int                     stripPerPacket;
    size_t                  stripDataSize;
    long                    totalDelay;
    bool                    payload;
    int                     stripIdx;
    uint8_t                 *packetData = (uint8_t*)mPacketBuffer.getData();
    uint8_t                 *stripData;
    
    while( mRunThread )
    {
        if ( mSession && mSession->getSocket()->is_open() )
        {
            touchedStrips = getTouchedStrips();
            
            // no commands or touched strips, nothing to do
            if ( mCommandQueue.empty() && touchedStrips.empty() )
            {
                std::this_thread::sleep_for( std::chrono::milliseconds( mThreadSleepMsec ) );
                continue;
            }
                
            maxStripsPerPacket  = getMaxStripsPerPacket();
            stripPerPacket      = std::min( (uint8_t)maxStripsPerPacket, getStripsAttached() );
            
            // adjust thread speed
            if ( false && getUpdatePeriod() > 1000 )
                mThreadSleepMsec = ( getUpdatePeriod() / 1000 ) + 1;
            else                                                                                    // Shoot for the framelimit.
                mThreadSleepMsec = ( ( 1000 / DeviceRegistry::getFrameLimit() ) / ( getStripsAttached() / stripPerPacket ) );
                                                                                                 
            // Handle errant delay calculation in the firmware.
            if ( getUpdatePeriod() > 100000 )
                mThreadSleepMsec = ( 16 / ( getStripsAttached() / stripPerPacket ) );
            
            
            
            
            // TODO: not used!!!!!!!!!!!!!! <<<<<<<<<<<<<<<<<<<<<<
            totalDelay = mThreadSleepMsec + mThreadExtraDelayMsec + getExtraDelay();
            
            
            
            
            // first check to see if we have an outstanding command.
            if ( !mCommandQueue.empty() )
            {
                for( size_t k=0; k < mCommandQueue.size(); k++ )
                {
                    // cmd + packen_number(4) or 4 + ( ( 1 + 3 * getPixelsPerStrip() ) * stripPerPacket )
                    int packetLength;
                    
                    // We need fixed size datagrams for the Photon, because the cc3000 sucks.
                    if ( ( getPusherFlags() & PFLAG_FIXEDSIZE ) != 0 )
                        packetLength = 4 + ( ( 1 + 3 * getPixelsPerStrip() ) * stripPerPacket );
                    else
                        packetLength = mCommandQueue[k]->mDataSize + 4;
                    
                    ci::Buffer  cmdPacket(packetLength);
                    uint8_t     *cmdPacketData = (uint8_t*)cmdPacket.getData();
                    
                    // Packet number
                    memcpy( &cmdPacketData[0], &mPacketNumber, 4 );
                    
                    // cmd data
                    memcpy( &cmdPacketData[4], &mCommandQueue[k]->mData.get()[0], mCommandQueue[k]->mDataSize );
                    
                    // send packet
                    mSession->write( cmdPacket );
                    mPacketNumber++;
                    
                    ci::app::console() << "send command " << packetLength << " " << mCommandQueue[k]->mDataSize << std::endl;
                    
                    ci::app::console() << "COMMAND: " << std::endl;
                    for( int j=0; j < mCommandQueue[k]->mDataSize; j++ )
                    {
                        ci::app::console() << (int)mCommandQueue[k]->mData.get()[j] << " ";
                    }
                    
                    ci::app::console() << std::endl << "-------------------------" << std::endl;
                    
                    ci::app::console() << "PACKET: " << std::endl;
                    for( int j=0; j < packetLength; j++ )
                    {
                        ci::app::console() << (int)cmdPacketData[j] << " ";
                    }
                    
                    ci::app::console() << std::endl << std::endl;
                }
                
                mCommandQueue.clear();
                
                std::this_thread::sleep_for( std::chrono::milliseconds( mThreadSleepMsec ) );
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
                    
                    // Don't weed untouched strips if we are recording.
                    //    if (!fileIsOpen) {
                    //        if (!strip.isTouched() && ((mPusher->getPusherFlags() & mPusher->PFLAG_FIXEDSIZE) == 0))
                    //            continue;
                    //    }
                    
                    payload = true;
                }
                
                if ( payload )
                {
                    // send packet
                    mSession->write( mPacketBuffer );
                    mPacketNumber++;
                    payload = false;
                }
            }
            
            std::this_thread::sleep_for( std::chrono::milliseconds( mThreadSleepMsec ) );
        }
        else
        {
            ci::app::console() << "Session is not open!" << std::endl;
            std::this_thread::sleep_for( std::chrono::milliseconds( mThreadSleepMsec ) );
        }
        
    }
    
    ci::app::console() << "PixelPusher::sendPacketToPusher() thread exited!" << std::endl;
}

