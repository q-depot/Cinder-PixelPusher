
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
        RESET                   = 1,    // only one supported, the others are for PixelPusher Photon
//        GLOBALBRIGHTNESS_SET    = 2,
//        WIFI_CONFIGURE          = 3,
//        LED_CONFIGURE           = 4,
    };
    
//    enum PusherStripType {
//        STRIP_LPD8806,
//        STRIP_WS2801,
//        STRIP_WS2811,
//        STRIP_APA102,
//    };
//    
//    enum PusherColorOrder {
//        ORDER_RGB,
//        ORDER_RBG,
//        ORDER_GBR,
//        ORDER_GRB,
//        ORDER_BGR,
//        ORDER_BRG
//    };
//    
//    enum PusherSecurity {
//        NONE,
//        WEP,
//        WPA,
//        WPA2
//    };
    
    
public:
    
    static PusherCommandRef createReset()
    {
        int                         dataSize    = PP_CMD_MAGIC_SIZE + 1;
        std::shared_ptr<uint8_t>    data        = std::shared_ptr<uint8_t>( new uint8_t[dataSize] );
        
        std::memcpy( &data.get()[0], &PP_CMD_MAGIC[0], PP_CMD_MAGIC_SIZE );
        
        data.get()[PP_CMD_MAGIC_SIZE] = (uint8_t)RESET;
        
        return PusherCommandRef( new PusherCommand( data, dataSize ) );
    }
    
    ~PusherCommand() {}
    
    
private:
    
    PusherCommand( std::shared_ptr<uint8_t> data, int dataSize ) : mData(data), mDataSize(dataSize) {}
    
    std::shared_ptr<uint8_t>    mData;
    int                         mDataSize;
};

#endif
