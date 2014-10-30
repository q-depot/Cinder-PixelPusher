#Cinder-PixelPusher

Cinder library to control the [Heroic Robotics](http://www.heroicrobotics.com/) PixelPusher device.

##Status
This is the first implementation of the library which is not fully tested.  
At the moment I have one PixelPusher device and I only tested it with the [apa102(aka BetterPixel)](http://www.illumn.com/pixelpusher-and-led-strips/pixelpusher-strip-betterpixel-72-ip67.html) chip led strips.

If you have multiple devices or a different led strip please let me know whether it works and report any problem on the Github issues page. 


##Requirements

* Cinder >= 0.8.6
* [Cinder-Asio](https://github.com/BanTheRewind/Cinder-Asio)


##Overview

The main entry point is the `PusherDiscoveryService`.  

`PusherDiscoveryService` open a UDP socket on port `PP_DISCOVERY_SERVICE_PORT` listening for broadcast messages sent by the PixelPusher devices.

#####Discovery
`PusherDiscoveryService` manages all the devices connected to the network, it automatically discover, create, group and destroy the available PixelPusher devices.

Each PixelPusher device broadcast a UDP packet every second to send some informations about the device itself, the `PusherDiscoveryService::onRead()` method is used to discover new devices and update the existing ones.

#####Timeout
If a device doesn't broadcast for more than `PP_DISCONNECT_TIMEOUT` seconds, `PusherDiscoveryService` will automatically destroy it also removing any empty groups.

#####Color correction
There is a basic feature to apply color correction, if the colors look a bit washed out you can enable it with `PusherDiscoveryService::enableColorCorrection( bool isEnable )`.

#####PixelPusher Groups and Controllers
The PixelPusher network is organised in Groups and Controllers. Each controller is always associated to a group, this is a property of the device itself and among other properties, it can only be changed by updating the PixelPusher configuration. To configure the device you can use the [PixelPusher-utilities](https://github.com/jasstrong/PixelPusher-utilities) configtool, please refer to the [PixelPusher Documentation](https://sites.google.com/a/heroicrobot.com/pixelpusher/home).

#####Get the devices
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
    std::vector<StripRef>	strips;
    std::vector<PixelRef>	pixels;
    Color					col;

	// get the PixelPusher devices
	std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();

	// or get the Pusher Groups
	std::vector<PusherGroupRef> groups = mPusherDiscoveryService->getGroups();
    
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
```

##TODO

* Implement multicast
* Test multiple devices
* Test strips: MBI6030, LPD8806, SD600A, WS2801, P9813, TLC59711

