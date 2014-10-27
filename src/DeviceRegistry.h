
#ifndef PIXEL_PUSHER_DEVICE_REGISTRY
#define PIXEL_PUSHER_DEVICE_REGISTRY

#pragma once

#include "UdpServer.h"
#include "PusherGroup.h"

#define DR_DISCOVERY_PORT           7331
#define DR_DISCONNECT_TIMEOUT       10
#define DR_EXPIRE_TIMER_INTERVAL    1

class PixelPusher;
typedef std::shared_ptr<PixelPusher>    PixelPusherRef;

class DeviceRegistry;
typedef std::shared_ptr<DeviceRegistry>    DeviceRegistryRef;


class DeviceRegistry {

public:
    
    static DeviceRegistryRef create( boost::asio::io_service& ioService  )
    {
        return DeviceRegistryRef( new DeviceRegistry( ioService ) );
    }
    
    ~DeviceRegistry() {}
    
    static double   getPowerScale() { return PowerScale; }
    static int      getFrameLimit() { return FrameLimit; }
    
    static double   getOverallBrightnessScale() { return OverallBrightnessScale; }
    static bool     getUseOverallBrightnessScale() { return UseOverallBrightnessScale; }
    
    std::vector<PixelPusherRef>     getPushers() { return mPushersSorted; }
    
private:
    
    DeviceRegistry( boost::asio::io_service& ioService );
    
    void onAccept( UdpSessionRef session );
	void onError( std::string err, size_t bytesTransferred );
	void onRead( ci::Buffer buffer );
    
    void addNewPusher( PixelPusherRef pusher );
    
    std::vector<PixelPusherRef>     getPushersByIp( std::string ipAddr );
    
    
private:
    
	UdpServerRef    mServer;
	UdpSessionRef   mSession;
    
private:

    static double       OverallBrightnessScale;
    static bool         UseOverallBrightnessScale;
    static uint32_t     TotalPower;
    static uint32_t     TotalPowerLimit;
    static double       PowerScale;
    static bool         AutoThrottle;
    static bool         AntiLog;
    static int          FrameLimit;
    static bool         HasDiscoveryListener;
    static bool     	AlreadyExist;
    
private:
    
    std::map<std::string, double>           mPusherLastSeenMap;
    std::map<std::string, PixelPusherRef>   mPusherMap;
    std::vector<PixelPusherRef>             mPushersSorted;
    std::map<uint32_t, PusherGroup>         mGroupMap;
    boost::asio::io_service&                mIoService;
    
    
//    boolean expiryEnabled = true;
//    Timer expiryTimer;
//    public Boolean hasDeviceExpiryTask=false;
  
};


#endif
