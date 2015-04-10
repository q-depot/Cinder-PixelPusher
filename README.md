#Cinder-PixelPusher

Cinder library to control the [Heroic Robotics](http://www.heroicrobotics.com/) PixelPusher device.

##Status
This is the first implementation of the library which is not fully tested.  
At the moment I have one PixelPusher device and I only tested it with the [apa102(aka BetterPixel)](http://www.illumn.com/pixelpusher-and-led-strips/pixelpusher-strip-betterpixel-72-ip67.html) chip led strips.

If you have multiple devices or a different led strip please let me know whether it works and report any problem on the [Github issues page](https://github.com/q-depot/Cinder-PixelPusher/issues). 


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
`PusherDiscoveryService` offers two methods to access the devices, you can get all the devices using `getPushers()` or get the groups with `getGroups()` and then iterate through devices.

#####Set pixels and PixelMap
There are two methods to set the pixels, you can iterate through strips and pixels or you can pass a Surface8u object to the PixelPusher device, this method use a PixelMap which lets you decide how  the source image is mapped to the strips.  
The PixelMap takes the offset(initial position) and the orientation( MAP_LEFT_RIGHT, MAP_RIGHT_LEFT, MAP_TOP_DOWN, MAP_BOTTOM_UP ).  
The default PixelMap is MAP_LEFT_RIGHT and offset(0,0), to change it you can either call set `PixelPusher::setPixelMap()` or `Strip::setPixelMap()`.  
Given an initial position, the `PixelPusher::setPixelMap()` always arrange the vertical strips from left to right and the horizontal strips from top to bottom.  
`Strip::setPixelMap()` instead allows you to decide the arrange, for example you could have a bunch of vertical strips starting from the last column in the image and going left.

```
// given a bunch of strips, set the color for strip "k", pixel "i"
strips[k]->setPixel( i, col.r * 255, col.g * 255, col.b * 255 );

// or set all the strips and pixels for the Pusher "k" using a Surface8u
Surface8u mOutputSurf;
// .. draw something
pushers[k]->setPixels( &mOutputSurf );
```

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
* proper color correction


--

### License
The MIT License (MIT)

Copyright (c) 2015 Nocte Studio Ltd. - [www.nocte.co.uk](http://www.nocte.co.uk)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.