#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "UdpServer.h"
#include "DeviceHeader.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TestDiscoveryThreadApp : public AppNative {
    
  public:

	void setup();
	void draw();

    void accept();
	void onAccept( UdpSessionRef session );
	void onError( std::string err, size_t bytesTransferred );
	void onRead( ci::Buffer buffer );
	void onReadComplete();
	void onWrite( size_t bytesTransferred );
    
	int32_t						mPort;
	int32_t						mPortPrev;
	UdpServerRef				mServer;
	UdpSessionRef				mSession;

};


void TestDiscoveryThreadApp::setup()
{
	gl::enableAlphaBlending();
    
	mPort		= 7331;
	
	// Initialize a server by passing a boost::asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it
	// manually (i.e., call poll(), poll_one(), run(), etc).
	mServer = UdpServer::create( io_service() );
    
	// Add callbacks to work with the server asynchronously.
	mServer->connectAcceptEventHandler( &TestDiscoveryThreadApp::onAccept, this );
	mServer->connectErrorEventHandler(  &TestDiscoveryThreadApp::onError, this );
	
	// Start listening.
	accept();
}


void TestDiscoveryThreadApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}



void TestDiscoveryThreadApp::accept()
{
	if ( mSession ) {
		mSession.reset();
	}
	if ( mServer ) {
		// This is how you start listening for a connection. Once
		// a connection occurs, a session will be created and passed
		// in through the onAccept method.
		mServer->accept( (uint16_t)mPort );
        
        console() << "Listening on port: " << mPort << endl;
	}
}


void TestDiscoveryThreadApp::onAccept( UdpSessionRef session )
{
	// Get the session from the argument and set callbacks.
	mSession = session;
	mSession->connectErrorEventHandler(         &TestDiscoveryThreadApp::onError,           this );
	mSession->connectReadCompleteEventHandler(  &TestDiscoveryThreadApp::onReadComplete,    this );
	mSession->connectReadEventHandler(          &TestDiscoveryThreadApp::onRead,            this );
	mSession->connectWriteEventHandler(         &TestDiscoveryThreadApp::onWrite,           this );
    
	// Start reading data from the client.
	mSession->read();
}


void TestDiscoveryThreadApp::onError( string err, size_t bytesTransferred )
{
	string text = "Error";
	if ( !err.empty() ) {
		text += ": " + err;
	}
    
    console() << "ERROR: " << text << endl;
}


void TestDiscoveryThreadApp::onRead( ci::Buffer buffer )
{
	// Data is packaged as a ci::Buffer. This allows
	// you to send any kind of data. Because it's more common to
	// work with strings, the session object has static convenience
	// methods for converting between std::string and ci::Buffer.
	string response	= UdpSession::bufferToString( buffer );
    
    uint8_t *data = (uint8_t*)buffer.getData();

    DeviceHeader    header  = DeviceHeader( data, buffer.getDataSize() );
    std::string     macAddr = header.getMacAddressString();
    
    if ( header.getDeviceType() != DeviceType::PIXELPUSHER ) {
        console() << "Ignoring non-PixelPusher discovery packet from " << endl;
//                        + header.toString());
        
        return;
    }
    
    console() << "received "        << getElapsedSeconds() << endl;
    
    console() << "ip: "             << header.getIpAddressString() << endl;
    console() << "mac: "            << header.getMacAddressString() << endl;
    console() << "type: "           << header.getDeviceType() << endl;
    console() << "HW rev: "         << header.getHardwareRevision() << endl;
    console() << "link speed: "     << header.getLinkSpeed() << endl;
    console() << "vendor id: "      << header.getVendorId()<< endl;
    console() << "product id: "     << header.getProductId()<< endl;
    console() << "protocol ver: "   << header.getProtocolVersion() << endl;
    console() << "SW rev: "         << header.getSoftwareRevision() << endl;
    console() << endl;
    
//    console() << getElapsedSeconds() << " READ: " << buffer.getDataSize() << endl;
//    for( int k=0; k < buffer.getDataSize(); k++ )
//        console() << to_string(data[k]) << " ";
//    
//    console() << endl;
    
	// Continue reading.
	mSession->read();
}


void TestDiscoveryThreadApp::onReadComplete()
{
	// Continue reading new responses.
	mSession->read();
}


void TestDiscoveryThreadApp::onWrite( size_t bytesTransferred )
{
	// Read after writing to look for a response.
	mSession->read();
}



CINDER_APP_NATIVE( TestDiscoveryThreadApp, RendererGl )

//////////////////////////////////////////////////////////
/*
public void receive(byte[] data) {

    DeviceHeader    header  = DeviceHeader(data);
    std::string     macAddr = header.GetMacAddressString();
    
    if (header.DeviceType != DeviceType.PIXELPUSHER) {
        if (logEnabled)
            LOGGER.fine("Ignoring non-PixelPusher discovery packet from "
                        + header.toString());
        return;
    }
    PixelPusher device = new PixelPusher(header.PacketRemainder, header);
    device.setAntiLog(AntiLog);
    
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

*/