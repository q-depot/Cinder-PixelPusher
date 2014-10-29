
#ifndef PIXEL_PUSHER_DEVICE_REGISTRY
#define PIXEL_PUSHER_DEVICE_REGISTRY

#pragma once

#include "UdpServer.h"
#include "PusherGroup.h"

#define PP_DISCOVERY_SERVICE_PORT       7331


class PixelPusher;
typedef std::shared_ptr<PixelPusher>    PixelPusherRef;

class PusherDiscoveryService;
typedef std::shared_ptr<PusherDiscoveryService>    PusherDiscoveryServiceRef;


class PusherDiscoveryService {

public:
    
    static PusherDiscoveryServiceRef create( boost::asio::io_service& ioService  )
    {
        return PusherDiscoveryServiceRef( new PusherDiscoveryService( ioService ) );
    }
    
    ~PusherDiscoveryService();
    
    std::vector<PixelPusherRef>     getPushers() { return mPushers; }
    std::vector<PusherGroupRef>     getGroups() { return mGroups; }
    
    PusherGroupRef  getGroupById( uint32_t groupId )
    {
        for( size_t k=0; k < mGroups.size(); k++ )
            if ( mGroups[k]->getId() == groupId )
                return mGroups[k];
        
        return PusherGroupRef();
    }
    
    PixelPusherRef  getPusherById( uint32_t pusherId )
    {
        for( size_t k=0; k < mPushers.size(); k++ )
            if ( mPushers[k]->getControllerId() == pusherId )
                return mPushers[k];
        
        return PixelPusherRef();
    }
    
public:
    
    static double   getOverallBrightnessScale()     { return OverallBrightnessScale; }
    static bool     getUseOverallBrightnessScale()  { return UseOverallBrightnessScale; }
    static uint32_t getTotalPower()                 { return TotalPower; }
    static uint32_t getTotalPowerLimit()            { return TotalPowerLimit; }
    static double   getPowerScale()                 { return PowerScale; }
    static bool     getAutoThrottle()               { return AutoThrottle; }
    static bool     getAntiLog()                    { return AntiLog; }
    static int      getFrameLimit()                 { return FrameLimit; }
    
    
private:
    
    PusherDiscoveryService( boost::asio::io_service& ioService );
    
    void onAccept( UdpSessionRef session );
	void onError( std::string err, size_t bytesTransferred );
	void onRead( ci::Buffer buffer );
    
    void addNewPusher( PixelPusherRef pusher );
    
    std::vector<PixelPusherRef>     getPushersByIp( std::string ipAddr );
    
    void updateGroups();
    
private:

    static double       OverallBrightnessScale;
    static bool         UseOverallBrightnessScale;
    static uint32_t     TotalPower;
    static uint32_t     TotalPowerLimit;
    static double       PowerScale;
    static bool         AutoThrottle;
    static bool         AntiLog;
    static int          FrameLimit;
    
private:
    
	UdpServerRef                    mServer;
	UdpSessionRef                   mSession;
    
    std::vector<PixelPusherRef>     mPushers;
    std::vector<PusherGroupRef>     mGroups;
    boost::asio::io_service&        mIoService;
    
    std::thread                     mUpdateGroupsThread;
    std::mutex                      mPushersMutex;
    bool                            mRunUpdateGroupsThread;
  
};


#endif
