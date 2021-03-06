/*
 *  PixelPusher.h
 *
 *  Created by Andrea Cuius
 *  The MIT License (MIT)
 *  Copyright (c) 2015 Nocte Studio Ltd.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef PIXEL_PUSHER_DEVICE
#define PIXEL_PUSHER_DEVICE

#pragma once

#include "cinder/app/App.h"
#include "cinder/Thread.h"
#include "DeviceHeader.h"
#include "Strip.h"
#include <boost/enable_shared_from_this.hpp>
#include "UdpClient.h"

#define         PP_DISCONNECT_TIMEOUT               5
#define         PP_RESET_DELAY                      8
#define         PP_CMD_RESET_SIZE                   17             // command magic first 16 bytes, last byte is the reset command(0x01)
const uint8_t   PP_CMD_RESET[PP_CMD_RESET_SIZE] =   { 0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5, 0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50, 0x01 };


class PixelPusher;
typedef std::shared_ptr<PixelPusher> PixelPusherRef;


class PixelPusher : public std::enable_shared_from_this<PixelPusher> {
    
    friend class PusherDiscoveryService;
    
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
    
    void createCardThread( asio::io_service& ioService );
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
    
    uint32_t    getThreadSleepFor() { return mThreadSleepMsec; }
    
    uint32_t    getPacketNumber() { return mPacketNumber; }
    
    std::vector<StripRef>   getStrips();
    
    uint16_t getArtnetUniverse()   { return mArtnetUniverse; }
    uint16_t getArtnetChannel()    { return mArtnetChannel; }

    StripRef getStrip( int stripNumber );

    int getMaxStripsPerPacket() { return mMaxStripsPerPacket; }

    int getPixelsPerStrip() { return mPixelsPerStrip; }

    uint32_t getPusherFlags() { return mPusherFlags; }

    void setPusherFlags( uint32_t pusherFlags ) { mPusherFlags = pusherFlags; }
    
    uint32_t getUpdatePeriod()  { return mUpdatePeriod; }
    uint32_t getPowerTotal()    { return mPowerTotal; }
    uint32_t getDeltaSequence() { return mDeltaSequence; }
    
    uint8_t getStripsAttached() { return mStripsAttached; }
    
    void increaseExtraDelay( uint32_t i );

    void decreaseExtraDelay( uint32_t i );
    
    uint32_t getExtraDelay() { return mExtraDelayMsec; }
    
    uint32_t getControllerId() { return mControllerId; }
    uint32_t getGroupId() { return mGroupId; }

    void updateVariables( PixelPusherRef device );
        
    void copyHeader( PixelPusherRef device );

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
    
    bool isIpAddrMulticast() { return mDeviceHeader.isMulticast(); }
    
    bool isAlive( double timeNow )
    {
        return timeNow - mLastPingAt < PP_DISCONNECT_TIMEOUT;
    }
    
    void reset()
    {
        mSendReset      = true;
        mResetSentAt    = ci::app::getElapsedSeconds();
    }
    
    void setPixels( ci::Surface8u *image );
    
    void setPixelMap( ci::vec2 offset, Strip::PixelMapOrientation orientation );
    
private:
    
    PixelPusher( DeviceHeader header );
    
    bool hasRGBOW();
    
    std::string formattedStripFlags() ;

    void createStrips();
    
    void sendPacketToPusher();
    void onConnect( UdpSessionRef session );
    void onError( std::string err, size_t bytesTransferred );
    
    void setLastPing( double timeNow ) { mLastPingAt = timeNow; }
    
private:
    
    std::vector<StripRef>   mStrips;
    uint32_t                mExtraDelayMsec;
  
    bool                    mMulticast;
    bool                    mMulticastPrimary;
  
    uint8_t                 mStripsAttached;
    uint8_t                 mMaxStripsPerPacket;
    uint16_t                mPixelsPerStrip;

    uint32_t                mUpdatePeriod;
    uint32_t                mPowerTotal;
    uint32_t                mDeltaSequence;
    uint32_t                mControllerId;
    uint32_t                mGroupId;
    
    uint16_t                mArtnetUniverse;
    uint16_t                mArtnetChannel;

    uint16_t                mPort;
    
    uint32_t                mPusherFlags;
    uint32_t                mSegments;
    uint32_t                mPowerDomain;
    int                     mLastUniverse;
    
    std::vector<int8_t>     mStripFlags;
    
	DeviceHeader            mDeviceHeader;

    UdpClientRef            mClient;
    UdpSessionRef           mSession;
    
    uint32_t                mThreadSleepMsec;
    uint32_t                mThreadExtraDelayMsec;
    std::thread             mSendDataThread;
    bool                    mRunThread;
    double                  mTerminateThreadAt;
    uint32_t                mPacketNumber;
    
    double                  mLastPingAt;
    bool                    mSendReset;
    double                  mResetSentAt;
    
};

#endif
