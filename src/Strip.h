/*
 *  Strip.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2014 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef PP_STRIP
#define PP_STRIP

#pragma once

#include "Pixel.h"

class Strip;
typedef std::shared_ptr<Strip> StripRef;


class Strip {

  public:
    
    struct PixelMap {
        ci::Vec2i   from;
        ci::Vec2i   to;
    };
    
    enum PixelMapOrientation {
        MAP_LEFT_RIGHT,
        MAP_RIGHT_LEFT,
        MAP_TOP_DOWN,
        MAP_BOTTOM_UP
    };
    
    
    static StripRef create( uint8_t stripNumber, int length )
    {
        return StripRef( new Strip( stripNumber, length ) );
    }
    
    ~Strip() {}

    bool getRGBOW() { return mIsRGBOW; }
    void setRGBOW( bool state );

    int getLength() { return mPixels.size(); }

    bool isTouched() { return mIsTouched; }

    uint8_t getStripNumber() { return mStripNumber; }

    void setPixel( int position, uint8_t r, uint8_t g, uint8_t b, uint8_t o = 0, uint8_t w = 0 );
    void setPixelsBlack();
    
    bool isMotion() { return mIsMotion; }

    void setMotion(bool isMotion) { mIsMotion = isMotion; }

    bool isNotIdempotent() { return mIsNotIdempotent; }

    void setNotIdempotent( bool isNotIdempotent ) { mIsNotIdempotent = isNotIdempotent; }
    
    std::vector<PixelRef>   getPixels() { return mPixels; }
    size_t                  getNumPixels() { return mPixels.size(); }
    
    // use 3 methods instead returning the Buffer to avoid mem realloc
    void    updatePixelsBuffer();
    uint8_t *getPixelsData() { return (uint8_t*)mPixelsBuffer.getData(); }
    size_t  getPixelsDataSize() { return mPixelsBuffer.getDataSize(); }
    
    void markTouched( bool isTouch = true ) { mIsTouched = isTouch; }
    
    void setPixelMap( ci::Vec2i offset, PixelMapOrientation orientation );
    
    PixelMap getPixelMap() { return mPixelMap; }
    
private:
    
    Strip( uint8_t stripNumber, int length );
    
  private:

    std::vector<PixelRef>   mPixels;
    uint8_t                 mStripNumber;
    bool                    mIsTouched;
    bool                    mIsRGBOW;
    bool                    mIsMotion;
    bool                    mIsNotIdempotent;
    ci::Buffer              mPixelsBuffer;
    PixelMap                mPixelMap;
    
    
  // TODO: check this shit! already defined somewhere else? WTF is wrong with source code/developer?
    const uint8_t sLinearExp[256] = {  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4,
        4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10,
        10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 17, 17, 17, 18,
        18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27,
        28, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 36, 36, 37, 37, 38, 38, 39,
        40, 40, 41, 42, 42, 43, 44, 44, 45, 46, 46, 47, 48, 48, 49, 50, 51, 51, 52, 53, 54, 54,
        55, 56, 57, 57, 58, 59, 60, 61, 62, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
        75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 89, 90, 91, 92, 93, 94, 96, 97, 98,
        99, 101, 102, 103, 105, 106, 107, 109, 110, 111, 113, 114, 116, 117, 119, 120, 121, 123, 125, 126,
        128, 129, 131, 132, 134, 136, 137, 139, 141, 142, 144, 146, 148, 150, 151, 153, 155, 157, 159, 161,
        163, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 186, 188, 190, 192, 195, 197, 199, 202, 204,
        206, 209, 211, 214, 216, 219, 221, 224, 227, 229, 232, 235, 237, 240, 243, 246, 249, 252 };

};

#endif