
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

#define         PP_CMD_MAGIC_SIZE   16
const uint8_t   PP_CMD_MAGIC[PP_CMD_MAGIC_SIZE] = { 0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5, 0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50 };

class PusherCommand;
typedef std::shared_ptr<PusherCommand> PusherCommandRef;

class PusherCommand {

    friend class PixelPusher;
    
public:
    
    enum PusherCmdType {
        RESET                   = 1,
        GLOBALBRIGHTNESS_SET    = 2,
        WIFI_CONFIGURE          = 3,
        LED_CONFIGURE           = 4,
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
    
    
public:
    
    static PusherCommandRef createReset()
    {
        int                         dataSize    = PP_CMD_MAGIC_SIZE + 1;
        std::shared_ptr<uint8_t>    data        = std::shared_ptr<uint8_t>( new uint8_t[dataSize] );
        
        std::memcpy( &data.get()[0], &PP_CMD_MAGIC[0], PP_CMD_MAGIC_SIZE );
        
        data.get()[PP_CMD_MAGIC_SIZE] = (uint8_t)RESET;
        
        return PusherCommandRef( new PusherCommand( data, dataSize ) );
    }
    

     // NOT Support with PixelPusher, only with the PixelPusher Photon
    static PusherCommandRef createGlobalBrightness( uint16_t brightness )
    {
        int                         dataSize    =  PP_CMD_MAGIC_SIZE + 3;
        std::shared_ptr<uint8_t>    data        = std::shared_ptr<uint8_t>( new uint8_t[dataSize] );
        
        std::memcpy( &data.get()[0], &PP_CMD_MAGIC[0], PP_CMD_MAGIC_SIZE );
        
        data.get()[PP_CMD_MAGIC_SIZE]   = (uint8_t)GLOBALBRIGHTNESS_SET;
        data.get()[PP_CMD_MAGIC_SIZE+1] = (uint8_t)( brightness & 0xff);
        data.get()[PP_CMD_MAGIC_SIZE+2] = (uint8_t)( ( brightness >> 8 ) & 0xff );
        
        return PusherCommandRef( new PusherCommand( data, dataSize ) );
    }
    
    /*
    static PusherCommandRef createWirelessConfig( std::string ssid, std::string key, PusherSecurity security )
    {
        // cmd_magic(16) + cmd(1) + ssid_length(+null terminator) + key_length(+null terminator) + security_type(1)
        int                         dataSize    = PP_CMD_MAGIC_SIZE + 1 + 1 + ssid.length() + 1 + key.length() + 1;;
        std::shared_ptr<uint8_t>    data        = std::shared_ptr<uint8_t>( new uint8_t[dataSize] );
        
        // cmd magic
        std::memcpy( &data.get()[0], &PP_CMD_MAGIC[0], PP_CMD_MAGIC_SIZE );
        
        // cmd
        data.get()[ PP_CMD_MAGIC_SIZE ] = WIFI_CONFIGURE;
        
        int totalLength = PP_CMD_MAGIC_SIZE + 1;
        
        // Ssid
        for ( int i=0; i < ssid.length(); i++ )
            data.get()[totalLength++] = (uint8_t)ssid[i];
        data.get()[totalLength++] = (uint8_t)0;             // null terminator
        
        // Key
        for ( int i=0; i < key.length(); i++ )
            data.get()[totalLength++] = (uint8_t)key[i];
        data.get()[totalLength++] = (uint8_t)0;             // null terminator
        
        data.get()[totalLength] = (uint8_t)security;
        
        return PusherCommandRef( new PusherCommand( data, dataSize ) );
    }

    
    static PusherCommandRef createStripsConfig( int32_t numStrips, int32_t stripLength,
                                                std::vector<PusherStripType> stripType, std::vector<PusherColorOrder> colorOrder,
                                                uint16_t group = 0, uint16_t controller = 0,
                                                uint16_t artnetUniverse = 0, uint16_t artnetChannel = 0 )
    {
        // ensure we have 8 values for strip type and order
        if ( stripType.size() != 8 )
            throw std::invalid_argument("Strip type vector size must be 8");
        
        if ( stripType.size() != 8 )
            throw std::invalid_argument("Color order vector size must be 8");
        
        // cmd_magic(16) + cmd(1) + num_strips(4) + strip_length(4) + strip_type(8) + color_order(8) + group(2) + controller(2) + artnet_univ(2) + artnet_ch(2)
        int                         dataSize    =  PP_CMD_MAGIC_SIZE + 1 + 4 + 4 + 8 + 8 + 2 + 2 + 2 + 2;
        std::shared_ptr<uint8_t>    data        = std::shared_ptr<uint8_t>( new uint8_t[dataSize] );
        
        // cmd magic
        std::memcpy( &data.get()[0], &PP_CMD_MAGIC[0], PP_CMD_MAGIC_SIZE );
        
        // cmd
        data.get()[ PP_CMD_MAGIC_SIZE ] = LED_CONFIGURE;
        
        // convert enum(32bit) to uint8_t
        uint8_t stripType_t[8];
        uint8_t colorOrder_t[8];
        
        for( int k=0; k < 8; k++ )
        {
            stripType_t[k]   = (uint8_t)stripType[k];
            colorOrder_t[k]  = (uint8_t)colorOrder[k];
        }
        
        // paramrters
        memcpy( &data.get()[PP_CMD_MAGIC_SIZE + 1],    &numStrips,          4 );
        memcpy( &data.get()[PP_CMD_MAGIC_SIZE + 5],    &stripLength,        4 );
        memcpy( &data.get()[PP_CMD_MAGIC_SIZE + 9],    &stripType_t[0],     8 );
        memcpy( &data.get()[PP_CMD_MAGIC_SIZE + 17],   &colorOrder_t[0],    8 );
        memcpy( &data.get()[PP_CMD_MAGIC_SIZE + 25],   &group,              2 );
        memcpy( &data.get()[PP_CMD_MAGIC_SIZE + 27],   &controller,         2 );
        memcpy( &data.get()[PP_CMD_MAGIC_SIZE + 29],   &artnetUniverse,     2 );
        memcpy( &data.get()[PP_CMD_MAGIC_SIZE + 31],   &artnetChannel,      2 );
        
        return PusherCommandRef( new PusherCommand( data, dataSize ) );
    }
    */
    
    ~PusherCommand() {}
    
    
private:
    
    PusherCommand( std::shared_ptr<uint8_t> data, int dataSize ) : mData(data), mDataSize(dataSize) {}
    
    std::shared_ptr<uint8_t>    mData;
    int                         mDataSize;
};

#endif
