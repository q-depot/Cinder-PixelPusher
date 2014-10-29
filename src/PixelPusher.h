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

#ifndef PIXEL_PUSHER_DEVICE
#define PIXEL_PUSHER_DEVICE

#pragma once

#include "PusherCommand.h"
#include "DeviceHeader.h"
#include <boost/enable_shared_from_this.hpp>
#include "UdpClient.h"


class Strip;
typedef std::shared_ptr<Strip> StripRef;

class PixelPusher;
typedef std::shared_ptr<PixelPusher> PixelPusherRef;

//class CardThread;
//typedef std::shared_ptr<CardThread> CardThreadRef;


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
    
    int             mRndId;
    
    
    static PixelPusherRef create( DeviceHeader header )
    {
        return PixelPusherRef( new PixelPusher( header ) );
    }
    
    ~PixelPusher();
    
    void createCardThread( boost::asio::io_service& ioService );
    void destroyCardThread();
    
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
	uint32_t    getLinkSpeed()          { return mDeviceHeader.getLinkSpeed(); }
    uint32_t    getSegments()           { return mSegments; }
    
    size_t      getNumStrips()
    {
        if ( mMulticast && !mMulticastPrimary )
            return 0;
        
        return mStrips.size();
    }
    
    bool        getAutoThrottle() { return mAutoThrottle; }
    
    uint32_t    getThreadSleepFor() { return mThreadSleepMsec; }
    
    uint32_t    getPacketNumber() { return mPacketNumber; }
    
    void sendCommand( PusherCommandRef pc )
    {
        mCommandQueue.push_back( pc );
    }
  
    
    std::vector<StripRef>   getStrips();
    
    uint16_t getArtnetUniverse()   { return mArtnetUniverse; }
    uint16_t getArtnetChannel()    { return mArtnetChannel; }

    StripRef getStrip( int stripNumber );
    
    void setAutoThrottle( bool state ) { mAutoThrottle = state; }

    int getMaxStripsPerPacket() { return mMaxStripsPerPacket; }

    int getPixelsPerStrip() { return mPixelsPerStrip; }

    uint32_t getPusherFlags() { return mPusherFlags; }

    void setPusherFlags( uint32_t pusherFlags ) { mPusherFlags = pusherFlags; }
    
    uint32_t getUpdatePeriod()  { return mUpdatePeriod; }
    uint32_t getPowerTotal()    { return mPowerTotal; }
    uint32_t getDeltaSequence() { return mDeltaSequence; }
    
    uint8_t getStripsAttached() { return mStripsAttached; }
    
    void increaseExtraDelay( uint32_t i )
    {
        if ( mAutoThrottle )
        {
            mExtraDelayMsec += i;
            ci::app::console() << "Group " << mGroupOrdinal << " card " << mControllerOrdinal << " extra delay now " << mExtraDelayMsec << std::endl;
        }
        else
            ci::app::console() << "Group " << mGroupOrdinal << " card " << mControllerOrdinal << " would increase delay, but autothrottle is disabled." << std::endl;
    }

    void decreaseExtraDelay( uint32_t i )
    {
        if (  mExtraDelayMsec >= i )
            mExtraDelayMsec = mExtraDelayMsec - i;
        else
            mExtraDelayMsec = 0;
    }
    
    uint32_t getExtraDelay()
    {
        if ( mAutoThrottle )
            return mExtraDelayMsec;
        else
            return 0;
    }
    
    void setExtraDelay( uint32_t i ) { mExtraDelayMsec = i; }
    
    uint32_t getControllerOrdinal() { return mControllerOrdinal; }
    uint32_t getGroupOrdinal() { return mGroupOrdinal; }

    void updateVariables( PixelPusherRef device );
        
    void copyHeader( PixelPusherRef device );

    void setAntiLog( bool antiLog );

    bool hasTouchedStrips();
  
    std::vector<StripRef> getTouchedStrips();
        
    uint32_t getPowerDomain() { return mPowerDomain; }

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
    
    // Commands
    
    void reset()
    {
        mCommandQueue.push_back( PusherCommand::createReset() );
    }
    
    /*
     void setGlobalBrightness( uint16_t brightness )
     {
     mCommandQueue.push_back( PusherCommand::createGlobalBrightness( brightness ) );
     }
     
     void setWirelessConfig( std::string ssid, std::string key, PusherCommand::PusherSecurity security )
     {
     mCommandQueue.push_back( PusherCommand::createWirelessConfig( ssid, key, security ) );
     }
     
     void setStripsConfig(  int32_t numStrips, int32_t stripLength,
     std::vector<PusherCommand::PusherStripType> stripType, std::vector<PusherCommand::PusherColorOrder> colorOrder,
     uint16_t group, uint16_t controller,
     uint16_t artnetUniverse, uint16_t artnetChannel )
     {
     
     mCommandQueue.push_back( PusherCommand::createStripsConfig( numStrips, stripLength, stripType, colorOrder,
     group = 0, controller = 0,
     artnetUniverse = 0, artnetChannel = 0 ) );
     }
     */
    
private:
    
    PixelPusher( DeviceHeader header );
    
    bool hasRGBOW();
    
    std::string formattedStripFlags() ;

    void createStrips();
    
    void sendPacketToPusher();
    void onConnect( UdpSessionRef session );
    void onError( std::string err, size_t bytesTransferred );
    
    
private:
    
      // private final Object stripLock = new Object();
    
    std::vector<StripRef> mStrips;
    uint32_t              mExtraDelayMsec;
    bool                  mAutoThrottle;
  
    bool                  mMulticast;
    bool                  mMulticastPrimary;
  
    /**
     * Queue for commands using the new majik strip protocol.
     */
  
    std::vector<PusherCommandRef> mCommandQueue;
  
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
    
    bool            mUseAntiLog;            // TODO: again this variable is every fucking where! DeviceRegistry should keep the global value!
    uint32_t        mPusherFlags;
    uint32_t        mSegments;
    uint32_t    	mPowerDomain;
    int             mLastUniverse;
    
    std::vector<int8_t>    mStripFlags;
    
	DeviceHeader	mDeviceHeader;

    // update Thread
    UdpClientRef	mClient;
    UdpSessionRef	mSession;
    
    uint32_t        mThreadSleepMsec;
    uint32_t        mThreadExtraDelayMsec;
    ci::Buffer      mPacketBuffer;
    std::thread     mSendDataThread;
    bool            mRunThread;
    double          mTerminateThreadAt;
    uint32_t        mPacketNumber;
    
};

#endif
