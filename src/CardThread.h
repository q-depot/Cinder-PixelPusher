

class PixelPusher;
typedef std::shared_ptr<PixelPusher>    PixelPusherRef;


class CardThread {

public:

    CardThread( PixelPusherRef pusher, DeviceRegistry dr );
    
    ~CardThread();
    
    void setExtraDelay( uint64_t msec ) { threadExtraDelayMsec = msec; }
    
//    bool controls(PixelPusher test) { return test.equals( mPusher); }

    int getBandwidthEstimate() { return mBandwidthEstimate; }

    void run();
    
    void shutDown();
    
    bool hasTouchedStrips();

    void setAntiLog( bool antiLog );
    
private:

    int sendPacketToPusher( PixelPusherRef pusher );

private:
    
    uint64_t        mThreadSleepMsec;
    uint64_t        mThreadExtraDelayMsec;
    uint64_t        mBandwidthEstimate;
    uint32_t        mMaxPacketSize;

    PixelPusherRef  mPusher;
    uint8_t         *mPacket;
    
    DatagramPacket  mUdppacket;
    DatagramSocket  mUdpsocket;
    
    bool            mCancel;

    int             mPusherPort;
    InetAddress     mCardAddress;
    uint64_t        mPacketNumber;
    DeviceRegistry  mRegistry;
    bool            mUseAntiLog;
    bool            mFileIsOpen;
    FileOutputStream mRecordFile;
    uint64_t        mLastWorkTime;
    uint64_t        mFirstSendTime;
    bool            mIsTerminated;
};
