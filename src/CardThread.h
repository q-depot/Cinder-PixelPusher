
#pragma once

#include "DeviceRegistry.h"
#include "UdpClient.h"


class CardThread;
typedef std::shared_ptr<CardThread> CardThreadRef;

class PixelPusher;
typedef std::shared_ptr<PixelPusher>    PixelPusherRef;


class CardThread {

public:

    static CardThreadRef create( PixelPusherRef pusher, boost::asio::io_service& ioService )
    {
        return CardThreadRef( new CardThread( pusher, ioService ) );
    }
    
    ~CardThread() {}
    
//    void setExtraDelay( uint64_t msec ) { mThreadExtraDelayMsec = msec; }
    
//    bool controls(PixelPusher test) { return test.equals( mPusher); }

//    int getBandwidthEstimate() { return mBandwidthEstimate; }

    void run();
    
    void shutDown();
    
//    bool hasTouchedStrips();

//    void setAntiLog( bool antiLog );
    
private:
    
    CardThread( PixelPusherRef pusher, boost::asio::io_service& ioService  );
    
    void sendPacketToPusher();
    
    void onConnect( UdpSessionRef session );
	void onError( std::string err, size_t bytesTransferred );
    
private:
    
    UdpClientRef				mClient;
	UdpSessionRef				mSession;
	
    
    uint64_t        mThreadSleepMsec;
    uint64_t        mThreadExtraDelayMsec;
//    uint64_t        mBandwidthEstimate;
//    uint32_t        mMaxPacketSize;

    PixelPusherRef  mPusher;
//    uint8_t         *mPacket;
    ci::Buffer      mPacketBuffer;
    
    std::thread     mSendDataThread;
    bool            mRunThread;
//    DatagramPacket  mUdppacket;
    
//    bool            mCancel;

//    int             mPusherPort;
//    std::string     mCardAddress;
    uint64_t        mPacketNumber;
//    DeviceRegistry  mRegistry;
//    bool            mUseAntiLog;
    
//    bool            mFileIsOpen;
//    FileOutputStream mRecordFile;
//    uint64_t        mLastWorkTime;
//    uint64_t        mFirstSendTime;
//    bool            mIsTerminated;
    
};
