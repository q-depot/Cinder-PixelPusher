
class Pixel {

public:

    Pixel() {
      mRed     = 0;
      mGreen   = 0;
      mBlue    = 0;
      mOrange  = 0;
      mWhite   = 0;
    }
    
    Pixel(Pixel &pixel) {
      setColor(pixel);
    }

    Pixel( unsigned char red, unsigned char green, unsigned char blue ) {
        
        Pixel( red, green, blue, 0, 0 );
    }

    Pixel( unsigned char red, unsigned char green, unsigned char blue, unsigned char orange, unsigned char white ) {
      mRed      = red;
      mGreen    = green;
      mBlue     = blue;
      mOrange   = orange;
      mWhite    = white;
    }

    // Processing "color" objects only support the axes of red, green and blue.
    void setColor( int color ) 
    {
      mBlue   = color & 0xff;
      mGreen  = (color >> 8) & 0xff;
      mRed    = (color >> 16) & 0xff;
      mOrange = 0;
      mWhite  = 0;
    }


    void setColorAntilog(int color) {
      mBlue   = sLinearExp[(int)(color & 0xff)];
      mGreen  = sLinearExp[(int)((color >> 8) & 0xff)];
      mRed    = sLinearExp[((int)(color >> 16) & 0xff)];
      mOrange = 0;
      mWhite  = 0;
    }


    Pixel(int color) {
      setColor(color);
    }

    Pixel(int color, bool useAntilog) {
      if (useAntilog)
        setColorAntilog(color);
      else
        setColor(color);
    }

    void setColor(Pixel pixel) {
      mRed = pixel.mRed;
      mBlue = pixel.mBlue;
      mGreen = pixel.mGreen;
      mOrange = pixel.mOrange;
      mWhite = pixel.mWhite;
    }

    void setColor(Pixel pixel, bool useAntiLog) {
      if (useAntiLog) {
        mRed    = sLinearExp[pixel.mRed & 0xff];
        mBlue   = sLinearExp[pixel.mBlue  & 0xff];
        mGreen  = sLinearExp[pixel.mGreen & 0xff];
        mOrange = sLinearExp[pixel.mOrange & 0xff];
        mWhite  = sLinearExp[pixel.mWhite & 0xff];
      } else {
        mRed    = pixel.mRed;
        mBlue   = pixel.mBlue;
        mGreen  = pixel.mGreen;
        mOrange = pixel.mOrange;
        mWhite  = pixel.mWhite;
      }
    }

  // @Override
  // public boolean equals(Object obj) {
  //   if (this == obj)
  //     return true;
    
  //   if (!(obj instanceof Pixel))
  //     return false;
    
  //   Pixel that = (Pixel) obj;
  //   return mRed == that.red
  //       && mGreen == that.green
  //       && mBlue == that.blue
  //       && mOrange == that.orange
  //       && mWhite == that.white;
  // }

  private:
    
    unsigned char mRed;
    unsigned char mGreen;
    unsigned char mBlue;
    unsigned char mOrange;
    unsigned char mWhite;

    const unsigned char sLinearExp[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12,
      13, 13, 13, 14, 14, 14, 14, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27,
      27, 28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57,
      59, 60, 61, 63, 64, 65, 67, 68, 70, 72, 73, 75, 76, 78, 80, 82, 83, 85, 87, 89, 91, 93, 95, 97, 99, 102, 104, 106, 109, 111, 114, 116,
      119, 121, 124, 127, 129, 132, 135, 138, 141, 144, 148, 151, 154, 158,
      161, 165, 168, 172, 176, 180, 184, 188, 192, 196, 201, 205,
      209, 214, 219, 224, 229, 234, 239, 244, 249, 255 };

};
