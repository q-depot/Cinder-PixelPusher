/*
 *  PusherDiscoveryService.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2014 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

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
    
    static int      getTotalPower()             { return TotalPower; }
    static int      getTotalPowerLimit()        { return TotalPowerLimit; }
    static int      getFrameLimit()             { return FrameLimit; }
    static double   getGlobalBrightness()       { return GlobalBrightness; }
    static bool     isGlobalBrightness()        { return IsGlobalBrightness; }
    static bool     getAutoThrottle()           { return IsAutoThrottle; }
    static bool     getAntiLog()                { return IsAntiLog; }
    static double   getPowerScale()             { return PowerScale; }
    
    static void     setTotalPowerLimit( int powerLimit )        { TotalPowerLimit           = powerLimit; }
    static void     setFrameLimit( int frameLimit )             { FrameLimit                = frameLimit; }
    static void     setGlobalBrightness( double brightness )    { GlobalBrightness          = brightness; }
    static void     enableGlobalBrightness( bool isEnable )     { IsGlobalBrightness        = isEnable; }
    static void     enableAutoThrottle( bool isEnable )         { IsAutoThrottle            = isEnable; }
    static void     enableAntiLog( bool isEnable )              { IsAntiLog                 = isEnable; }
    
    
private:
    
    PusherDiscoveryService( boost::asio::io_service& ioService );
    
    void onAccept( UdpSessionRef session );
	void onError( std::string err, size_t bytesTransferred );
	void onRead( ci::Buffer buffer );
    
    void addNewPusher( PixelPusherRef pusher );
    
    std::vector<PixelPusherRef>     getPushersByIp( std::string ipAddr );
    
    void updateGroups();
    
private:
    
    static double   PowerScale;
    static double   GlobalBrightness;
    static bool     IsGlobalBrightness;
    static bool     IsAutoThrottle;
    static bool     IsAntiLog;
    static int      TotalPower;
    static int      TotalPowerLimit;
    static int      FrameLimit;
    
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
