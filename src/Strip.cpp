

#include "Strip.h"
#include "PixelPusher.h"
#include "PusherDiscoveryService.h"


Strip::Strip( uint8_t stripNumber, int length )
{
    for ( int i = 0; i < length; i++ )
        mPixels.push_back( Pixel::create() );

    mStripNumber  = stripNumber;
    mTouched      = false;
    mIsRGBOW      = false;
    mPixelsBuffer = ci::Buffer( mPixels.size() * 3 );
}


void Strip::setRGBOW( bool state )
{
    if ( state == mIsRGBOW )
        return;

    int length = mPixels.size();

    if ( mIsRGBOW )   // if we're already set to RGBOW mode
    {
        mPixels.clear();
        int newSize = length*3;
  
        for (int i = 0; i < newSize; i++)
            mPixels.push_back( Pixel::create() );
        
        mPixelsBuffer.resize( mPixels.size() * 3 );
    }
    
    // otherwise, we were in RGB mode.
    else if ( state )  // if we are going to RGBOW mode
    {
        mPixels.clear();
        int newSize = length/3; // shorten the pixel array
    
        for (int i = 0; i < newSize; i++)
            mPixels.push_back( Pixel::create() );
        
        mPixelsBuffer.resize( mPixels.size() * 9 );
        mIsRGBOW = state;
    }
    
    mTouched = true;
}


void Strip::setPixel( int position, uint8_t r, uint8_t g, uint8_t b, uint8_t o, uint8_t w )
{
    if (position >= mPixels.size() )
        return;
    
    mPixels[position]->setColor( r, g, b, o, w, PusherDiscoveryService::getAntiLog() );
    
    mTouched = true;
}


void Strip::setPixelsBlack()
{
    for( size_t k=0; k < mPixels.size(); k++ )
        mPixels[k]->setColor( 0, 0, 0, 0, 0 );
    
    mTouched = true;
}


void Strip::updatePixelsBuffer()
{
    int         byteIdx;
    PixelRef    px;
    uint8_t     *data                   = (uint8_t*)mPixelsBuffer.getData();
    double      brightness              = PusherDiscoveryService::getPowerScale();
    
    if ( PusherDiscoveryService::isGlobalBrightness() )
        brightness *= PusherDiscoveryService::getGlobalBrightness();
    
    if ( mIsRGBOW )
    {
        for( size_t k=0; k < mPixels.size(); k++ )
        {
            px      = mPixels[k];
            byteIdx = k * 9;
            
            data[byteIdx+0] = (uint8_t)( (double)(px->mRed & 0xff)      * brightness );
            data[byteIdx+1] = (uint8_t)( (double)(px->mGreen & 0xff)    * brightness );
            data[byteIdx+2] = (uint8_t)( (double)(px->mBlue & 0xff)     * brightness );
            
            data[byteIdx+3] = (uint8_t)( (double)(px->mOrange & 0xff)   * brightness );
            data[byteIdx+4] = (uint8_t)( (double)(px->mOrange & 0xff)   * brightness );
            data[byteIdx+5] = (uint8_t)( (double)(px->mOrange & 0xff)   * brightness );
            
            data[byteIdx+6] = (uint8_t)( (double)(px->mWhite & 0xff)    * brightness );
            data[byteIdx+7] = (uint8_t)( (double)(px->mWhite & 0xff)    * brightness );
            data[byteIdx+8] = (uint8_t)( (double)(px->mWhite & 0xff)    * brightness );
        }
    }
    else
    {
        for( size_t k=0; k < mPixels.size(); k++ )
        {
            px      = mPixels[k];
            byteIdx = k * 3;
            
            data[byteIdx+0] = (uint8_t)( (double)(px->mRed & 0xff)      * brightness );
            data[byteIdx+1] = (uint8_t)( (double)(px->mGreen & 0xff)    * brightness );
            data[byteIdx+2] = (uint8_t)( (double)(px->mBlue & 0xff)     * brightness );
        }
    }
}

