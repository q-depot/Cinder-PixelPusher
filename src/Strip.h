

#pragma once
#include "Pixel.h"

class PixelPusher;
typedef std::shared_ptr<PixelPusher> PixelPusherRef;

class Strip;
typedef std::shared_ptr<Strip> StripRef;


class Strip {

  public:
    
    static StripRef create( PixelPusherRef pusher, int stripNumber, int length, bool antiLog )
    {
        return StripRef( new Strip( pusher, stripNumber, length, antiLog ) );
    }
    
    static StripRef create( PixelPusherRef pusher, int stripNumber, int length)
    {
        return StripRef( new Strip( pusher, stripNumber, length, false ) );
    }
    
    ~Strip() {}
    
    PixelPusherRef getPusher() {
        return mPusher;
    }

    // get the RGBOW state of the strip.
    bool getRGBOW() { return mIsRGBOW; }

    // set the RGBOW state of the strip;  this function is idempotent.
    void setRGBOW( bool state );

    int getLength() { return mPixels.size(); }

    void setPowerScale(double scale) { mPowerScale = scale; }

    std::string getMacAddress();

    // synchronized
    bool isTouched() { return mTouched; }

    // synchronized
    void markClean();

    int getStripNumber() { return mStripNumber; }

    int getStripIdentifier()
    {
        // Return a compact reversible identifier
        return -1;
    }

    void setPixels( std::vector<Pixel> pixels);
    
    ///////////////////
    // TODO: replace the setPixel RGB with something that makes sense!!!!!!!!!!!!!! <<<<<<<<<<<<<<<
    
    void setPixelRed( uint8_t intensity, int position );
    void setPixelBlue( uint8_t intensity, int position );
    void setPixelGreen( uint8_t intensity, int position );
    void setPixelOrange( uint8_t intensity, int position );
    void setPixelWhite( uint8_t intensity, int position );
    void setPixel( int color, int position );
    void setPixel( Pixel pixel, int position );
    
    /*
  public byte[] serialize() {
    int i = 0;
    if (isRGBOW) {
      for (Pixel pixel : pixels) {
        if (pixel == null)
          pixel = new Pixel();
        msg[i++] = (byte) (((double)(pixel.red & 0xff))   * powerScale);    // C
        msg[i++] = (byte) (((double)(pixel.green & 0xff)) * powerScale);
        msg[i++] = (byte) (((double)(pixel.blue & 0xff))  * powerScale);

        msg[i++] = (byte) (((double)(pixel.orange & 0xff)) * powerScale);   // O
        msg[i++] = (byte) (((double)(pixel.orange & 0xff)) * powerScale);
        msg[i++] = (byte) (((double)(pixel.orange & 0xff)) * powerScale);

        msg[i++] = (byte) (((double)(pixel.white & 0xff)) * powerScale);    // W
        msg[i++] = (byte) (((double)(pixel.white & 0xff)) * powerScale);
        msg[i++] = (byte) (((double)(pixel.white & 0xff)) * powerScale);
      }
    } else {
      for (Pixel pixel : pixels) {
        if (pixel == null)
          pixel = new Pixel();
        msg[i++] = (byte) ((double)((pixel.red & 0xff)) * powerScale);
        msg[i++] = (byte) ((double)((pixel.green & 0xff)) * powerScale);
        msg[i++] = (byte) ((double)((pixel.blue & 0xff)) * powerScale);
      }
    }
    return msg;
  }
*/
    
    void useAntiLog( bool antiLog ) { mUseAntiLog = antiLog; }

    void setPusher( PixelPusherRef pixelPusher ) { mPusher = pixelPusher; }
    
/*
  byte[] serialize(double overallBrightnessScale) {
    int i = 0;
    if (isRGBOW) {
      for (Pixel pixel : pixels) {
        if (pixel == null)
          pixel = new Pixel();
        msg[i++] = (byte) ((double)(pixel.red & 0xff)  * powerScale * overallBrightnessScale);    // C
        msg[i++] = (byte) ((double)(pixel.green & 0xff) * powerScale * overallBrightnessScale);
        msg[i++] = (byte) ((double)(pixel.blue & 0xff)  * powerScale * overallBrightnessScale);

        msg[i++] = (byte) ((double)(pixel.orange & 0xff) * powerScale * overallBrightnessScale);   // O
        msg[i++] = (byte) ((double)(pixel.orange & 0xff) * powerScale * overallBrightnessScale);
        msg[i++] = (byte) ((double)(pixel.orange & 0xff) * powerScale * overallBrightnessScale);

        msg[i++] = (byte) ((double)(pixel.white & 0xff) * powerScale * overallBrightnessScale);    // W
        msg[i++] = (byte) ((double)(pixel.white & 0xff) * powerScale * overallBrightnessScale);
        msg[i++] = (byte) ((double)(pixel.white & 0xff) * powerScale * overallBrightnessScale);
      }
    } else {
      for (Pixel pixel : pixels) {
        if (pixel == null)
          pixel = new Pixel();
        msg[i++] = (byte) ((double)(pixel.red & 0xff) * powerScale * overallBrightnessScale);
        msg[i++] = (byte) ((double)(pixel.green & 0xff) * powerScale * overallBrightnessScale);
        msg[i++] = (byte) ((double)(pixel.blue & 0xff) * powerScale * overallBrightnessScale);
      }
    }
    return msg;
  }
*/
    bool isMotion() { return mIsMotion; }

    void setMotion(bool isMotion) { mIsMotion = isMotion; }

    bool isNotIdempotent() { return mIsNotIdempotent; }

    void setNotIdempotent( bool isNotIdempotent ) { mIsNotIdempotent = isNotIdempotent; }

    long getPushedAt() { return mPushedAt; }

    void setPushedAt(long pushedAt) { mPushedAt = pushedAt; }


private:
    
    Strip( PixelPusherRef pusher, int stripNumber, int length, bool antiLog );
    
  private:

        std::vector<Pixel>  mPixels;
        PixelPusherRef      mPusher;
        long int            mPushedAt;
        int                 mStripNumber;
        bool                mTouched;
        double          	mPowerScale;
        bool            	mIsRGBOW;
//        byte[]          	mMsg;
        bool                mUseAntiLog;
        bool            	mIsMotion;
        bool            	mIsNotIdempotent;

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
    
  /*  
    
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12,
    13, 13, 13, 14, 14, 14, 14, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27,
    27, 28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57,
    59, 60, 61, 63, 64, 65, 67, 68, 70, 72, 73, 75, 76, 78, 80, 82, 83, 85, 87, 89, 91, 93, 95, 97, 99, 102, 104, 106, 109, 111, 114, 116,
    119, 121, 124, 127, (byte)129, (byte)132, (byte)135, (byte)138, (byte)141, (byte)144, (byte)148, (byte)151, (byte)154, (byte)158,
    (byte)161, (byte)165, (byte)168, (byte)172, (byte)176, (byte)180, (byte)184, (byte)188, (byte)192, (byte)196, (byte)201, (byte)205,
    (byte)209, (byte)214, (byte)219, (byte)224, (byte)229, (byte)234, (byte)239, (byte)244, (byte)249, (byte)255 };
*/


};
