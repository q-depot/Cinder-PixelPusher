
#ifndef PIXEL_PUSHER_COMMAND
#define PIXEL_PUSHER_COMMAND

/*  const uint8_t pp_command_magic[16] =
  *   { 0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5, 0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50  };
  *
  * #define COMMAND_RESET                0x01
  * #define COMMAND_GLOBALBRIGHTNESS_SET 0x02
  * #define COMMAND_WIFI_CONFIGURE       0x03
  * #define COMMAND_LED_CONFIGURE        0x04
  */

#pragma once
#include <algorithm>

#define PP_CMD_MAGIC_SIZE 16

class PusherCommand {

public:
    
    enum CmdType {
        RESET                   = 0x01,
        GLOBALBRIGHTNESS_SET    = 0x02,
        WIFI_CONFIGURE          = 0x03,
        LED_CONFIGURE           = 0x04,
        STRIP_LPD8806           = 0,
        STRIP_WS2801            = 1,
        STRIP_WS2811            = 2,
        STRIP_APA102            = 3,
        ORDER_RGB               = 0,
        ORDER_RBG               = 1,
        ORDER_GBR               = 2,
        ORDER_GRB               = 3,
        ORDER_BGR               = 4,
        ORDER_BRG               = 5
    };
    
    PusherCommand( CmdType command)
    {
        mCommand = command;
    }
  
    PusherCommand( CmdType command, short parameter)
    {
        mCommand    = command;
        mParameter  = parameter;
    }
  
    PusherCommand( CmdType command, std::string ssid, std::string key, std::string security )
    {
        mCommand  = command;
        mSsid     = ssid;
        mKey      = key;
        
        std::transform( security.begin(), security.end(), security.begin(), ::tolower );
        
        if (security == "none" )        mSecurity = 0;
        else if (security == "wep" )    mSecurity = 1;
        else if (security == "wpa" )    mSecurity = 2;
        else if (security == "wpa2" )   mSecurity = 3;
    }
  
    PusherCommand( CmdType command, int numStrips, int stripLength, std::string stripType, std::string colourOrder)
    {
        PusherCommand( command, numStrips, stripLength, stripType, colourOrder, 0, 0, 0, 0 );
    }
  
    PusherCommand( CmdType command, int numStrips, int stripLength, std::string stripType, std::string colourOrder, int group, int controller)
    {
        PusherCommand( command, numStrips, stripLength, stripType, colourOrder, group, controller, 0, 0 );
    }
  
    PusherCommand( CmdType command, int numStrips, int stripLength, std::string stripType, std::string colourOrder, int group, int controller, int artnet_universe, int artnet_channel )
    {
        mCommand        = command;
        mNumStrips      = numStrips;
        mStripLength    = stripLength;
        mStripType      = stripType;
        mColourOrder    = colourOrder;
        mGroup          = group;
        mController     = controller;
        mArtnetChannel  = artnet_channel;
        mArtnetUniverse = artnet_universe;
    }
    
    
    ~PusherCommand() {}
    
    
    // TODO: TEST THIS!!!!! <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    std::vector<uint8_t> generateBytes()
    {
        std::vector<uint8_t> cmd;

        if ( mCommand == RESET )
        {
            cmd.resize( PP_CMD_MAGIC_SIZE + 1 );
            std::memcpy( &cmd[0], &pp_cmd_magic[0], PP_CMD_MAGIC_SIZE );
            cmd[PP_CMD_MAGIC_SIZE] = RESET;
        }
    
        else if ( mCommand == GLOBALBRIGHTNESS_SET)
        {
            cmd.resize( PP_CMD_MAGIC_SIZE + 3 );
            std::memcpy( &cmd[0], &pp_cmd_magic[0], PP_CMD_MAGIC_SIZE );
            cmd[PP_CMD_MAGIC_SIZE] = GLOBALBRIGHTNESS_SET;
            cmd[PP_CMD_MAGIC_SIZE] = (uint8_t) ( mParameter & 0xff);
            cmd[PP_CMD_MAGIC_SIZE] = (uint8_t) ( ( mParameter >> 8 ) & 0xff );
        }
    
        else if ( mCommand == WIFI_CONFIGURE )
        {
            int bufLength = 0;
            bufLength += ( PP_CMD_MAGIC_SIZE ) + 1;   // length of command
            bufLength += 1;                           // length of key type
            bufLength += mSsid.length() + 1;          // ssid plus null terminator
            bufLength += mKey.length() + 1;           // key plus null terminator
          
            cmd.resize( bufLength );
            std::memcpy( &cmd[0], &pp_cmd_magic[0], PP_CMD_MAGIC_SIZE );
      
            cmd[ PP_CMD_MAGIC_SIZE ] = mCommand;
          
            // TODO: use just 1 damn counter! <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            for ( int i=0; i < mSsid.length(); i++ )
                cmd[ PP_CMD_MAGIC_SIZE + 1 + i] = (uint8_t)mSsid[i];
          
            for ( int i=0; i < mKey.length(); i++ )
                cmd[ PP_CMD_MAGIC_SIZE + 1 + mSsid.length() + 1 + i ] = (uint8_t)mKey[i];
          
          
            cmd[ PP_CMD_MAGIC_SIZE + 1 + mKey.length() + 1 + mSsid.length() + 1 ] = mSecurity;
        }
    
        else if ( mCommand == LED_CONFIGURE )
        {
            // two ints, eight uint8_t, eight uint8_t, plus command, plus group and controller
            // plus artnet universe and channel
            cmd.resize( PP_CMD_MAGIC_SIZE + 33 );
            std::memcpy( &cmd[0], &pp_cmd_magic[0], PP_CMD_MAGIC_SIZE );

            cmd[ PP_CMD_MAGIC_SIZE ] = LED_CONFIGURE;
          
            cmd[ PP_CMD_MAGIC_SIZE + 1 + 0 ] = (uint8_t) (  mNumStrips & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 1 + 1 ] = (uint8_t) (( mNumStrips >> 8 ) & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 1 + 2 ] = (uint8_t) (( mNumStrips >> 16 ) & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 1 + 3 ] = (uint8_t) (( mNumStrips >> 24 ) & 0xFF );
          
            cmd[ PP_CMD_MAGIC_SIZE + 5 + 0 ] = (uint8_t) (  mStripLength & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 5 + 1 ] = (uint8_t) (( mStripLength >> 8 ) & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 5 + 2 ] = (uint8_t) (( mStripLength >> 16 ) & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 5 + 3 ] = (uint8_t) (( mStripLength >> 24 ) & 0xFF );

            for (int i = PP_CMD_MAGIC_SIZE + 9; i< PP_CMD_MAGIC_SIZE + 17; i++)
                cmd[i] = (uint8_t)mStripType[ i - ( PP_CMD_MAGIC_SIZE + 9 ) ];
          
            for (int i = PP_CMD_MAGIC_SIZE + 17; i< PP_CMD_MAGIC_SIZE + 25; i++)
                cmd[i] = (uint8_t)mColourOrder[ i - ( PP_CMD_MAGIC_SIZE + 17 ) ];
          
         
            cmd[ PP_CMD_MAGIC_SIZE + 25 + 0 ] = (uint8_t) (  mGroup & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 25 + 1 ] = (uint8_t) (( mGroup >> 8) & 0xFF );
          
            cmd[ PP_CMD_MAGIC_SIZE + 27 + 0 ] = (uint8_t) (  mController & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 27 + 1 ] = (uint8_t) (( mController >> 8 ) & 0xFF );
          
            cmd[ PP_CMD_MAGIC_SIZE + 29 + 0 ] = (uint8_t) (  mArtnetUniverse & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 29 + 1 ] = (uint8_t) (( mArtnetUniverse >> 8) & 0xFF );
          
            cmd[ PP_CMD_MAGIC_SIZE + 31 + 0 ] = (uint8_t) (  mArtnetChannel & 0xFF );
            cmd[ PP_CMD_MAGIC_SIZE + 31 + 1 ] = (uint8_t) (( mArtnetChannel >> 8 ) & 0xFF );
        }
        
        return cmd;
    }


  private:

    const uint8_t pp_cmd_magic[PP_CMD_MAGIC_SIZE] = { 0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5, 0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50 };
    
    CmdType         mCommand;
    int             mParameter;
    std::string     mSsid;
    std::string     mKey;
    uint8_t   mSecurity;
  
    int             mNumStrips;
    int             mStripLength;
    std::string     mStripType;
    std::string     mColourOrder;

    int             mGroup;
    int             mController;
  
    int             mArtnetUniverse;
    int             mArtnetChannel;
  
/*  enum Security {
    NONE = 0,
    WEP  = 1,
    WPA  = 2,
    WPA2 = 3
 };
 
 typedef enum ColourOrder {RGB=0, RBG=1, GBR=2, GRB=3, BGR=4, BRG=5} ColourOrder;
 
  */
};

#endif
