#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "Strip.h"
#include "DeviceRegistry.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class BasicSampleApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void update();
	void draw();
    
    DeviceRegistryRef   mDeviceRegistry;
};

void BasicSampleApp::setup()
{
    mDeviceRegistry = DeviceRegistry::create( io_service() );
}

void BasicSampleApp::mouseDown( MouseEvent event )
{
}

void BasicSampleApp::update()
{
    std::vector<PixelPusherRef> pushers = mDeviceRegistry->getPushers();
    std::vector<StripRef>       strips;
    std::vector<PixelRef>       pixels;
    uint8_t                     val_t;
    
    Color col;
    
    if ( !pushers.empty() )
    {
        strips = pushers.front()->getStrips();
        for( size_t k=0; k < strips.size(); k++ )
        {
            pixels = strips[k]->getPixels();
            for( size_t i=0; i < pixels.size(); i++ )
            {
                //                val_t = 0.5 * ( 1.0 + sin( i + getElapsedSeconds() ) ) * 255;
                val_t = 0.5 * ( 1.0 + sin( i + getElapsedSeconds() ) ) * 255;
                
                col.r = 0.5f * ( 1.0f + sin( (float)i / 15 + 2 * getElapsedSeconds() ) );
                col.g = 0.5f * ( 1.0f + cos( (float)i / 15 + 2 * getElapsedSeconds() ) );
                col.b = 1.0 - 0.5f * ( 1.0f + cos( (float)i / 5 + 2 * getElapsedSeconds() ) );
                //                    val = 0.5f * ( 1.0f + sin( i + 2 * getElapsedSeconds() ) );
                //                    col = Color( CM_HSV, Vec3f( 1.0- val, 1.0, val ) );
                if ( k == 0 )
                    col = Color( 0.2, 0.0, 0.0 );
                
                strips[k]->setPixelRGB( i, (uint8_t)( col.r * 255 ), (uint8_t)( col.g * 255 ), (uint8_t)( col.b * 255 ) );
                
            }
        }
    }
}

void BasicSampleApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( BasicSampleApp, RendererGl )
