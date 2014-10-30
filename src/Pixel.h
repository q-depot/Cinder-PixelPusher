/*
 *  Pixel.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2014 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef PIXEL_PUSHER_PIXEL
#define PIXEL_PUSHER_PIXEL

#include "PusherDiscoveryService.h"

class Pixel;
typedef std::shared_ptr<Pixel> PixelRef;


class Pixel {
    
    friend class Strip;
    
public:
    
    static PixelRef create()
    {
        return PixelRef( new Pixel() );
    }
    
    static PixelRef create( uint8_t red, uint8_t green, uint8_t blue )
    {
        return PixelRef( new Pixel( red, green, blue ) );
    }
    
    static PixelRef create( uint8_t red, uint8_t green, uint8_t blue, uint8_t orange, uint8_t white )
    {
        return PixelRef( new Pixel( red, green, blue, orange, white ) );
    }
    
    ~Pixel() {}
    
    void setColor( Pixel pixel, bool colorCorrection = false )
    {
        setColor( pixel.mRed, pixel.mGreen, pixel.mBlue, pixel.mOrange, pixel.mWhite, colorCorrection );
    }
    
    void setColor( uint8_t r, uint8_t g, uint8_t b, uint8_t o = 0, uint8_t w = 0, bool colorCorrection = false )
    {
        if ( colorCorrection )
        {
            mRed     = sLinearExp[r];
            mGreen   = sLinearExp[g];
            mBlue    = sLinearExp[b];
            mOrange  = sLinearExp[r];
            mWhite   = sLinearExp[g];
        }
        else
        {
            mRed     = r;
            mGreen   = g;
            mBlue    = b;
            mOrange  = sLinearExp[o];
            mWhite   = sLinearExp[w];
        }
    }
    
    
private:
    
    Pixel() {
        mRed     = 0;
        mGreen   = 0;
        mBlue    = 0;
        mOrange  = 0;
        mWhite   = 0;
    }
    
    Pixel( uint8_t red, uint8_t green, uint8_t blue, uint8_t orange = 0, uint8_t white = 0 )
    {
        mRed      = red;
        mGreen    = green;
        mBlue     = blue;
        mOrange   = orange;
        mWhite    = white;
    }


private:
    
    uint8_t mRed;
    uint8_t mGreen;
    uint8_t mBlue;
    uint8_t mOrange;
    uint8_t mWhite;

    const uint8_t sLinearExp[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12,
      13, 13, 13, 14, 14, 14, 14, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27,
      27, 28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57,
      59, 60, 61, 63, 64, 65, 67, 68, 70, 72, 73, 75, 76, 78, 80, 82, 83, 85, 87, 89, 91, 93, 95, 97, 99, 102, 104, 106, 109, 111, 114, 116,
      119, 121, 124, 127, 129, 132, 135, 138, 141, 144, 148, 151, 154, 158,
      161, 165, 168, 172, 176, 180, 184, 188, 192, 196, 201, 205,
      209, 214, 219, 224, 229, 234, 239, 244, 249, 255 };

};

#endif
