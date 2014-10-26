

#include "CardThread.h"
#include "cinder/Utilities.h"
//#include <thread>

// TODO: why do i need an extra class only to thread the output ?!?!?!!


CardThread::CardThread( PixelPusherRef pusher, boost::asio::io_service& ioService )//, DeviceRegistry dr ) : mRegistry(dr)
{
//    mThreadSleepMsec        = 4;
    mThreadExtraDelayMsec   = 0;
//    mBandwidthEstimate      = 0;
//    mMaxPacketSize          = 1460;
//    mIsTerminated           = false;;
    
    mPusher         = pusher;
//    mPusherPort     = mPusher->getPort();
//    mLastWorkTime   = ci::app::getElapsedSeconds();
//    mRegistry       = dr;
    
//    try {
//      this.udpsocket = new DatagramSocket();
//    } catch (SocketException se) {
//      System.err.println("SocketException: " + se.getMessage());
//    }
    
    
    int maxPacketSize  = 4 +  ( ( 1 + 3 * mPusher->getPixelsPerStrip() ) * mPusher->getMaxStripsPerPacket() );
//    mPacket         = new uint8_t[mMaxPacketSize];
    mPacketBuffer       = ci::Buffer( maxPacketSize );
    
//    mCardAddress    = mPusher->getIp();
    mPacketNumber   = 0;
//    mCancel         = false;
//    mFileIsOpen     = false;

    // Initialize a client by passing a boost::asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it
	// manually (i.e., call poll(), poll_one(), run(), etc).
	mClient = UdpClient::create( ioService );
    
	// Add callbacks to work with the client asynchronously.
	// Note that you can use lambdas.
	mClient->connectConnectEventHandler( &CardThread::onConnect, this );
	mClient->connectErrorEventHandler( &CardThread::onError, this );
//	mClient->connectResolveEventHandler( [ & ]()
//                                        {
//                                            mText.push_back( "Endpoint resolved" );
//                                        } );
    
    mClient->connect( mPusher->getIp(), mPusher->getPort() );
    
    if ( mPusher->getUpdatePeriod() > 100 && mPusher->getUpdatePeriod() < 1000000 )
        mThreadSleepMsec = ( mPusher->getUpdatePeriod() / 1000 ) + 1;
    
    mSendDataThread = std::thread( &CardThread::sendPacketToPusher, this );
}


void CardThread::shutDown()
{
    mPusher->shutDown();
    
    /*
    if ( mFileIsOpen )
    {
        try {
            mPusher->setAmRecording(false);
            mFileIsOpen = false;
            recordFile.close();
        }
        catch ( Exception e )
        {
           ci::app::console() << e.what() << std::endl;
        }
    }
    */
//    
//    mCancel = true;
//    
//    while ( !mIsTerminated )
//    {
//        try
//        {
//            ci::sleep( 10 );
//        }
//        catch ( Exception e )
//        {
//            ci::app::console() << "Interrupted terminating CardThread " << mPusher->getMacAddress() << std::endl;
//            ci::app::console() << e.what() << std::endl;
//        }
//    }
}


//bool CardThread::hasTouchedStrips()
//{
//    std::vector<StripRef> strips = mPusher->getStrips();
//    for( size_t k=0; k < strips.size(); k++ )
//        if ( strips[k]->isTouched() )
//            return true;
//
//    return false;
//}


//void CardThread::getAntiLog( bool antiLog )
//{
//    UseAntiLog = antiLog;
//    for (Strip strip: mPusher->getStrips())
//        strip.useAntiLog(useAntiLog);
//}


void CardThread::onConnect( UdpSessionRef session )
{
    ci::app::console() << "CardThread Connected" << std::endl;
    
	// Get the session from the argument and set callbacks.
	// Note that you can use lambdas.
	mSession = session;
	mSession->connectErrorEventHandler( &CardThread::onError, this );
}

void CardThread::onError( std::string err, size_t bytesTransferred )
{
    ci::app::console() << "CardThread ERROR: " << err << std::endl;
}


void CardThread::sendPacketToPusher()
{
    mRunThread = true;
    
    StripRef                strip;
    std::vector<StripRef>   touchedStrips;
    std::vector<PixelRef>   pixels;
    int                     packetLength;
    int                     maxStripsPerPacket;
    int                     stripPerPacket;
    size_t                  stripDataSize;
    long                    totalDelay;
    bool                    payload;
    int                     stripIdx;
    uint8_t                 *packetData = (uint8_t*)mPacketBuffer.getData();
    uint8_t                 *stripData;
    
    
	while( mRunThread )
    {
        if ( mSession && mSession->getSocket()->is_open() )
        {
//            int     totalLength = 0;
            
            // IF no commands queue AND no touched strips RETURN
            
            touchedStrips = mPusher->getTouchedStrips();
            
            maxStripsPerPacket  = mPusher->getMaxStripsPerPacket();
            stripPerPacket      = std::min( (uint8_t)maxStripsPerPacket, mPusher->getStripsAttached() );
            
            if ( mPusher->getUpdatePeriod() > 1000 )
                mThreadSleepMsec = ( mPusher->getUpdatePeriod() / 1000 ) + 1;
            else                                                                                    // Shoot for the framelimit.
                mThreadSleepMsec = ( ( 1000 / DeviceRegistry::getFrameLimit() ) / ( mPusher->getStripsAttached() / stripPerPacket ) );
            
            // Handle errant delay calculation in the firmware.
            if ( mPusher->getUpdatePeriod() > 100000 )
                mThreadSleepMsec = ( 16 / ( mPusher->getStripsAttached() / stripPerPacket ) );
            
            totalDelay = mThreadSleepMsec + mThreadExtraDelayMsec + mPusher->getExtraDelay();
            
            stripIdx = 0;
            
            // send packet
            while( stripIdx < touchedStrips.size() )
            {
                packetLength    = 0;
                payload         = false;
                
                // Packet number
                packetData[packetLength++] = mPacketNumber & 0xFF;
                packetData[packetLength++] = (mPacketNumber >> 8) & 0xFF;
                packetData[packetLength++] = (mPacketNumber >> 16) & 0xFF;
                packetData[packetLength++] = (mPacketNumber >> 24) & 0xFF;
                
                
                // first check to see if we have an outstanding command.
                
                bool commandSent = false;
                
                // SEND COMMANDS FIRST
                /*
                if (!(mPusher->commandQueue.isEmpty())) {
                    commandSent = true;
                    System.out.println("Pusher "+mPusher->getMacAddress()+" has a PusherCommand outstanding.");
                    PusherCommand pc = mPusher->commandQueue.remove();
                    byte[] commandBytes= pc.generateBytes();
                    
                    packetLength = 0;
                    packetNumberArray = ByteUtils.unsignedIntToByteArray(packetNumber, true);
                    for(int j = 0; j < packetNumberArray.length; j++) {
                        this.packet[packetLength++] = packetNumberArray[j];
                    }
                    for(int j = 0; j < commandBytes.length; j++) {
                        this.packet[packetLength++] = commandBytes[j];
                    }
                    // We need fixed size datagrams for the Photon, because the cc3000 sucks.
                    if ((mPusher->getPusherFlags() & mPusher->PFLAG_FIXEDSIZE) != 0) {
                        packetLength = 4 + ((1 + 3 * mPusher->getPixelsPerStrip()) * stripPerPacket);
                    }
                    packetNumber++;
                    udppacket = new DatagramPacket(packet, packetLength, cardAddress,
                                                   pusherPort);
                    try {
                        udpsocket.send(udppacket);
                    } catch (IOException ioe) {
                        System.err.println("IOException: " + ioe.getMessage());
                    }
                    
                    totalLength += packetLength;
                    
                } else {
                    commandSent = false;
                }
                */
                
                if ( !commandSent )
                {
                    
                    // Now loop over remaining strips.
                    for ( int k = 0; k < stripPerPacket; k++ )
                    {
                        if ( stripIdx >= touchedStrips.size() )
                            break;
                        
                        strip = touchedStrips[stripIdx++];
                        
                        
                        // add strip number
                        packetData[packetLength++] = strip->getStripNumber();
                        
                        // update pixels buffer
                        strip->updatePixelsBuffer();
                        
                        stripData       = strip->getPixelsData();
                        stripDataSize   = strip->getPixelsDataSize();
                        
                        
                        // add pixels data
                        memcpy( &packetData[packetLength], stripData, stripDataSize );
                        packetLength += stripDataSize;
                        
                        
                        // Don't weed untouched strips if we are recording.
                        //    if (!fileIsOpen) {
                        //        if (!strip.isTouched() && ((mPusher->getPusherFlags() & mPusher->PFLAG_FIXEDSIZE) == 0))
                        //            continue;
                        //    }
                        
                        payload = true;
                    }
                    
                    if ( payload )
                    {
                        // send packet
                        mSession->write( mPacketBuffer );
                        mPacketNumber++;
                        payload = false;
                    }
                }
            }
            std::this_thread::sleep_for( std::chrono::milliseconds( mThreadSleepMsec ) );
        }
        else
        {
            ci::app::console() << "Session is not open!" << std::endl;
            std::this_thread::sleep_for( std::chrono::milliseconds( mThreadSleepMsec ) );
        }
        
	}
    
    ci::app::console() << "CardThread::sendPacketToPusher() thread exited!" << std::endl;
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
