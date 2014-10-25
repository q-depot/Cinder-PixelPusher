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


#include "PixelPusher.h"
#include "Strip.h"


PixelPusher::PixelPusher( DeviceHeader header ) : mDeviceHeader(header)
{
    mArtnetUniverse   	= 0;
    mArtnetChannel    	= 0;
    mPort               = 9798;
    mStripsAttached     = 0;
    mPixelsPerStrip     = 0;
  
    mExtraDelayMsec     = 0;
    mAutothrottle       = false;
  
    mMulticast          = false;
    mMulticastPrimary   = false;
    
    mSegments           = 0;
    mPowerDomain        = 0;
    mAmRecording        = false;
    
    setPusherFlags(0);
    
//      commandQueue = new ArrayBlockingQueue<PusherCommand>(3);
    
    uint32_t swRev      = header.getSoftwareRevision();
    
    if ( swRev < ACCEPTABLE_LOWEST_SW_REV )
    {
        ci::app::console() << "WARNING!  This PixelPusher Library requires firmware revision " + std::to_string( ACCEPTABLE_LOWEST_SW_REV / 100.0 ) << std::endl;
        ci::app::console() << "WARNING!  This PixelPusher is using " << std::to_string( swRev / 100.0 ) << std::endl;
        ci::app::console() << "WARNING!  This is not expected to work.  Please update your PixelPusher." << std::endl;
    }
    
    uint8_t *packet     = mDeviceHeader.getPacketReminder();
    int     packetSize  = mDeviceHeader.getPacketReminderSize();
    
    if ( packetSize < 28 )
        throw std::invalid_argument("Packet size < 28");
  
    memcpy( &mStripsAttached,       &packet[0],     1 );
    memcpy( &mMaxStripsPerPacket,   &packet[1],     1 );
    memcpy( &mPixelsPerStrip,       &packet[2],     2 );
    memcpy( &mUpdatePeriod,         &packet[4],     4 );
    memcpy( &mPowerTotal,           &packet[8],     4 );
    memcpy( &mDeltaSequence,        &packet[12],    4 );
    memcpy( &mControllerOrdinal,    &packet[16],    4 );
    memcpy( &mGroupOrdinal,         &packet[20],    4 );
    memcpy( &mArtnetUniverse,       &packet[24],    2 );
    memcpy( &mArtnetChannel,        &packet[26],    2 );
    

    if ( packetSize > 28 && swRev > 100)
        memcpy( &mPort, &packet[28],    2 );
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
        memcpy( &mStripFlags[0], &packet[30], stripFlagSize );

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
        uint64_t pusherFlags;
        memcpy( &pusherFlags, &packet[32+stripFlagSize], 4 );
        setPusherFlags( pusherFlags );
        
        memcpy( &mSegments,     &packet[36+stripFlagSize], 4 );
        memcpy( &mPowerDomain,  &packet[40+stripFlagSize], 4 );
    }
}


std::vector<StripRef> PixelPusher::getStrips() {
    // Devices that are members of a multicast group,
    // but which are not the primary member of that group,
    // do not return strips.
    if ( mMulticast && !mMulticastPrimary)
        return std::vector<StripRef>();

    if ( mStrips.empty() )
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


void PixelPusher::updateVariables( PixelPusher device )
{
    mDeltaSequence      = device.mDeltaSequence;
    mMaxStripsPerPacket = device.mMaxStripsPerPacket;
    mPowerTotal         = device.mPowerTotal;
    mUpdatePeriod       = device.mUpdatePeriod;
}
  

void PixelPusher::copyHeader( PixelPusher device )
{
    mControllerOrdinal  = device.mControllerOrdinal;
    mDeltaSequence      = device.mDeltaSequence;
    mGroupOrdinal       = device.mGroupOrdinal;
    mMaxStripsPerPacket = device.mMaxStripsPerPacket;
    mPowerTotal         = device.mPowerTotal;
    mUpdatePeriod       = device.mUpdatePeriod;
    mArtnetChannel      = device.mArtnetChannel;
    mArtnetUniverse     = device.mArtnetUniverse;
    mPort               = device.mPort;
    mFilename           = device.mFilename;
    mAmRecording        = device.mAmRecording;
    mPowerDomain        = device.mPowerDomain;
    
    setPusherFlags( device.getPusherFlags() );
    
    
    // if the number of strips we have doesn't match,
    // we'll need to make a fresh set.
    if ( mStripsAttached != device.mStripsAttached )
    {
        mStrips.clear();
        mStripsAttached = device.mStripsAttached;
    }
    
    // likewise, if the length of each strip differs,
    // we will need to make a new set.
    if ( mPixelsPerStrip != device.mPixelsPerStrip )
    {
        mStrips.clear();
        mPixelsPerStrip = device.mPixelsPerStrip;
    }
    
    // and it's the same for segments
    if ( mSegments != device.mSegments )
    {
        mStrips.clear();
        mSegments = device.mSegments;
    }
    
    if ( !mStrips.empty() )
        for( size_t k=0; k < mStrips.size(); k++ )
            mStrips[k]->setPusher( shared_from_this() );
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
    for( int k = 0; k < mStripsAttached; k++ )
    {
        StripRef strip = Strip::create( shared_from_this(), k, mPixelsPerStrip );

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
    
    mTouchedStrips = false;
    
}

