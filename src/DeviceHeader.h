  /**
   * Device Header format:
   * uint8_t mac_address[6];
   * uint8_t ip_address[4];
   * uint8_t device_type;
   * uint8_t protocol_version; // for the device, not the discovery
   * uint16_t vendor_id;
   * uint16_t product_id;
   * uint16_t hw_revision;
   * uint16_t sw_revision;
   * uint32_t link_speed; // in bits per second
   */

enum DeviceType {
  ETHERDREAM, 
  LUMIABRIDGE, 
  PIXELPUSHER
};

#include <stdexcept>

class DeviceHeader {
  
  public:

    DeviceHeader( uint8_t *packet, int packetLength ) {

      if ( packetLength < headerLength )
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
        
        mPacketRemainderSize    = packetLength - headerLength;
        mPacketRemainder        = new uint8_t[mPacketRemainderSize];
        memcpy( &mPacketRemainder[0],   &packet[headerLength], mPacketRemainderSize );
    }
    
    
    ~DeviceHeader()
    {
        delete[] mPacketRemainder;
    }
    
    
    std::string getMacAddressString()
    {
        char buffer [24];
        sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mMacAddress[0], mMacAddress[1], mMacAddress[2], mMacAddress[3], mMacAddress[4], mMacAddress[5] );
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
    
    uint8_t         *getPacketReminder() { return mPacketRemainder; }
    int             getPacketReminderSize() { return mPacketRemainderSize; }
    
    DeviceType  getDeviceType()         { return mDeviceType; }
    uint32_t    getProtocolVersion()    { return mProtocolVersion; }
    uint32_t    getVendorId()           { return mVendorId; }
    uint32_t    getProductId()          { return mProductId; }
    uint32_t    getHardwareRevision()   { return mHardwareRevision; }
    uint32_t    getSoftwareRevision()   { return mSoftwareRevision; }
    uint64_t    getLinkSpeed()          { return mLinkSpeed; }
    
    
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
    uint8_t         *mPacketRemainder;
    int             mPacketRemainderSize;
    
private:

    const int headerLength = 24;

};
