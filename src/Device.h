#include "DeviceHeader.h"

class Device {

public:

	Device( DeviceHeader header ) : mDeviceHeader(header) {}

	std::string getMacAddress()		{ return mDeviceHeader.getMacAddressString(); }
	std::string getIp()				{ return mDeviceHeader.getIpAddressString(); }
	DeviceType getDeviceType()		{ return mDeviceHeader.getDeviceType(); }
	uint32_t getProtocolVersion()	{ return mDeviceHeader.getProtocolVersion(); }
	uint32_t getVendorId()			{ return mDeviceHeader.getVendorId(); }
	uint32_t getProductId()			{ return mDeviceHeader.getProductId(); }
	uint32_t getHardwareRevision()	{ return mDeviceHeader.getHardwareRevision(); }
	uint32_t getSoftwareRevision()	{ return mDeviceHeader.getSoftwareRevision(); }
	uint64_t getLinkSpeed()         { return mDeviceHeader.getLinkSpeed(); }

private:

	DeviceHeader	mDeviceHeader;	

};

