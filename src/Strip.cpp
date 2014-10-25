

#include "Strip.h"
#include "PixelPusher.h"


Strip::Strip( PixelPusherRef pusher, int stripNumber, int length, bool antiLog )
{
    for ( int i = 0; i < length; i++ )
        mPixels.push_back( Pixel() );

    mPusher       = pusher;
    mStripNumber  = stripNumber;
    mTouched      = false;
    mPowerScale   = 1.0;
    mIsRGBOW      = false;
    mUseAntiLog   = antiLog;
//      mMsg          = new byte[mPixels.size() * 3];
}


void Strip::setRGBOW( bool state )
{
    if (state == mIsRGBOW)
        return;
    
    mTouched = true;
    mPusher->markTouched();
    int length = mPixels.size();

    if ( mIsRGBOW )   // if we're already set to RGBOW mode
    {
        mPixels.clear();
        int newSize = length*3;
  
        for (int i = 0; i < newSize; i++)
            mPixels.push_back( Pixel() );

//            mMsg = new byte[mPixels.size()*3];
    }
    
    // otherwise, we were in RGB mode.
    else if ( state )  // if we are going to RGBOW mode
    {
        mPixels.clear();
        int newSize = length/3; // shorten the pixel array
    
        for (int i = 0; i < newSize; i++)
            mPixels.push_back( Pixel() );

//            delete mMsg;
//            mMsg = new byte[mPixels.size() * 9];  // but lengthen the serialization buffer.
        mIsRGBOW = state;
    }
}


std::string Strip::getMacAddress() { return mPusher->getMacAddress(); }


void Strip::markClean()
{
    mTouched = false;
    mPusher->markUntouched();
}


void Strip::setPixels( std::vector<Pixel> pixels )
{
    for( size_t k=0; k < pixels.size(); k++ )
    {
        if ( k >= mPixels.size() )
            break;

        mPixels[k].setColor( pixels[k] );
    }

    mTouched    = true;
    mPushedAt   = 0;
    mPusher->markTouched();
}


void Strip::setPixelRed( uint8_t intensity, int position )
{
    if (position >= mPixels.size() )
        return;
    
    if ( mUseAntiLog )
        mPixels[position].mRed = sLinearExp[intensity];
    else
        mPixels[position].mRed = intensity;

    mTouched    = true;
    mPushedAt   = 0;
    mPusher->markTouched();
}


void Strip::setPixelBlue( uint8_t intensity, int position )
{
    if (position >= mPixels.size() )
        return;

    if ( mUseAntiLog )
        mPixels[position].mBlue = sLinearExp[intensity];
    else
        mPixels[position].mBlue = intensity;

    mTouched = true;
    mPushedAt = 0;
    mPusher->markTouched();
}


void Strip::setPixelGreen( uint8_t intensity, int position )
{

    if ( position >= mPixels.size() )
        return;
    
    if ( mUseAntiLog )
        mPixels[position].mGreen = sLinearExp[intensity];
    else
        mPixels[position].mGreen = intensity;

    mTouched    = true;
    mPushedAt   = 0;
    mPusher->markTouched();
}


void Strip::setPixelOrange( uint8_t intensity, int position )
{
    if ( position >= mPixels.size() )
        return;

    if ( mUseAntiLog )
        mPixels[position].mOrange = sLinearExp[intensity];
    else
        mPixels[position].mOrange = intensity;

    mTouched    = true;
    mPushedAt   = 0;
    mPusher->markTouched();
}


void Strip::setPixelWhite( uint8_t intensity, int position )
{
    if ( position >= mPixels.size() )
        return;

    if ( mUseAntiLog )
        mPixels[position].mWhite = sLinearExp[intensity];
    else
        mPixels[position].mWhite = intensity;
    
    mTouched    = true;
    mPushedAt   = 0;
    mPusher->markTouched();
}


void Strip::setPixel( int color, int position )
{
    if (position >= mPixels.size() )
        return;

    if ( mUseAntiLog )
        mPixels[position].setColorAntilog(color);
    else
        mPixels[position].setColor(color);

    mTouched    = true;
    mPushedAt   = 0;
    mPusher->markTouched();
}


void Strip::setPixel( Pixel pixel, int position )
{
    if ( position >= mPixels.size() )
        return;

    if ( mUseAntiLog )
        mPixels[position].setColor(pixel, true);
    else
        mPixels[position].setColor(pixel);

    mTouched    = true;
    mPushedAt   = 0;
    mPusher->markTouched();
}
