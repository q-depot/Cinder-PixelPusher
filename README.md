#Cinder-PixelPusher

Cinder library to control the [Heroic Robotics](http://www.heroicrobotics.com/) PixelPusher device.

This is the first implementation of the library which is not fully tested, I only have one device at the moment, if you have multiple devices please let me know whether it works.

If there is any problem please report it on the Github issues. 


##Requirements

* Cinder >= 0.8.6
* [Cinder-Asio](https://github.com/BanTheRewind/Cinder-Asio)


##Overview

The main entry point is the `PusherDiscoveryService`.  

`PusherDiscoveryService` open a UDP socket on port `PP_DISCOVERY_SERVICE_PORT` listening for boadcast message sent by the PixelPusher devices.

#####Discovery
`PusherDiscoveryService` manages all the devices connected to the network, it automatically discover, create, group and destroy the available PixelPusher devices.

Each PixelPusher device broadcast a packet every second to send some informations about the device itself, the `PusherDiscoveryService::onRead()` method is used to discover new devices and update existing ones.

#####Timeout
If a device doesn't broadcast for more than `PP_DISCONNECT_TIMEOUT` seconds, `PusherDiscoveryService` will automatically destroy it also removing any empty groups.

#####Framerate
The framerate should change automatically depending on your network, PixelPusher and so Cinder-PixelPusher always aim to send data as fast as possible and in theory it should be about 60fps which is the target frame rate set `PusherDiscoveryService::FrameLimit`, use PusherDiscoveryService::setFrameLimit() to change the default framerate of `60fps`.
By default `PusherDiscoveryService` is set with AutoThrottle which adjust the speed by increasing and decresing a delay, to change this behavior you can use the static method `PusherDiscoveryService::setAutoThrottle()`


#####PixelPusher Groups and Controllers
The PixelPusher network is organised in Groups and Controllers. Each controller is always associated to a group, this is a property of the device itself and among other properties, it can only be changed by updating the PixelPusher configuration. To configure the device you can use the [PixelPusher-utilities](https://github.com/jasstrong/PixelPusher-utilities) configtool, please refer to the [PixelPusher Documentation](https://sites.google.com/a/heroicrobot.com/pixelpusher/home).

`PusherDiscoveryService` offers two methods to access the devices, you can get all the devices using `getPushers()` or get the groups with `getGroups()` and then iterate through the group's devices.

For more info about PixelPusher visit:  
* [Heroic Robotics](http://www.heroicrobotics.com)  
* [PixelPusher Documentation](https://sites.google.com/a/heroicrobot.com/pixelpusher/home)


##How to use it

```
// include the header
#include "PusherDiscoveryService.h"


class BasicSampleApp : public AppNative {
  public:

	// declare a Ref(shared_ptr) object
    PusherDiscoveryServiceRef   mPusherDiscoveryService;
    
};
```

```
void BasicSampleApp::setup()
{
	// create a new PusherDiscoveryObject passing the App io service
    mPusherDiscoveryService = PusherDiscoveryService::create( io_service() );
}
```

```
void BasicSampleApp::update()
{
	// get all the PixelPusher devices
	std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();

	// or get the Pusher Groups
	std::vector<PusherGroupRef> groups = mPusherDiscoveryService->getGroups();


    std::vector<StripRef>       strips;
    std::vector<PixelRef>       pixels;
    Color                       col;
    
    if ( !pushers.empty() )
    {
        for( size_t j=0; j < pushers.size(); j++ )
        {
	        // get the Strips for this device
	        strips = pushers[k]->getStrips();

	        for( size_t k=0; k < strips.size(); k++ )
	        {
	        	// get the pixels for this strip
	            pixels = strips[k]->getPixels();

	            // finally iterate through each pixel
	            for( size_t i=0; i < pixels.size(); i++ )
	            {
	                col.r = 0.5f * ( 1.0f + sin( (float)( i + k * pixels.size() ) / 15 + 2 * getElapsedSeconds() ) );
	                col.g = 0.5f * ( 1.0f + cos( (float)( i + k * pixels.size() ) / 15 + 2 * getElapsedSeconds() ) );
	                col.b = 1.0 - 0.5f * ( 1.0f + cos( (float)( i + k * pixels.size() ) / 5 + 2 * getElapsedSeconds() ) );
	                
	                // set the RGB color
	                strips[k]->setPixel( i, (uint8_t)( col.r * 255 ), (uint8_t)( col.g * 255 ), (uint8_t)( col.b * 255 ) );

	                // or set the RGBOW color ( rgb + orange and white)
	                // strips[k]->setPixel( i, (uint8_t)( col.r * 255 ), (uint8_t)( col.g * 255 ), (uint8_t)( col.b * 255 ), 0, 0 );
	            }
	        }
	    }
    }

}
```
