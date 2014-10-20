
class Pixel {

public:

    Pixel() {
      red     = 0;
      green   = 0;
      blue    = 0;
      orange  = 0;
      white   = 0;
    }
    
    Pixel(Pixel pixel) {
      setColor(pixel);
    }

    Pixel(byte red, byte green, byte blue) {
      mRed = red;
      mGreen = green;
      mBlue = blue;
      mOrange = 0;
      mWhite = 0;
    }

    Pixel(byte red, byte green, byte blue, byte orange, byte white) {
      mRed = red;
      mGreen = green;
      mBlue = blue;
      mOrange = orange;
      mWhite = white;
    }

    // Processing "color" objects only support the axes of red, green and blue.
    void setColor( int color ) 
    {
      mBlue   = (byte)(color & 0xff);
      mGreen  = (byte) ((color >> 8) & 0xff);
      mRed    = (byte) ((color >> 16) & 0xff);
      mOrange = (byte) 0;
      mWhite  = (byte) 0;
    }


    void setColorAntilog(int color) {
      mBlue   = sLinearExp[(int)(color & 0xff)];
      mGreen  = sLinearExp[(int)((color >> 8) & 0xff)];
      mRed    = sLinearExp[((int)(color >> 16) & 0xff)];
      mOrange = (byte) 0;
      mWhite  = (byte) 0;
    }


    Pixel(int color) {
      setColor(color);
    }

    Pixel(int color, boolean useAntilog) {
      if (useAntilog)
        setColorAntilog(color);
      else
        setColor(color);
    }

    void setColor(Pixel pixel) {
      mRed = pixel.red;
      mBlue = pixel.blue;
      mGreen = pixel.green;
      mOrange = pixel.orange;
      mWhite = pixel.white;
    }

    void setColor(Pixel pixel, boolean useAntiLog) {
      if (useAntiLog) {
        mRed    = sLinearExp[pixel.red & 0xff];
        mBlue   = sLinearExp[pixel.blue  & 0xff];
        mGreen  = sLinearExp[pixel.green & 0xff];
        mOrange = sLinearExp[pixel.orange & 0xff];
        mWhite  = sLinearExp[pixel.white & 0xff];
      } else {
        mRed    = pixel.red;
        mBlue   = pixel.blue;
        mGreen  = pixel.green;
        mOrange = pixel.orange;
        mWhite  = pixel.white;
      }
    }

  int hashCode() {
    return (red ^ green ^ blue ^ orange ^ white);
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
    
    byte mRed;
    byte mRreen;
    byte mBlue;
    byte mOrange;
    byte mWhite;

    const byte sLinearExp[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12,
      13, 13, 13, 14, 14, 14, 14, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27,
      27, 28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57,
      59, 60, 61, 63, 64, 65, 67, 68, 70, 72, 73, 75, 76, 78, 80, 82, 83, 85, 87, 89, 91, 93, 95, 97, 99, 102, 104, 106, 109, 111, 114, 116,
      119, 121, 124, 127, (byte)129, (byte)132, (byte)135, (byte)138, (byte)141, (byte)144, (byte)148, (byte)151, (byte)154, (byte)158,
      (byte)161, (byte)165, (byte)168, (byte)172, (byte)176, (byte)180, (byte)184, (byte)188, (byte)192, (byte)196, (byte)201, (byte)205,
      (byte)209, (byte)214, (byte)219, (byte)224, (byte)229, (byte)234, (byte)239, (byte)244, (byte)249, (byte)255 };

};
