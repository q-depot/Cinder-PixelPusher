
#include "cinder/app/AppNative.h"
#include "DeviceRegistry.h"
#include "PixelPusher.h"


DeviceRegistry::DeviceRegistry( boost::asio::io_service& ioService )
{
    // Initialize a server by passing a boost::asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it
	// manually (i.e., call poll(), poll_one(), run(), etc).
	mServer = UdpServer::create( ioService );
    
    getApp
	// Add callbacks to work with the server asynchronously.
	mServer->connectAcceptEventHandler( &DeviceRegistry::onAccept,  this );
	mServer->connectErrorEventHandler(  &DeviceRegistry::onError,   this );

    if ( mServer )
    {
        // This is how you start listening for a connection. Once
        // a connection occurs, a session will be created and passed
        // in through the onAccept method.
        mServer->accept( (uint16_t)DR_DISCOVERY_PORT );
        
        ci::app::console() << "Listening on port: " << DR_DISCOVERY_PORT << std::endl;
    }
    
    else
    {
        ci::app::console() << "DeviceRegistry failed to create a server" << std::endl;
    }
}


void DeviceRegistry::onAccept( UdpSessionRef session )
{
    // Get the session from the argument and set callbacks.
    mSession = session;
    mSession->connectErrorEventHandler( &DeviceRegistry::onError,   this );
    mSession->connectReadEventHandler(  &DeviceRegistry::onRead,    this );
    
    // Start reading data from the client.
    mSession->read();
}


void DeviceRegistry::onError( std::string err, size_t bytesTransferred )
{
    ci::app::console() << "DeviceRegistry socket error!" << std::endl;
}


void DeviceRegistry::onRead( ci::Buffer buffer )
{
    uint8_t         *data   =(uint8_t*)buffer.getData();
    DeviceHeader    header  = DeviceHeader( data, buffer.getDataSize() );
    std::string     macAddr = header.getMacAddressString();
    
    // ignore non-PixelPusher devices
    if ( header.getDeviceType() != DeviceType::PIXELPUSHER )
        return;
    
    PixelPusherRef device = PixelPusher::create( header);
    device->setAntiLog(AntiLog);
    
    ///////
    PixelPusher( uint8_t *packet, int packetSize, DeviceHeader header );
    
    
    
    
    
    
    
    // Set the timestamp for the last time this device checked in
    pusherLastSeenMap.put(macAddr, System.nanoTime());
    if (!pusherMap.containsKey(macAddr)) {
        // We haven't seen this device before
        addNewPusher(macAddr, device);
    } else {
        if (!pusherMap.get(macAddr).equals(device)) { // we already saw it but it's changed.
            updatePusher(macAddr, device);
        } else {
            // The device is identical, nothing important has changed
            if (logEnabled) {
                LOGGER.fine("Device still present: " + macAddr);
                System.out.println("Updating pusher from bcast.");
            }
            pusherMap.get(macAddr).updateVariables(device);
            // if we dropped more than occasional packets, slow down a little
            if (device.getDeltaSequence() > 3)
                pusherMap.get(macAddr).increaseExtraDelay(5);
            if (device.getDeltaSequence() < 1)
                pusherMap.get(macAddr).decreaseExtraDelay(1);
            if (logEnabled)
                System.out.println(device.toString());
        }
    }
    
    // update the power limit variables
    if (totalPowerLimit > 0) {
        totalPower = 0;
        for (Iterator<PixelPusher> iterator = sortedPushers.iterator(); iterator
             .hasNext();) {
            PixelPusher pusher = iterator.next();
            totalPower += pusher.getPowerTotal();
        }
        if (totalPower > totalPowerLimit) {
            powerScale = totalPowerLimit / totalPower;
        } else {
            powerScale = 1.0;
        }
    }
    updateLock.release();
}

    
}
