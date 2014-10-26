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

#ifndef PP_DEVICE
#define PP_DEVICE

#pragma once

#include "PusherCommand.h"
#include "DeviceHeader.h"
#include <boost/enable_shared_from_this.hpp>


class Strip;
typedef std::shared_ptr<Strip> StripRef;

class PixelPusher;
typedef std::shared_ptr<PixelPusher> PixelPusherRef;

class CardThread;
typedef std::shared_ptr<CardThread> CardThreadRef;


class PixelPusher : public std::enable_shared_from_this<PixelPusher> {
    
private:
    
    const int ACCEPTABLE_LOWEST_SW_REV  = 121;
    const int SFLAG_RGBOW               = 1;
    const int SFLAG_WIDEPIXELS          = (1<<1);
    const int SFLAG_LOGARITHMIC         = (1<<2);
    const int SFLAG_MOTION              = (1<<3);
    const int SFLAG_NOTIDEMPOTENT       = (1<<4);
    
    const int PFLAG_PROTECTED           = (1<<0);
    const int PFLAG_FIXEDSIZE           = (1<<1);
    
    
public:
    
    static PixelPusherRef create( DeviceHeader header )
    {
        return PixelPusherRef( new PixelPusher( header ) );
    }
    
    ~PixelPusher();
    
    void createCardThread( boost::asio::io_service& ioService );
    
    uint16_t getPort()
    {
        if ( mPort > 0 )
          return mPort;

        return 9897;
    }

    void setPort( uint16_t port ) { mPort = port ; }

    std::string getMacAddress()         { return mDeviceHeader.getMacAddressString(); }
	std::string getIp()                 { return mDeviceHeader.getIpAddressString(); }
	DeviceType  getDeviceType()         { return mDeviceHeader.getDeviceType(); }
	uint32_t 	getProtocolVersion()	{ return mDeviceHeader.getProtocolVersion(); }
	uint32_t    getVendorId()			{ return mDeviceHeader.getVendorId(); }
	uint32_t    getProductId()			{ return mDeviceHeader.getProductId(); }
	uint32_t    getHardwareRevision()	{ return mDeviceHeader.getHardwareRevision(); }
	uint32_t    getSoftwareRevision()	{ return mDeviceHeader.getSoftwareRevision(); }
	uint64_t    getLinkSpeed()          { return mDeviceHeader.getLinkSpeed(); }
    uint32_t    getSegments()           { return mSegments; }
    int         getNumberOfStrips()     { return mStrips.size(); }
    
//    void sendCommand( PusherCommand pc )
//    {
//        commandQueue.push_back( pc );
//    }
  
    
    std::vector<StripRef>   getStrips();
    size_t                  getNumStrips();
    
    uint16_t getArtnetUniverse()   { return mArtnetUniverse; }
    uint16_t getArtnetChannel()    { return mArtnetChannel; }

    StripRef getStrip( int stripNumber );
    
    void setAutoThrottle( bool state ) { mAutothrottle = state; }

    int getMaxStripsPerPacket() { return mMaxStripsPerPacket; }

    int getPixelsPerStrip() { return mPixelsPerStrip; }

    uint64_t getPusherFlags() { return mPusherFlags; }

    void setPusherFlags( uint64_t pusherFlags ) { mPusherFlags = pusherFlags; }
    
    uint64_t getUpdatePeriod()  { return mUpdatePeriod; }
    uint64_t getPowerTotal()    { return mPowerTotal; }
    uint64_t getDeltaSequence() { return mDeltaSequence; }
    
    uint8_t getStripsAttached() { return mStripsAttached; }
    
    
    void increaseExtraDelay( uint64_t i )
    {
        if ( mAutothrottle )
        {
            mExtraDelayMsec += i;
            ci::app::console() << "Group " << mGroupOrdinal << " card " << mControllerOrdinal << " extra delay now " << mExtraDelayMsec << std::endl;
        }
        else
            ci::app::console() << "Group " << mGroupOrdinal << " card " << mControllerOrdinal << " would increase delay, but autothrottle is disabled." << std::endl;
    }

    void decreaseExtraDelay( uint64_t i )
    {
        mExtraDelayMsec = std::max( (uint64_t)0, mExtraDelayMsec - i );
    }
    
    uint64_t getExtraDelay()
    {
        if ( mAutothrottle )
            return mExtraDelayMsec;
        else
            return 0;
    }
    
    void setExtraDelay( uint64_t i ) { mExtraDelayMsec = i; }
    
    uint64_t getControllerOrdinal() { return mControllerOrdinal; }
    uint64_t getGroupOrdinal() { return mGroupOrdinal; }

    void updateVariables( PixelPusherRef device );
        
    void copyHeader( PixelPusherRef device );

    void setAntiLog( bool antiLog );

    void startRecording( std::string filename )
    {
        mAmRecording = true;
        setFilename( filename );
    }

    std::string getFilename() { return mFilename; }

    void setFilename( std::string filename) { mFilename = filename; }
    
    bool isAmRecording() { return mAmRecording; }

    void setAmRecording( bool amRecording ) { mAmRecording = amRecording; }

    void makeBusy() { mIsBusy = true; }

    void clearBusy() { mIsBusy = false; }

    bool isBusy() { return mIsBusy; }

    bool hasTouchedStrips() { return mTouchedStrips; }
  
    void markUntouched() {  mTouchedStrips = false; }
  
    void markTouched() { mTouchedStrips = true; }
  
    std::vector<StripRef> getTouchedStrips();
        
    uint64_t getPowerDomain() { return mPowerDomain; }

    void shutDown()
    {
        clearBusy();
    }

    bool isMulticast() { return mMulticast; }

    bool isMulticastPrimary() { return mMulticastPrimary; }

    void setMulticastPrimary( bool b ) { mMulticastPrimary = b; }

    void setMulticast( bool b ) { mMulticast = b; }

    void setLastUniverse(int universe) { mLastUniverse = universe; }

    int getLastUniverse() { return mLastUniverse; }
    
    bool isEqual( PixelPusherRef otherDevice );
    
    bool isIpAddrMulticast()
    {
        return mDeviceHeader.isMulticast();
    }
    
private:
    
    PixelPusher( DeviceHeader header );
    
    bool hasRGBOW();
    
    std::string formattedStripFlags() ;

    void createStrips();
    
    /**
       * All access (including iteration) and mutation must be performed
       * while holding stripLock
       */
    
private:
    
      // private final Object stripLock = new Object();
    
    std::vector<StripRef> mStrips;
    uint64_t              mExtraDelayMsec;
    bool                  mAutothrottle;
  
    bool                  mMulticast;
    bool                  mMulticastPrimary;
  
    /**
     * Queue for commands using the new majik strip protocol.
     */
  
    std::vector<PusherCommand> mCommandQueue;
  
    uint8_t       	mStripsAttached;
    uint8_t         mMaxStripsPerPacket;
    uint16_t        mPixelsPerStrip;

    uint32_t        mUpdatePeriod;
    uint32_t        mPowerTotal;
    uint32_t        mDeltaSequence;
    uint32_t        mControllerOrdinal;
    uint32_t        mGroupOrdinal;
    
    uint16_t        mArtnetUniverse;
    uint16_t        mArtnetChannel;

    
    uint16_t        mPort;
    
    bool            mTouchedStrips;
    bool            mUseAntiLog;
    std::string     mFilename;
    bool            mAmRecording;
    bool            mIsBusy;
    uint32_t        mPusherFlags;
    uint32_t        mSegments;
    uint32_t    	mPowerDomain;
    int             mLastUniverse;
    
    std::vector<int8_t>    mStripFlags;
    
	DeviceHeader	mDeviceHeader;
    CardThreadRef   mCardThread;

};

#endif
