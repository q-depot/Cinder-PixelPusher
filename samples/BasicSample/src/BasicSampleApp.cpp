#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/params/Params.h"
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
    
    params::InterfaceGl         mParams;
    
    PixelPusher::StripMap       mStripMap;
    PixelPusher::StripFlip      mStripFlip;
    
    Vec2f                       mTestPos;
    Vec2f                       mTestPosOff;
    Vec2f                       mTestSpeed;
};


void BasicSampleApp::setup()
{
    mPusherDiscoveryService = PusherDiscoveryService::create( io_service() );
    
    mFontBig    = gl::TextureFont::create( Font( "Arial", 16 ) );
    mFontSmall  = gl::TextureFont::create( Font( "Arial", 12 ) );
    
    mOutputSurf = Surface8u( 180, 180, false );
    mStripMap   = PixelPusher::MAP_STRIP_ROW;
    mStripFlip  = PixelPusher::MAP_FLIP_NONE;
    
    mParams = params::InterfaceGl( "params", Vec2i( 260, 300 ) );
    vector<string> opts; opts.push_back( "Rows to strips" ); opts.push_back( "Cols to strips" );
    mParams.addParam( "Strip map", opts, (int*)&mStripMap );
    
    opts.clear(); opts.push_back( "none" ); opts.push_back( "Flip Y" ); opts.push_back( "Flip X" ); opts.push_back( "Flip XY" );
    mParams.addParam( "Strip flip", opts, (int*)&mStripFlip );
    
    mParams.addParam( "Speed x", &mTestSpeed.x );
    mParams.addParam( "Speed y", &mTestSpeed.y );
    mParams.addParam( "Offset X", &mTestPosOff.x );
    mParams.addParam( "Offset Y", &mTestPosOff.y );
    
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
    
    else if ( code == KeyEvent::KEY_SPACE )
        mTestPos = Vec2i::zero();
}


void BasicSampleApp::update()
{
    mTestPos += mTestSpeed;
    
    if ( mTestPos.x >= mOutputSurf.getWidth() )     mTestPos.x = 0;
    if ( mTestPos.y >= mOutputSurf.getHeight() )    mTestPos.y = 0;
    
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();
    std::vector<StripRef>       strips;
    std::vector<PixelRef>       pixels;
    ColorA                      col;
//    
//    for( size_t j=0; j < pushers.size(); j++ )
//    {
//        strips = pushers[j]->getStrips();
//        for( size_t k=0; k < strips.size(); k++ )
//        {
//            pixels = strips[k]->getPixels();
//            for( size_t i=0; i < pixels.size(); i++ )
//            {
//                col.r = 0.5f * ( 1.0f + sin( (float)( i + k * pixels.size() ) / 15 + 2 * getElapsedSeconds() ) );
//                col.g = 0.5f * ( 1.0f + cos( (float)( i + k * pixels.size() ) / 15 + 2 * getElapsedSeconds() ) );
//                col.b = 1.0 - 0.5f * ( 1.0f + cos( (float)( i + k * pixels.size() ) / 5 + 2 * getElapsedSeconds() ) );
//                
//                strips[k]->setPixel( i, (uint8_t)( col.r * 255 ), (uint8_t)( col.g * 255 ), (uint8_t)( col.b * 255 ) );
//                
//                mOutputSurf.setPixel( Vec2i( i, j * 8 + k ), col );
//            }
//        }
//    }
    
    Vec3f hsv;
    Vec2i pos;
    
    for( size_t x=0; x < mOutputSurf.getWidth(); x++ )
    {
        for( size_t y=0; y < mOutputSurf.getHeight(); y++ )
        {
//            col.r = 0.5f * ( 1.0f + sin( (float)( x + y + 2 * getElapsedSeconds() ) ) );
//            col.g = 0.5f * ( 1.0f + cos( (float)( x + y * 10 ) / 15 + 2 * getElapsedSeconds() ) );
//            col.b = 1.0 - 0.5f * ( 1.0f + cos( (float)( x + y * 1000 ) / 5 + 2 * getElapsedSeconds() ) );
            
//            col.r = 0.5f * ( 1.0f + sin( (float)( x * 0.1 + y * 0.2 + 2 * getElapsedSeconds() ) ) );
//            col.r = (float)x / 480;
//            col.b = (float)y / 480;
            
//            if ( y > mOutputSurf.getHeight() - 325 )
//                col.g = 1.0;
//            else
//                col.g = 0.0;
                pos = Vec2i( x, y );
            if ( pos.x == (int)( mTestPos.x + mTestPosOff.x )  && pos.y == (int)(int)( mTestPos.y + mTestPosOff.y ) )
                col.r = 1;
            else
                col.r = 0;
            
            mOutputSurf.setPixel( pos, col );
        }
    }
    for( size_t k=0; k < pushers.size(); k++ )
    {
        pushers[k]->setPixels( &mOutputSurf, mStripMap, mStripFlip );
        
//        pushers[k]->getStrip(0)->setPixel(0, 0, 0, 255 );
    }
    
    
//    LEFT_RIGHT
//      RIGHT_LEFT
//    TOP_BOTTOM
//    BOTTOM_UP
}


void BasicSampleApp::draw()
{
    gl::enableAlphaBlending();
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::draw( mOutputSurf, Vec2f( getWindowWidth() - mOutputSurf.getWidth(), 0 ) );
    
    Vec2i                       pos( 15, 25 );
    int                         lineH = 16;
    std::vector<PixelPusherRef> pushers = mPusherDiscoveryService->getPushers();    // get all the devices
    std::vector<PusherGroupRef> groups  = mPusherDiscoveryService->getGroups();     // or get the groups
    PixelPusherRef              pusher;
    
    mFontBig->drawString( "FPS: " + to_string( getAverageFps() ),   pos );   pos.y += lineH * 2;
    
    mFontBig->drawString( "Pusher Discovery Service",                                                                   pos );   pos.y += lineH;
    mFontSmall->drawString( "- - - - - - - - - - - - - - - - - - - - - - - - - -",                                      pos );   pos.y += lineH;
    mFontSmall->drawString( "Globabl brightness:\t\t"   + to_string( PusherDiscoveryService::getGlobalBrightness() ),   pos );   pos.y += lineH;
    mFontSmall->drawString( "Total power:\t\t\t"        + to_string( PusherDiscoveryService::getTotalPower() ),         pos );   pos.y += lineH;
    mFontSmall->drawString( "Total power limit:\t\t"    + to_string( PusherDiscoveryService::getTotalPowerLimit() ),    pos );   pos.y += lineH;
    mFontSmall->drawString( "Color correction:\t\t"     + to_string( PusherDiscoveryService::getColorCorrection() ),    pos );   pos.y += lineH;
    mFontSmall->drawString( "Frame limit:\t\t\t"        + to_string( PusherDiscoveryService::getFrameLimit() ),         pos );   pos.y += lineH;
    mFontSmall->drawString( "Total Devices:\t\t\t"      + to_string( pushers.size() ),                                  pos );   pos.y += lineH;
    mFontSmall->drawString( "Total groups:\t\t\t"       + to_string( groups.size() ),                                   pos );   pos.y += lineH;

    pos.y += lineH * 2;
    
    for( size_t j=0; j < pushers.size(); j++ )
    {
        pusher = pushers[j];
        
        mFontBig->drawString( "Group " + to_string( pusher->getGroupId() ),                             pos );   pos.y += lineH;
        mFontSmall->drawString( "Controller "           + to_string( pusher->getControllerId() ),       pos );   pos.y += lineH;
        mFontSmall->drawString( "- - - - - - - - - - - - - - - - - - -",                                pos );   pos.y += lineH;
        
        // Network
        mFontSmall->drawString( "IP: " + pusher->getIp() + " (" + to_string( pusher->getPort() ) + ")", pos );  pos.y += lineH;
        mFontSmall->drawString( "Mac: " + pusher->getMacAddress(), pos );                                       pos.y += lineH;
        mFontSmall->drawString( "Artnet: universe "     + to_string( pusher->getArtnetUniverse() ) + ", ch " + to_string( pusher->getArtnetChannel() ), pos ); pos.y += lineH;
        
        // Software/hardware
        mFontSmall->drawString( "Software rev: "        + to_string( pusher->getSoftwareRevision() ),   pos );  pos.y += lineH;
        mFontSmall->drawString( "Hardware rev: "        + to_string( pusher->getHardwareRevision() ),   pos );  pos.y += lineH;
        
        // strips
        mFontSmall->drawString( "Num strips: "          + to_string( pusher->getNumStrips() ),          pos );  pos.y += lineH;
        mFontSmall->drawString( "Pixels per strip: "    + to_string( pusher->getPixelsPerStrip() ),     pos );  pos.y += lineH;
        
        mFontSmall->drawString( "Total power: "         + to_string( pusher->getPowerTotal() ),         pos );  pos.y += lineH;
        mFontSmall->drawString( "Update period: "       + to_string( pusher->getUpdatePeriod() ),       pos );  pos.y += lineH;
        mFontSmall->drawString( "Thread sleep for: "    + to_string( pusher->getThreadSleepFor() ),     pos );  pos.y += lineH;
        mFontSmall->drawString( "Packet number: "       + to_string( pusher->getPacketNumber() ),       pos );  pos.y += lineH;
        
        pos.x += 250;
    }
    
    mParams.draw();
}


CINDER_APP_NATIVE( BasicSampleApp, RendererGl )

