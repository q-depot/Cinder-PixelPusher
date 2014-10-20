#include "DeviceHeader.h"

class Device {

public:

	Device( DeviceHeader header ) : mDeviceHeader(header) {}

	std::string getMacAddress()		{ return mDeviceHeader.getMacAddressString(); }
	InetAddress getIp()				{ return mDeviceHeader.IpAddress; }
	DeviceType getDeviceType()		{ return mDeviceHeader.DeviceType; }
	int getProtocolVersion()		{ return mDeviceHeader.ProtocolVersion; }
	int getVendorId()				{ return mDeviceHeader.VendorId; }
	int getProductId()				{ return mDeviceHeader.ProductId; }
	int getHardwareRevision()		{ return mDeviceHeader.HardwareRevision; }
	int getSoftwareRevision()		{ return mDeviceHeader.SoftwareRevision; }
	long getLinkSpeed()				{ return mDeviceHeader.LinkSpeed; }

  	// std::string toString() {
   //  	return "Mac: " + mDeviceHeader.GetMacAddressString() + ", IP: "
   //      getHostAddress() + " Firmware revision: "+getSoftwareRevision(); 
   //  }

private:

	DeviceHeader	mDeviceHeader;	

};

