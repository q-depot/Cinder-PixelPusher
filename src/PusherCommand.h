
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

class PusherCommand;
typedef std::shared_ptr<PusherCommand> PusherCommandRef;

class PusherCommand {

public:
    
    enum PusherCmdType {
        RESET,
        GLOBALBRIGHTNESS_SET,
        WIFI_CONFIGURE,
        LED_CONFIGURE,
    };
    
    enum PusherStripType {
        STRIP_LPD8806,
        STRIP_WS2801,
        STRIP_WS2811,
        STRIP_APA102,
    };
    
    enum PusherColorOrder {
        ORDER_RGB,
        ORDER_RBG,
        ORDER_GBR,
        ORDER_GRB,
        ORDER_BGR,
        ORDER_BRG
    };
    
    enum PusherSecurity {
        NONE,
        WEP,
        WPA,
        WPA2
    };
    
//    static createResetCmd()
//    {
////        ci::Buffer
//    }
//    
//    PusherCommandBuffer {
//        uint8_t *data;
//        int     dataSize;
//    }
//
    struct PusherCommandData {
        std::shared_ptr<uint8_t>    data;
        int                         dataSize;
    };
    
    
    static PusherCommandData createResetCmd()
    {
        int                         dataSize = PP_CMD_MAGIC_SIZE + 1;
        std::shared_ptr<uint8_t>    data = std::shared_ptr<uint8_t>( new uint8_t[cmd.dataSize] );
        
        cmd.dataSize    = PP_CMD_MAGIC_SIZE + 1;
        cmd.data        = std::shared_ptr<uint8_t>( new uint8_t[cmd.dataSize] );
        
        std::memcpy( &cmd.data.get()[0], &pp_cmd_magic[0], PP_CMD_MAGIC_SIZE );
        cmd.data.get()[PP_CMD_MAGIC_SIZE] = RESET;
        
        return cmd;
    }

    
    static PusherCommandData createResetCmd()
    {
        PusherCommandData cmd;
        cmd.dataSize    = PP_CMD_MAGIC_SIZE + 1;
        cmd.data        = new uint8_t[cmd.dataSize];
        
        std::memcpy( &cmd.data[0], &pp_cmd_magic[0], PP_CMD_MAGIC_SIZE );
        cmd.data[PP_CMD_MAGIC_SIZE] = RESET;
        
        return cmd;
    }

    
    PusherCommand( PusherCmdType command )
    {
        mCommand = command;
    }
  
    PusherCommand( PusherCmdType command, short parameter)
    {
        mCommand    = command;
        mParameter  = parameter;
    }
  
    PusherCommand( PusherCmdType command, std::string ssid, std::string key, PusherSecurity security )
    {
        mCommand    = command;
        mSsid       = ssid;
        mKey        = key;
        mSecurity   = (uint8_t)security;
    }
  
    PusherCommand( PusherCmdType command, int32_t numStrips, int32_t stripLength,
                   std::vector<PusherStripType> stripType, std::vector<PusherColorOrder> colorOrder,
                   uint16_t group = 0, uint16_t controller = 0,
                   uint16_t artnetUniverse = 0, uint16_t artnetCh = 0 )
    {
        if ( stripType.size() != 8 )
            throw std::invalid_argument("Strip type vector size must be 8");
            
        if ( stripType.size() != 8 )
            throw std::invalid_argument("Color order vector size must be 8");
        
        for( int k=0; k < 8; k++ )
        {
            mStripType[k]   = (uint8_t)stripType[k];
            mColorOrder[k]  = (uint8_t)colorOrder[k];
        }
        
        mCommand        = command;
        mNumStrips      = numStrips;
        mStripLength    = stripLength;
        mGroup          = group;
        mController     = controller;
        mArtnetChannel  = artnetCh;
        mArtnetUniverse = artnetUniverse;
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
          
            memcpy( &cmd[PP_CMD_MAGIC_SIZE + 1],    &mNumStrips,        4 );
            memcpy( &cmd[PP_CMD_MAGIC_SIZE + 5],    &mStripLength,      4 );
            memcpy( &cmd[PP_CMD_MAGIC_SIZE + 9],    &mStripType[0],     8 );
            memcpy( &cmd[PP_CMD_MAGIC_SIZE + 17],   &mColorOrder[0],    8 );
            memcpy( &cmd[PP_CMD_MAGIC_SIZE + 25],   &mGroup,            2 );
            memcpy( &cmd[PP_CMD_MAGIC_SIZE + 27],   &mController,       2 );
            memcpy( &cmd[PP_CMD_MAGIC_SIZE + 29],   &mArtnetUniverse,   2 );
            memcpy( &cmd[PP_CMD_MAGIC_SIZE + 31],   &mArtnetChannel,    2 );
        }
        
        return cmd;
    }


  private:
    
    PusherCommand( std::shared_ptr<uint8_t> data, int dataSize ) : mCmdData(data), mCmdDataSize(dataSize) {}
    
    
    const uint8_t pp_cmd_magic[PP_CMD_MAGIC_SIZE] = { 0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5, 0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50 };
    
    PusherCmdType   mCommand;
    int             mParameter;
    std::string     mSsid;
    std::string     mKey;
    uint8_t         mSecurity;
  
    int32_t         mNumStrips;
    int32_t         mStripLength;
    uint8_t         mStripType[8];
    uint8_t         mColorOrder[8];

    uint16_t        mGroup;
    uint16_t        mController;
  
    uint16_t        mArtnetUniverse;
    uint16_t        mArtnetChannel;
    
    std::shared_ptr<uint8_t>    mCmdData;
    int                         mCmdDataSize;
};

#endif
