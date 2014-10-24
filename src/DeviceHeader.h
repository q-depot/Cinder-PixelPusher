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

    DeviceHeader( char *packet, int packetLength ) {

      if ( packetLength < headerLength )
        throw std::invalid_argument("...");

        memcpy( &mMacAddress[0],    &packet[0], 6);            // MAC
        memcpy( &mIpAddress[0],     &packet[6], 4 );             // IP

        mDeviceType         = (DeviceType)packet[10];
        mProtocolVersion    = packet[11];
        
        memcpy( &mVendorId,             &packet[12], 2 );
        memcpy( &mProductId,            &packet[14], 2 );
        memcpy( &mHardwareRevision,     &packet[16], 2 );
        memcpy( &mSoftwareRevision,     &packet[18], 2 );
        memcpy( &mLinkSpeed,            &packet[20], 4 );
        memcpy( &mPacketRemainder[0],   &packet[headerLength], packetLength - headerLength );
  }

    std::string getMacAddressString()
    {
        char buffer [24];
        sprintf(buffer, "%02dx:%02dx:%02dx:%02dx:%02dx:%02dx", mMacAddress[0], mMacAddress[1], mMacAddress[2], mMacAddress[3], mMacAddress[4], mMacAddress[5] );
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
    
    DeviceType  getDeviceType()         { return mDeviceType; }
    uint32_t    getProtocolVersion()    { return mProtocolVersion; }
    uint32_t    getVendorId()           { return mVendorId; }
    uint32_t    getProductId()          { return mProductId; }
    uint32_t    getHardwareRevision()   { return mHardwareRevision; }
    uint32_t    getSoftwareRevision()   { return mSoftwareRevision; }
    uint64_t    getLinkSpeed()          { return mLinkSpeed; }
    
    
  private:

    char            mMacAddress[6];
    char            mIpAddress[4];
    DeviceType      mDeviceType;
    uint32_t        mProtocolVersion;
    uint32_t        mVendorId;
    uint32_t        mProductId;
    uint32_t        mHardwareRevision;
    uint32_t        mSoftwareRevision;
    uint64_t        mLinkSpeed;
    char            mPacketRemainder[1000];             // double check this length!!!! <<<<<<<<<<<<<<<<<<<<<<<<

    private:

    const int headerLength = 24;

};
