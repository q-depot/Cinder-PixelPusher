/*
 *  DeviceHeader.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2014 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef PIXEL_PUSHER_DEVICE_HEADER
#define PIXEL_PUSHER_DEVICE_HEADER

enum DeviceType {
  ETHERDREAM, 
  LUMIABRIDGE, 
  PIXELPUSHER
};

#include <stdexcept>
#include <string>

#define PP_HEADER_LENGTH 24


class DeviceHeader {
  
  public:
    
    DeviceHeader( uint8_t *packet, int packetLength )
    {
        if ( packetLength < PP_HEADER_LENGTH )
            throw std::invalid_argument("...");

        memcpy( &mMacAddress[0],    &packet[0], 6);             // MAC
        memcpy( &mIpAddress[0],     &packet[6], 4 );            // IP

        mDeviceType         = (DeviceType)packet[10];
        mProtocolVersion    = packet[11];
        
        memcpy( &mVendorId,             &packet[12], 2 );
        memcpy( &mProductId,            &packet[14], 2 );
        memcpy( &mHardwareRevision,     &packet[16], 2 );
        memcpy( &mSoftwareRevision,     &packet[18], 2 );
        memcpy( &mLinkSpeed,            &packet[20], 4 );
        
        mPacketRemainderSize    = packetLength - PP_HEADER_LENGTH;
        mPacketRemainder        = std::shared_ptr<uint8_t>( new uint8_t[mPacketRemainderSize] );
        memcpy( &mPacketRemainder.get()[0],   &packet[PP_HEADER_LENGTH], mPacketRemainderSize );
    }
    
    
    ~DeviceHeader() {}
    
    std::string getMacAddressString()
    {
        char buffer [24];
        sprintf( buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mMacAddress[0], mMacAddress[1], mMacAddress[2], mMacAddress[3], mMacAddress[4], mMacAddress[5] );
        return buffer;
    }
    
    std::string getIpAddressString()
    {
        std::string str =   std::to_string(mIpAddress[0]) + "." +
                            std::to_string(mIpAddress[1]) + "." +
                            std::to_string(mIpAddress[2]) + "." +
                            std::to_string(mIpAddress[3]);
        return str;
    }
    
    std::shared_ptr<uint8_t>    getPacketReminder()     { return mPacketRemainder; }
    int                         getPacketReminderSize() { return mPacketRemainderSize; }
    
    DeviceType  getDeviceType()         { return mDeviceType; }
    uint32_t    getProtocolVersion()    { return mProtocolVersion; }
    uint32_t    getVendorId()           { return mVendorId; }
    uint32_t    getProductId()          { return mProductId; }
    uint32_t    getHardwareRevision()   { return mHardwareRevision; }
    uint32_t    getSoftwareRevision()   { return mSoftwareRevision; }
    uint32_t    getLinkSpeed()          { return mLinkSpeed; }
    
    bool        isMulticast()           { return ( mIpAddress[0] >= 224 && mIpAddress[0] <= 239 ); }
    
  private:

    uint8_t         mMacAddress[6];
    uint8_t         mIpAddress[4];
    DeviceType      mDeviceType;
    uint8_t         mProtocolVersion;
    uint16_t        mVendorId;
    uint16_t        mProductId;
    uint16_t        mHardwareRevision;
    uint16_t        mSoftwareRevision;
    uint32_t        mLinkSpeed;
    
    std::shared_ptr<uint8_t>    mPacketRemainder;
    int                         mPacketRemainderSize;
    
private:

};


/*

Device Header format:
uint8_t mac_address[6];
uint8_t ip_address[4];
uint8_t device_type;
uint8_t protocol_version; // for the device, not the discovery
uint16_t vendor_id;
uint16_t product_id;
uint16_t hw_revision;
uint16_t sw_revision;
uint32_t link_speed; // in bits per second

 typedef struct pixel _PACKED_ {
 uint8_t red;
 uint8_t green;
 uint8_t blue;
 } pixel_t;
 
 // the packet goes like:
 
 uint32_t sequence_number;  // monotonically ascends, per-pusher.
 while (packet_not_full_up) {
 uint8_t strip_number;
 pixel_t strip_data[NUMBER_OF_PIXELS];  // you must fill at least one entire strip.
 }
 
 Here's a C header for the discovery packet as of firmware 1.5:
 

typedef enum DeviceType { ETHERDREAM = 0,
    LUMIABRIDGE = 1,
    PIXELPUSHER = 2 } DeviceType;

typedef struct PixelPusher {
    uint8_t  strips_attached;
    uint8_t  max_strips_per_packet;
    uint16_t pixels_per_strip;  // uint16_t used to make alignment work
    uint32_t update_period; // in microseconds
    uint32_t power_total;   // in PWM units
    uint32_t delta_sequence;  // difference between received and expected sequence numbers
    int32_t controller_ordinal; // ordering number for this controller.
    int32_t group_ordinal;      // group number for this controller.
    uint16_t artnet_universe;   // configured artnet starting point for this controller
    uint16_t artnet_channel;
    uint16_t my_port;
} PixelPusher;

typedef struct LumiaBridge {
    // placekeeper
} LumiaBridge;

typedef struct EtherDream {
    uint16_t buffer_capacity;
    uint32_t max_point_rate;
    uint8_t light_engine_state;
    uint8_t playback_state;
    uint8_t source;     //   0 = network
    uint16_t light_engine_flags;
    uint16_t playback_flags;
    uint16_t source_flags;
    uint16_t buffer_fullness;
    uint32_t point_rate;                // current point playback rate
    uint32_t point_count;           //  # points played
} EtherDream;

typedef union {
    PixelPusher pixelpusher;
    LumiaBridge lumiabridge;
    EtherDream etherdream;
} Particulars;

typedef struct DiscoveryPacketHeader {
    uint8_t mac_address[6];
    uint8_t ip_address[4];  // network byte order
    uint8_t device_type;
    uint8_t protocol_version; // for the device, not the discovery
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t hw_revision;
    uint16_t sw_revision;
    uint32_t link_speed;    // in bits per second
} DiscoveryPacketHeader;

typedef struct DiscoveryPacket {
    DiscoveryPacketHeader header;
    Particulars p;
} DiscoveryPacket;
*/

#endif