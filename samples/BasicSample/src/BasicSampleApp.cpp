#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TextureFont.h"
#include "Strip.h"
#include "PusherDiscoveryService.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class BasicSampleApp : public AppNative {
  public:
	void setup();
	void keyDown( KeyEvent event );
	void update();
	void draw();
    
    PusherDiscoveryServiceRef   mPusherDiscoveryService;
    gl::TextureFontRef  mFont;
    
};


void BasicSampleApp::setup()
{
    mPusherDiscoveryService = PusherDiscoveryService::create( io_service() );

    mFont = gl::TextureFont::create( Font( "Arial", 12 ) );
    
    setWindowSize( 1200, 800 );
}


void BasicSampleApp::keyDown( KeyEvent event )
{
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
    if ( pushers.empty() )
        return;
    
    int code = event.getCode();
    
    if ( code == KeyEvent::KEY_r )
        pushers.front()->reset();
}


void BasicSampleApp::update()
{
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
    std::vector<StripRef>       strips;
    std::vector<PixelRef>       pixels;
    Color                       col;
    
    if ( !pushers.empty() )
    {
        strips = pushers.front()->getStrips();
        for( size_t k=0; k < strips.size(); k++ )
        {
            pixels = strips[k]->getPixels();
            for( size_t i=0; i < pixels.size(); i++ )
            {
                col.r = 0.5f * ( 1.0f + sin( (float)( i + k * pixels.size() ) / 15 + 2 * getElapsedSeconds() ) );
                col.g = 0.5f * ( 1.0f + cos( (float)( i + k * pixels.size() ) / 15 + 2 * getElapsedSeconds() ) );
                col.b = 1.0 - 0.5f * ( 1.0f + cos( (float)( i + k * pixels.size() ) / 5 + 2 * getElapsedSeconds() ) );
                
                strips[k]->setPixel( i, (uint8_t)( col.r * 255 ), (uint8_t)( col.g * 255 ), (uint8_t)( col.b * 255 ) );
            }
        }
    }
}


void BasicSampleApp::draw()
{
    gl::enableAlphaBlending();
	gl::clear( Color( 0, 0, 0 ) );
    
    Vec2i                       pos( 15, 25 );
    int                         lineH = 15;
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
    PixelPusherRef              pusher;

    mFont->drawString( "FPS: " + to_string(getAverageFps()), Vec2i( 450, 25 ) );
    
    for( size_t j=0; j < pushers.size(); j++ )
    {
        pusher = pushers[j];
        
        // Network
        mFont->drawString( "IP: "                   + pusher->getIp(),                              pos );   pos.y += lineH;
        mFont->drawString( "Mac: "                  + pusher->getMacAddress(),                      pos );   pos.y += lineH;
        mFont->drawString( "Port: "                 + to_string( pusher->getPort() ),               pos );   pos.y += lineH;
        mFont->drawString( "Link speed: "           + to_string( pusher->getLinkSpeed() ),          pos );   pos.y += lineH;
        mFont->drawString( "Multicast: "            + to_string( pusher->isMulticast() ),           pos );   pos.y += lineH;
        mFont->drawString( "Multicast primary: "    + to_string( pusher->isMulticastPrimary() ),    pos );   pos.y += lineH;
        
        // Artnet
        mFont->drawString( "Artnet universe: "      + to_string( pusher->getArtnetUniverse() ),     pos );   pos.y += lineH;
        mFont->drawString( "Artnet channel: "       + to_string( pusher->getArtnetChannel() ),      pos );   pos.y += lineH;

        pos.y += lineH;
        
        // Group
        mFont->drawString( "Group: "                + to_string( pusher->getGroupId() ),            pos );   pos.y += lineH;
        mFont->drawString( "Controller: "           + to_string( pusher->getControllerId() ),       pos );   pos.y += lineH;
        mFont->drawString( "Delta sequence: "       + to_string( pusher->getDeltaSequence() ),      pos );   pos.y += lineH;
        mFont->drawString( "Device type: "          + to_string( pusher->getDeviceType() ),         pos );   pos.y += lineH;
        mFont->drawString( "Extra delay: "          + to_string( pusher->getExtraDelay() ),         pos );   pos.y += lineH;
        mFont->drawString( "Thread sleep for: "     + to_string( pusher->getThreadSleepFor() ),     pos );   pos.y += lineH;
        mFont->drawString( "Auto throttle: "        + to_string( pusher->getAutoThrottle() ),       pos );   pos.y += lineH;
        
        pos.y += lineH;
        
        // Software/hardware
        mFont->drawString( "Software rev: "         + to_string( pusher->getSoftwareRevision() ),   pos );   pos.y += lineH;
        mFont->drawString( "Hardware rev: "         + to_string( pusher->getHardwareRevision() ),   pos );   pos.y += lineH;
        mFont->drawString( "Vendor id: "            + to_string( pusher->getVendorId() ),           pos );   pos.y += lineH;
        mFont->drawString( "Product id: "           + to_string( pusher->getProductId() ),          pos );   pos.y += lineH;
        mFont->drawString( "Protocol ver: "         + to_string( pusher->getProtocolVersion() ),    pos );   pos.y += lineH;
        
        pos = Vec2i( 250, 25 );
        
        // strips
        mFont->drawString( "Strips attached: "      + to_string( pusher->getStripsAttached() ),     pos );   pos.y += lineH;
        mFont->drawString( "Max strips/packet: "    + to_string( pusher->getMaxStripsPerPacket() ), pos );   pos.y += lineH;
        mFont->drawString( "Num strips: "           + to_string( pusher->getNumStrips() ),          pos );   pos.y += lineH;
        mFont->drawString( "Pixels per strip: "     + to_string( pusher->getPixelsPerStrip() ),     pos );   pos.y += lineH;

        pos.y += lineH;
        
        mFont->drawString( "Last universe: "        + to_string( pusher->getLastUniverse() ),       pos );   pos.y += lineH;
        mFont->drawString( "Power domain: "         + to_string( pusher->getPowerDomain() ),        pos );   pos.y += lineH;
        mFont->drawString( "Power total: "          + to_string( pusher->getPowerTotal() ),         pos );   pos.y += lineH;
        mFont->drawString( "Pusher flags: "         + to_string( pusher->getPusherFlags() ),        pos );   pos.y += lineH;
        mFont->drawString( "Segments: "             + to_string( pusher->getSegments() ),           pos );   pos.y += lineH;
        mFont->drawString( "Update period: "        + to_string( pusher->getUpdatePeriod() ),       pos );   pos.y += lineH;
        
        pos.y += lineH;
        mFont->drawString( "Packet number: "        + to_string( pusher->getPacketNumber() ),       pos );   pos.y += lineH;
    }

}

CINDER_APP_NATIVE( BasicSampleApp, RendererGl )
