
#pragma once

#include "UdpServer.h"

#define DR_DISCOVERY_PORT           7331
#define DR_DISCONNECT_TIMEOUT       10
#define DR_EXPIRE_TIMER_INTERVAL    1


class DeviceRegistry {

public:
    
    DeviceRegistry( boost::asio::io_service& ioService );
    ~DeviceRegistry();
    
private:
    
    void onAccept( UdpSessionRef session );
	void onError( std::string err, size_t bytesTransferred );
	void onRead( ci::Buffer buffer );
//	void onReadComplete();
//	void onWrite( size_t bytesTransferred );
    
private:
    
	UdpServerRef    mServer;
	UdpSessionRef   mSession;
    
private:
//    static Semaphore updateLock;
//    static int DISCOVERY_PORT = 7331;
//    static int MAX_DISCONNECT_SECONDS = 10;
//    static long EXPIRY_TIMER_MSEC = 1000L;
//    DiscoveryListenerThread _dlt;
//    
//    static double overallBrightnessScale = 1.0;
//    public static boolean useOverallBrightnessScale = false;
//    
//    static long totalPower = 0;
//    static long totalPowerLimit = -1;
//    static double powerScale = 1.0;
//    static bool autoThrottle = false;
    static bool AntiLog = false;
//    static boolean logEnabled = true;
//    static int frameLimit = 85;
//    static Boolean hasDiscoveryListener = false;
//    static Boolean alreadyExist = false;
//    boolean expiryEnabled = true;
//    
//    Map<String, PixelPusher> pusherMap;
//    Map<String, Long> pusherLastSeenMap;
//    
//    Timer expiryTimer;
//    
//    SceneThread sceneThread;
//    
//    TreeMap<Integer, PusherGroup> groupMap;
//    
//    TreeSet<PixelPusher> sortedPushers;
    
    
//    public Boolean hasDeviceExpiryTask=false;
  
};



/*
private final static Logger LOGGER = Logger.getLogger(DeviceRegistry.class
                                                      .getName());
private static Semaphore updateLock;
private static int DISCOVERY_PORT = 7331;
private static int MAX_DISCONNECT_SECONDS = 10;
private static long EXPIRY_TIMER_MSEC = 1000L;
private DiscoveryListenerThread _dlt;

private static double overallBrightnessScale = 1.0;
public static boolean useOverallBrightnessScale = false;

private static long totalPower = 0;
private static long totalPowerLimit = -1;
private static double powerScale = 1.0;
private static boolean autoThrottle = false;
private static boolean AntiLog = false;
private static boolean logEnabled = true;
private static int frameLimit = 85;
private static Boolean hasDiscoveryListener = false;
private static Boolean alreadyExist = false;
private boolean expiryEnabled = true;

private Map<String, PixelPusher> pusherMap;
private Map<String, Long> pusherLastSeenMap;

private Timer expiryTimer;

private SceneThread sceneThread;

private TreeMap<Integer, PusherGroup> groupMap;

private TreeSet<PixelPusher> sortedPushers;


public Boolean hasDeviceExpiryTask=false;

*/