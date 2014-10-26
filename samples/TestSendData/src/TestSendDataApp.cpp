#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "UdpClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TestSendDataApp : public AppNative {
  public:
	void setup();
	void keyDown( KeyEvent event );
	void update();
	void draw();
    
	void onConnect( UdpSessionRef session );
	void onError( std::string err, size_t bytesTransferred );
    void write();
    
	UdpClientRef				mClient;
	std::string					mHost;
	int32_t						mPort;
	UdpSessionRef				mSession;
    int                         mStripsN;
    int                         mPixelsPerStrip;
    Color                       mColor;
    uint64_t                    mPacketNumber;
	ci::params::InterfaceGlRef	mParams;

};

void TestSendDataApp::setup()
{
//    if (packet.length > 28 && super.getSoftwareRevision() > 100) {
//        my_port = (int) ByteUtils.unsignedShortToInt(Arrays.copyOfRange(packet, 28, 30));
//    } else {
//        my_port = 9798;
    
        
	mHost           = "192.168.1.167";
	mPort           = 9897;
    
    mStripsN        = 1;
    mPixelsPerStrip = 480;
    mColor          = Color( 1.0, 0.0, 0.0 );
    mPacketNumber   = 0;
    
    mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Host",              &mHost );
	mParams->addParam( "Port",              &mPort, "min=0 max=65535 step=1" );
	mParams->addParam( "StripsN",			&mStripsN, "min=0 max=8 step=1" );
	mParams->addParam( "PixelsPerStrip",	&mPixelsPerStrip, "min=0 max=480" );
	mParams->addParam( "Color",             &mColor);
    
	// Initialize a client by passing a boost::asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it
	// manually (i.e., call poll(), poll_one(), run(), etc).
	mClient = UdpClient::create( io_service() );
    
	// Add callbacks to work with the client asynchronously.
	// Note that you can use lambdas.
	mClient->connectConnectEventHandler(    &TestSendDataApp::onConnect, this );
	mClient->connectErrorEventHandler(      &TestSendDataApp::onError, this );
}


void TestSendDataApp::keyDown( KeyEvent event )
{
    int code = event.getCode();
    
    if ( code == KeyEvent::KEY_SPACE )
        write();
}


void TestSendDataApp::update()
{
}

void TestSendDataApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    mParams->draw();
}


void TestSendDataApp::onConnect( UdpSessionRef session )
{
	console() << "Connected" << endl;
    
	mSession = session;
	mSession->connectErrorEventHandler( &TestSendDataApp::onError, this );
    
	write();
}


void TestSendDataApp::onError( string err, size_t bytesTransferred )
{
    console() << "TestSendDataApp::onError " << err << endl;
}


void TestSendDataApp::write()
{
	// This sample is meant to work with only one session at a time.
	if ( mSession && mSession->getSocket()->is_open() ) {
        while( true )
        {
            // Write data is packaged as a ci::Buffer. This allows
            // you to send any kind of data. Because it's more common to
            // work with strings, the session object has static convenience
            // methods for converting between std::string and ci::Buffer.
            
            Buffer buff(  ( 4 + 1 + mPixelsPerStrip * 3 ) );
            uint8_t *data = (uint8_t*)buff.getData();
            
            int packetLength = 0;
            
    //        for( size_t k=0; k < mStripsN; k++ )
            {
                // packet number
                data[packetLength++] = mPacketNumber & 0xFF;
                data[packetLength++] = (mPacketNumber >> 8) & 0xFF;
                data[packetLength++] = (mPacketNumber >> 16) & 0xFF;
                data[packetLength++] = (mPacketNumber >> 24) & 0xFF;
                
                data[packetLength++] = 0;   // strip N
                
                // process pixels
                float val;
                for ( size_t i=0; i < mPixelsPerStrip; i++ )
                {
                    val = (float)i / (float)mPixelsPerStrip;
                    
                    Color col;
                    col.r = 0.5f * ( 1.0f + sin( (float)i / 15 + 2 * getElapsedSeconds() ) );
                    col.g = 0.5f * ( 1.0f + cos( (float)i / 15 + 2 * getElapsedSeconds() ) );
                    col.b = 1.0 - 0.5f * ( 1.0f + cos( (float)i / 5 + 2 * getElapsedSeconds() ) );
//                    val = 0.5f * ( 1.0f + sin( i + 2 * getElapsedSeconds() ) );
//                    col = Color( CM_HSV, Vec3f( 1.0- val, 1.0, val ) );
                    
                    data[packetLength++] = (uint8_t)( col.r * 255 );// & 0xff;
                    data[packetLength++] = (uint8_t)( col.g * 255 );// & 0xff;
                    data[packetLength++] = (uint8_t)( col.b * 255 );// & 0xff;
                }
            }
            
            mPacketNumber++;
            
            mSession->write( buff );
            
            data[0] = mPacketNumber & 0xFF;
            data[1] = (mPacketNumber >> 8) & 0xFF;
            data[2] = (mPacketNumber >> 16) & 0xFF;
            data[3] = (mPacketNumber >> 24) & 0xFF;
            data[4] = 1;   // strip N
            
            mPacketNumber++;
            
            mSession->write( buff );
            ci::sleep( 15 );
        }
	} else {
		// Before we can write, we need to establish a connection
		// and create a session. Check out the onConnect method.
        console() << "Connecting to: " << mHost << ":" << to_string( mPort ) << endl;
		mClient->connect( mHost, (uint16_t)mPort );
	}
}


CINDER_APP_NATIVE( TestSendDataApp, RendererGl )


