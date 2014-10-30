#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TextureFont.h"

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
    gl::TextureFontRef          mFontBig, mFontSmall;
    Surface8u                   mOutputSurf;
};


void BasicSampleApp::setup()
{
    mPusherDiscoveryService = PusherDiscoveryService::create( io_service() );
    
    mFontBig    = gl::TextureFont::create( Font( "Arial", 16 ) );
    mFontSmall  = gl::TextureFont::create( Font( "Arial", 12 ) );
    
    mOutputSurf = Surface8u( 480, 500, false );
    
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
    
//    else if ( code == KeyEvent::KEY_UP )
//    {
//        std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
//        for( size_t k=0; k < pushers.size(); k++ )
//            pushers[k]->increaseExtraDelay( 1 );
//    }
//    
//    else if ( code == KeyEvent::KEY_DOWN )
//    {
//        std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
//        for( size_t k=0; k < pushers.size(); k++ )
//            pushers[k]->decreaseExtraDelay( 1 );
//    }

}


void BasicSampleApp::update()
{
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
    std::vector<StripRef>       strips;
    std::vector<PixelRef>       pixels;
    ColorA                      col;
 
    
    for( size_t j=0; j < pushers.size(); j++ )
    {
        strips = pushers[j]->getStrips();
        for( size_t k=0; k < strips.size(); k++ )
        {
            pixels = strips[k]->getPixels();
            for( size_t i=0; i < pixels.size(); i++ )
            {
                col.r = 0.5f * ( 1.0f + sin( (float)( i + k * pixels.size() ) / 15 + 2 * getElapsedSeconds() ) );
                col.g = 0.5f * ( 1.0f + cos( (float)( i + k * pixels.size() ) / 15 + 2 * getElapsedSeconds() ) );
                col.b = 1.0 - 0.5f * ( 1.0f + cos( (float)( i + k * pixels.size() ) / 5 + 2 * getElapsedSeconds() ) );
                
                strips[k]->setPixel( i, (uint8_t)( col.r * 255 ), (uint8_t)( col.g * 255 ), (uint8_t)( col.b * 255 ) );
                
                mOutputSurf.setPixel( Vec2i( i, j * 8 + k ), col );
            }
        }
    }
}


void BasicSampleApp::draw()
{
    gl::enableAlphaBlending();
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::draw( mOutputSurf, Vec2f( getWindowWidth() - mOutputSurf.getWidth(), 0 ) );
    
    Vec2i                       pos( 15, 25 );
    int                         lineH = 18;
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
    std::vector<PusherGroupRef> groups  = mPusherDiscoveryService->getGroups();
    PixelPusherRef              pusher;
    
    mFontBig->drawString( "FPS: " + to_string( getAverageFps() ),   pos );   pos.y += lineH * 2;
    mFontBig->drawString( "Pusher Discovery Service",               pos );   pos.y += lineH * 1.5;
    
    mFontSmall->drawString( "Globabl brightness:\t\t"   + to_string( PusherDiscoveryService::getGlobalBrightness() ),   pos );   pos.y += lineH;
    mFontSmall->drawString( "Total power:\t\t\t"        + to_string( PusherDiscoveryService::getTotalPower() ),         pos );   pos.y += lineH;
    mFontSmall->drawString( "Total power limit:\t\t"    + to_string( PusherDiscoveryService::getTotalPowerLimit() ),    pos );   pos.y += lineH;
    mFontSmall->drawString( "Power scale:\t\t\t"        + to_string( PusherDiscoveryService::getPowerScale() ),         pos );   pos.y += lineH;
    mFontSmall->drawString( "AntiLog:\t\t\t\t"          + to_string( PusherDiscoveryService::getAntiLog() ),            pos );   pos.y += lineH;
    mFontSmall->drawString( "Frame limit:\t\t\t"        + to_string( PusherDiscoveryService::getFrameLimit() ),         pos );   pos.y += lineH;
    mFontSmall->drawString( "Auto throttle:\t\t\t"      + to_string( PusherDiscoveryService::getAutoThrottle() ),       pos );   pos.y += lineH;
    mFontSmall->drawString( "Total Devices:\t\t\t"      + to_string( pushers.size() ),                                  pos );   pos.y += lineH;
    mFontSmall->drawString( "Total groups:\t\t\t"       + to_string( groups.size() ),                                   pos );   pos.y += lineH;

    pos.y += lineH;
    
    for( size_t j=0; j < pushers.size(); j++ )
    {
        pusher = pushers[j];
        
        // Network
        mFontSmall->drawString( "IP: "                   + pusher->getIp(),                              pos );   pos.y += lineH;
        mFontSmall->drawString( "Mac: "                  + pusher->getMacAddress(),                      pos );   pos.y += lineH;
        mFontSmall->drawString( "Port: "                 + to_string( pusher->getPort() ),               pos );   pos.y += lineH;
        mFontSmall->drawString( "Link speed: "           + to_string( pusher->getLinkSpeed() ),          pos );   pos.y += lineH;
        mFontSmall->drawString( "Multicast: "            + to_string( pusher->isMulticast() ),           pos );   pos.y += lineH;
        mFontSmall->drawString( "Multicast primary: "    + to_string( pusher->isMulticastPrimary() ),    pos );   pos.y += lineH;
        
        // Artnet
        mFontSmall->drawString( "Artnet universe: "      + to_string( pusher->getArtnetUniverse() ),     pos );   pos.y += lineH;
        mFontSmall->drawString( "Artnet channel: "       + to_string( pusher->getArtnetChannel() ),      pos );   pos.y += lineH;

        pos.y += lineH;
        
        // Group
        mFontSmall->drawString( "Group: "                + to_string( pusher->getGroupId() ),            pos );   pos.y += lineH;
        mFontSmall->drawString( "Controller: "           + to_string( pusher->getControllerId() ),       pos );   pos.y += lineH;
        mFontSmall->drawString( "Delta sequence: "       + to_string( pusher->getDeltaSequence() ),      pos );   pos.y += lineH;
        mFontSmall->drawString( "Device type: "          + to_string( pusher->getDeviceType() ),         pos );   pos.y += lineH;
        mFontSmall->drawString( "Extra delay: "          + to_string( pusher->getExtraDelay() ),         pos );   pos.y += lineH;
        mFontSmall->drawString( "Thread sleep for: "     + to_string( pusher->getThreadSleepFor() ),     pos );   pos.y += lineH;
        
        pos.y += lineH;
        
        // Software/hardware
        mFontSmall->drawString( "Software rev: "         + to_string( pusher->getSoftwareRevision() ),   pos );   pos.y += lineH;
        mFontSmall->drawString( "Hardware rev: "         + to_string( pusher->getHardwareRevision() ),   pos );   pos.y += lineH;
        mFontSmall->drawString( "Vendor id: "            + to_string( pusher->getVendorId() ),           pos );   pos.y += lineH;
        mFontSmall->drawString( "Product id: "           + to_string( pusher->getProductId() ),          pos );   pos.y += lineH;
        mFontSmall->drawString( "Protocol ver: "         + to_string( pusher->getProtocolVersion() ),    pos );   pos.y += lineH;
        
        pos = Vec2i( 250, 25 );
        
        // strips
        mFontSmall->drawString( "Strips attached: "      + to_string( pusher->getStripsAttached() ),     pos );   pos.y += lineH;
        mFontSmall->drawString( "Max strips/packet: "    + to_string( pusher->getMaxStripsPerPacket() ), pos );   pos.y += lineH;
        mFontSmall->drawString( "Num strips: "           + to_string( pusher->getNumStrips() ),          pos );   pos.y += lineH;
        mFontSmall->drawString( "Pixels per strip: "     + to_string( pusher->getPixelsPerStrip() ),     pos );   pos.y += lineH;

        pos.y += lineH;
        
        mFontSmall->drawString( "Last universe: "        + to_string( pusher->getLastUniverse() ),       pos );   pos.y += lineH;
        mFontSmall->drawString( "Power domain: "         + to_string( pusher->getPowerDomain() ),        pos );   pos.y += lineH;
        mFontSmall->drawString( "Power total: "          + to_string( pusher->getPowerTotal() ),         pos );   pos.y += lineH;
        mFontSmall->drawString( "Pusher flags: "         + to_string( pusher->getPusherFlags() ),        pos );   pos.y += lineH;
        mFontSmall->drawString( "Segments: "             + to_string( pusher->getSegments() ),           pos );   pos.y += lineH;
        mFontSmall->drawString( "Update period: "        + to_string( pusher->getUpdatePeriod() ),       pos );   pos.y += lineH;
        
        pos.y += lineH;
        mFontSmall->drawString( "Packet number: "        + to_string( pusher->getPacketNumber() ),       pos );   pos.y += lineH;
    }

}

CINDER_APP_NATIVE( BasicSampleApp, RendererGl )
