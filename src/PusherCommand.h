 /*  const uint8_t pp_command_magic[16] = 
  *   { 0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5, 0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50  };
  *
  * #define COMMAND_RESET                0x01
  * #define COMMAND_GLOBALBRIGHTNESS_SET 0x02
  * #define COMMAND_WIFI_CONFIGURE       0x03
  * #define COMMAND_LED_CONFIGURE        0x04
  */


class PusherCommand {

public:

  PusherCommand( unsigned char command) {
    mCommand = command;
  }
  
  PusherCommand(byte command, short parameter) {
    mCommand    = command;
    mParameter  = parameter;
  }
  
  PusherCommand(byte command, String ssid, String key, String security) {
    mCommand  = command;
    mSsid     = ssid;
    mKey      = key;
    if (security.toLowerCase().compareTo("none") == 0) 
      mSecurity = 0;
    if (security.toLowerCase().compareTo("wep") == 0) 
      mSecurity = 1;
    if (security.toLowerCase().compareTo("wpa") == 0) 
      mSecurity = 2;
    if (security.toLowerCase().compareTo("wpa2") == 0) 
      mSecurity = 3;
  }
  
  PusherCommand(byte command, int numStrips, int stripLength, byte[] stripType, byte[] colourOrder) 
  {
      PusherCommand( command, numStrips, stripLength, stripType, colourOrder, 0, 0, 0, 0 );
  }
  
  PusherCommand(byte command, int numStrips, int stripLength, byte[] stripType, byte[] colourOrder, short group, short controller) 
  {
    PusherCommand( command, numStrips, stripLength, stripType, colourOrder, group, controller, 0, 0 );
  }
  
  PusherCommand(byte command, int numStrips, int stripLength, byte[] stripType, byte[] colourOrder, short group, short controller, 
                       short artnet_universe, short artnet_channel) 
  {
    mCommand        = command;
    mNumStrips      = numStrips;
    mStripLength    = stripLength;
    mStripType      = Arrays.copyOf(stripType, 8);
    mColourOrder    = Arrays.copyOf(colourOrder, 8);
    mGroup          = group;
    mController     = controller;
    mArtnetChannel  = artnet_channel;
    mArtnetUniverse = artnet_universe;
  }
  
  byte [] generateBytes() 
  {
    byte[] returnVal= null;

    if (command == RESET) 
    {
      returnVal = Arrays.copyOf(pp_command_magic, pp_command_magic.length+1);
      returnVal[pp_command_magic.length] = RESET;
    } 
    else if (command == GLOBALBRIGHTNESS_SET) {
      returnVal = Arrays.copyOf(pp_command_magic, pp_command_magic.length+3);
      returnVal[pp_command_magic.length] = GLOBALBRIGHTNESS_SET;
      returnVal[pp_command_magic.length+1] = (byte) (parameter & 0xff);
      returnVal[pp_command_magic.length+1] = (byte) ((parameter>>8) & 0xff);
    } 
    else if (command == WIFI_CONFIGURE) {
      byte[] ssidBytes = ssid.getBytes();
      byte[] keyBytes = key.getBytes();
      int bufLength = 0;
      bufLength += (pp_command_magic.length) + 1; /* length of command */
      bufLength += 1; // length of key type
      bufLength += ssidBytes.length + 1; // ssid plus null terminator
      bufLength += keyBytes.length + 1;  // key plus null terminator
      
      returnVal = Arrays.copyOf(pp_command_magic, bufLength);
      
      returnVal[pp_command_magic.length] = command;
      
      for (int i=0; i<ssidBytes.length; i++ )
        returnVal[pp_command_magic.length+ 1 + i] 
            = ssidBytes[i];
      
      for (int i=0; i<keyBytes.length; i++ )
        returnVal[pp_command_magic.length+ 1 + ssidBytes.length + 1 + i] 
            = keyBytes[i];
      
      returnVal[pp_command_magic.length+ 1 + keyBytes.length + 1 + ssidBytes.length + 1] = security;
    } else if (command == LED_CONFIGURE) {
      returnVal = Arrays.copyOf(pp_command_magic, pp_command_magic.length+33); // two ints, eight bytes, eight bytes, plus command, plus group and controller
                                                                               // plus artnet universe and channel
      returnVal[pp_command_magic.length] = LED_CONFIGURE;

      returnVal[pp_command_magic.length+1+0] = (byte) (num_strips & 0xFF);   
      returnVal[pp_command_magic.length+1+1] = (byte) ((num_strips >> 8) & 0xFF);   
      returnVal[pp_command_magic.length+1+2] = (byte) ((num_strips >> 16) & 0xFF);   
      returnVal[pp_command_magic.length+1+3] = (byte) ((num_strips >> 24) & 0xFF);
      
      returnVal[pp_command_magic.length+5+0] = (byte) (strip_length & 0xFF);   
      returnVal[pp_command_magic.length+5+1] = (byte) ((strip_length >> 8) & 0xFF);   
      returnVal[pp_command_magic.length+5+2] = (byte) ((strip_length >> 16) & 0xFF);   
      returnVal[pp_command_magic.length+5+3] = (byte) ((strip_length >> 24) & 0xFF); 
      
      for (int i = pp_command_magic.length+9; i< pp_command_magic.length+17; i++)
        returnVal[i] = strip_type[i-(pp_command_magic.length+9)];
      for (int i = pp_command_magic.length+17; i< pp_command_magic.length+25; i++)
        returnVal[i] = colour_order[i-(pp_command_magic.length+17)];
      
      returnVal[pp_command_magic.length+25+0] = (byte) (group & 0xFF);   
      returnVal[pp_command_magic.length+25+1] = (byte) ((group >> 8) & 0xFF);
      
      returnVal[pp_command_magic.length+27+0] = (byte) (controller & 0xFF);   
      returnVal[pp_command_magic.length+27+1] = (byte) ((controller >> 8) & 0xFF);
      
      returnVal[pp_command_magic.length+29+0] = (byte) (artnet_universe & 0xFF);   
      returnVal[pp_command_magic.length+29+1] = (byte) ((artnet_universe >> 8) & 0xFF);
      
      returnVal[pp_command_magic.length+31+0] = (byte) (artnet_channel & 0xFF);   
      returnVal[pp_command_magic.length+31+1] = (byte) ((artnet_channel >> 8) & 0xFF);
    
      
    } // end if(command)
    return returnVal;
  }

  public:
    
  
    const unsigned char RESET = 0x01;
    const unsigned char GLOBALBRIGHTNESS_SET = 0x02;
    const unsigned char WIFI_CONFIGURE = 0x03;
    const unsigned char LED_CONFIGURE = 0x04;
    const unsigned char STRIP_LPD8806 = 0;
    const unsigned char STRIP_WS2801 = 1;
    const unsigned char STRIP_WS2811 = 2;
    const unsigned char STRIP_APA102 = 3;
    
    const unsigned char ORDER_RGB = 0;
    const unsigned char ORDER_RBG = 1;
    const unsigned char ORDER_GBR = 2;
    const unsigned char ORDER_GRB = 3;
    const unsigned char ORDER_BGR = 4;
    const unsigned char ORDER_BRG = 5;

  private:

    unsigned char mCommand;
    
    const unsigned char pp_command_magic[] = { 0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5, 0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09,0xaf,0x50 };

    short   mParameter;
    String  mSsid;
    String  mKey;
    byte    mSecurity;
  
    int     mNumStrips;
    int     mStripLength;
    byte[]  mStripType;
    byte[]  mColourOrder;

    short   mGroup;
    short   mController;
  
    short   mArtnetUniverse;
    short   mArtnetChannel;
  
/*  enum Security {
    NONE = 0,
    WEP  = 1,
    WPA  = 2,
    WPA2 = 3
 };
 
 typedef enum ColourOrder {RGB=0, RBG=1, GBR=2, GRB=3, BGR=4, BRG=5} ColourOrder;
 
  */
  };
