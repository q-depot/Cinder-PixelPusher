
#pragma once
#include "PixelPusher.h"

class Strip {

  public:

  PixelPusherRef getPusher() {
    return mPusher;
  }

    Strip( PixelPusherRef pusher, int stripNumber, int length, boolean antiLog) 
    {
      for ( int i = 0; i < length; i++ )
        mPixels[i].push_back( Pixel() );

      mPusher       = pusher;
      mStripNumber  = stripNumber;
      mTouched      = false;
      mPowerScale   = 1.0;
      mIsRGBOW      = false;
      mUseAntiLog   = antiLog;
      mMsg          = new byte[mPixels.size() * 3];
  }

  Strip(PixelPusher pusher, int stripNumber, int length) 
  {
    Strip( pusher, stripNumber, length, false );
  }

  // get the RGBOW state of the strip.
  bool getRGBOW() {
    return mIsRGBOW;
  }

  // set the RGBOW state of the strip;  this function is idempotent.
  void setRGBOW( bool state ) {
    if (state == mIsRGBOW)
        return;
    mTouched = true;
    mPusher->markTouched();
    int length = mPixels.size();

    if (mIsRGBOW) {  // if we're already set to RGBOW mode
      mPixels.clear();
      int newSize = length*3;
      
      for (int i = 0; i < newSize; i++)
        mPixels[i].push_back( Pixel() );

      mMsg = new byte[mPixels.size()*3];
      return;
    }
    // otherwise, we were in RGB mode.
    if (state) { // if we are going to RGBOW mode
      mPixels.clear();
      int newSize = length/3; // shorten the pixel array
        
      for (int i = 0; i < newSize; i++)
        mPixels[i].push_back( Pixel() );

      delete mMsg;
      mMsg = new byte[mPixels.size() * 9];  // but lengthen the serialization buffer.
      mIsRGBOW = state;
      
      return;
    }
    // otherwise, do nothing.

  }

  int getLength() {
      return mPixels.size();
  }

  void setPowerScale(double scale)
  {
    mPowerScale = scale;
  }

  std::string getMacAddress() {
    return mPusher->getMacAddress();
  }

  // synchronized 
  bool isTouched() {
    return mTouched;
  }

  // synchronized 
  void markClean() {
    mTouched = false;
    mPusher->markUntouched();
  }

  int getStripNumber() {
    return mStripNumber;
  }

  int getStripIdentifier() {
    // Return a compact reversible identifier
    return -1;
  }

  // synchronized 
  void setPixels( vector<Pixel> pixels) {
    for( size_t k=0; k < pixels.size(); k++ )
    {
      if ( k >= mPixels.size() )
        break;

      mPixels[k] = pixels[k];
    }

    mPouched    = true;
    mPushedAt   = 0;
    mPusher->markTouched();
  }

  // synchronized 
  void setPixelRed(byte intensity, int position) {
    if (position >= this.pixels.length)
      return;
    try {
      if (useAntiLog) {
        this.pixels[position].red = sLinearExp[(int)intensity];
      } else
        this.pixels[position].red = intensity;
    } catch (NullPointerException nope) {
      System.err.println("Tried to write to pixel "+position+" but it wasn't there.");
      nope.printStackTrace();
    }
    this.touched = true;
    pushedAt = 0;
    pusher.markTouched();
  }

  // synchronized 
  void setPixelBlue( byte intensity, int position ) 
  {
    if (position >= mPixels.size() )
      return;

      if (useAntiLog)
        mPixels[position].blue = sLinearExp[(int)intensity];
      else
        mPixels[position].blue = intensity;
    
    mTouched = true;
    mPushedAt = 0;
    mPusher->markTouched();
  }

  // synchronized
   void setPixelGreen(byte intensity, int position) {
    
    if (position >= mPixels.size() )
      return;
    if (useAntiLog) {
        mPixels[position].green = sLinearExp[(int)intensity];
      } else
      mPixels[position].green = intensity;
    
    mTouched = true;
    mPushedAt = 0;
    mPusher.markTouched();
  }

  // synchronized 
  void setPixelOrange(byte intensity, int position) 
  {
    if (position >= mPixels.size() )
      return;
    
    if (useAntiLog) {
        mPixels[position].orange = sLinearExp[(int)intensity];
      } else
      mPixels[position].orange = intensity;
    
    mTouched = true;
    mPushedAt = 0;
    mPusher->markTouched();
  }

  // synchronized 
  void setPixelWhite(byte intensity, int position) {
    
    if (position >= mPixels.size() )
      return;

    try {
      if (useAntiLog) {
        this.pixels[position].white = sLinearExp[(int)intensity];
      } else
      this.pixels[position].white = intensity;
    } catch (NullPointerException nope) {
      System.err.println("Tried to write to pixel "+position+" but it wasn't there.");
      nope.printStackTrace();
    }
    this.touched = true;
    pushedAt = 0;
    pusher.markTouched();
  }

  // synchronized 
  void setPixel(int color, int position) {
    
    if (position >= mPixels.size() )
      return;

      if (useAntiLog)
        mPixels[position].setColorAntilog(color);
      else
        mPixels[position].setColor(color);
    
    mTouched = true;
    mPushedAt = 0;
    mPusher->markTouched();
  }

    // synchronized 
    void setPixel(Pixel pixel, int position) {

    if (position >= mPixels.size() )
      return;

    if (useAntiLog)
      mPixels[position].setColor(pixel, true);
    else
      mPixels[position].setColor(pixel);
    mTouched = true;
    mPushedAt = 0;
    mPusher.markTouched();
  }
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
  void useAntiLog( bool antiLog ) {
    mUseAntiLog = antiLog;
  }

  void setPusher(PixelPusher pixelPusher) {
    mPusher = pixelPusher;
  }
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
  bool isMotion() {
    return mIsMotion;
  }

  void setMotion(bool isMotion) {
    mIsMotion = isMotion;
  }

  bool isNotIdempotent() {
    return mIsNotIdempotent;
  }

  void setNotIdempotent(bool isNotIdempotent) {
    mIsNotIdempotent = isNotIdempotent;
  }

  long getPushedAt() {
    return mPushedAt;
  }

  void setPushedAt(long pushedAt) {
    mPushedAt = pushedAt;
  }

    

  private:

    vector<Pixel>   mPixels;
    // Pixel[]     mPixels;
    // int         mPixelsNum;
    PixelPusher     pusher;
    long int        mPushedAt;
    int             mStripNumber;
    bool            mTouched;
    double          mPowerScale;
    bool            mIsRGBOW;
    byte[]          mMsg;
    bool            mUseAntiLog;
    bool            mIsMotion;
    bool            mIsNotIdempotent;

  // already defined somewhere else? WTF is wrong with source code/developer?
  const byte sLinearExp[] = { (byte) 0,(byte) 0,(byte) 0,(byte) 0,(byte) 0,(byte) 0,(byte) 1,(byte) 1,(byte) 1,(byte) 1,
    (byte) 1,(byte) 2,(byte) 2,(byte) 2,(byte) 2,(byte) 2,(byte) 3,(byte) 3,(byte) 3,(byte) 3,(byte) 4,(byte) 4,
    (byte) 4,(byte) 4,(byte) 5,(byte) 5,(byte) 5,(byte) 5,(byte) 6,(byte) 6,(byte) 6,(byte) 6,(byte) 7,(byte) 7,
    (byte) 7,(byte) 7,(byte) 8,(byte) 8,(byte) 8,(byte) 8,(byte) 9,(byte) 9,(byte) 9,(byte) 10,(byte) 10,(byte) 10,
    (byte) 10,(byte) 11,(byte) 11,(byte) 11,(byte) 12,(byte) 12,(byte) 12,(byte) 13,(byte) 13,(byte) 13,(byte) 14,
    (byte) 14,(byte) 14,(byte) 15,(byte) 15,(byte) 15,(byte) 16,(byte) 16,(byte) 17,(byte) 17,(byte) 17,(byte) 18,
    (byte) 18,(byte) 18,(byte) 19,(byte) 19,(byte) 20,(byte) 20,(byte) 20,(byte) 21,(byte) 21,(byte) 22,(byte) 22,
    (byte) 22,(byte) 23,(byte) 23,(byte) 24,(byte) 24,(byte) 25,(byte) 25,(byte) 26,(byte) 26,(byte) 27,(byte) 27,
    (byte) 28,(byte) 28,(byte) 29,(byte) 29,(byte) 30,(byte) 30,(byte) 31,(byte) 31,(byte) 32,(byte) 32,(byte) 33,
    (byte) 33,(byte) 34,(byte) 34,(byte) 35,(byte) 36,(byte) 36,(byte) 37,(byte) 37,(byte) 38,(byte) 38,(byte) 39,
    (byte) 40,(byte) 40,(byte) 41,(byte) 42,(byte) 42,(byte) 43,(byte) 44,(byte) 44,(byte) 45,(byte) 46,(byte) 46,
    (byte) 47,(byte) 48,(byte) 48,(byte) 49,(byte) 50,(byte) 51,(byte) 51,(byte) 52,(byte) 53,(byte) 54,(byte) 54,
    (byte) 55,(byte) 56,(byte) 57,(byte) 57,(byte) 58,(byte) 59,(byte) 60,(byte) 61,(byte) 62,(byte) 62,(byte) 63,
    (byte) 64,(byte) 65,(byte) 66,(byte) 67,(byte) 68,(byte) 69,(byte) 70,(byte) 71,(byte) 72,(byte) 73,(byte) 74,
    (byte) 75,(byte) 76,(byte) 77,(byte) 78,(byte) 79,(byte) 80,(byte) 81,(byte) 82,(byte) 83,(byte) 84,(byte) 85,
    (byte) 86,(byte) 87,(byte) 89,(byte) 90,(byte) 91,(byte) 92,(byte) 93,(byte) 94,(byte) 96,(byte) 97,(byte) 98,
    (byte) 99,(byte) 101,(byte) 102,(byte) 103,(byte) 105,(byte) 106,(byte) 107,(byte) 109,(byte) 110,(byte) 111,
    (byte) 113,(byte) 114,(byte) 116,(byte) 117,(byte) 119,(byte) 120,(byte) 121,(byte) 123,(byte) 125,(byte) 126,
    (byte) 128,(byte) 129,(byte) 131,(byte) 132,(byte) 134,(byte) 136,(byte) 137,(byte) 139,(byte) 141,(byte) 142,
    (byte) 144,(byte) 146,(byte) 148,(byte) 150,(byte) 151,(byte) 153,(byte) 155,(byte) 157,(byte) 159,(byte) 161,
    (byte) 163,(byte) 165,(byte) 167,(byte) 169,(byte) 171,(byte) 173,(byte) 175,(byte) 177,(byte) 179,(byte) 181,
    (byte) 183,(byte) 186,(byte) 188,(byte) 190,(byte) 192,(byte) 195,(byte) 197,(byte) 199,(byte) 202,(byte) 204,
    (byte) 206,(byte) 209,(byte) 211,(byte) 214,(byte) 216,(byte) 219,(byte) 221,(byte) 224,(byte) 227,(byte) 229,
    (byte) 232,(byte) 235,(byte) 237,(byte) 240,(byte) 243,(byte) 246,(byte) 249,(byte) 252 };
    
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
