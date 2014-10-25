


#include "CardThread.h"
#include "PixelPusher.h"


CardThread( PixelPusherRef pusher, DeviceRegistry dr )
{
    mThreadSleepMsec        = 4;
    mThreadExtraDelayMsec   = 0;
    mBandwidthEstimate      = 0;
    mMaxPacketSize          = 1460;
    mIsTerminated           = false;;
      
//    super("CardThread for PixelPusher "+pusher.getMacAddress());
    
    mPusher         = pusher;
    mPusherPort     = mPusher->getPort();
    mPastWorkTime   = ci::app::getElapsedSeconds();
    mRegistry       = dr;
    
//    try {
//      this.udpsocket = new DatagramSocket();
//    } catch (SocketException se) {
//      System.err.println("SocketException: " + se.getMessage());
//    }
    
    
    mMaxPacketSize  = 4 +  ( ( 1 + 3 * mPusher->getPixelsPerStrip() ) * mPusher->getMaxStripsPerPacket() );
    mPacket         = new uint8_t[mMaxPacketSize];
    mCardAddress    = mPusher->getIp();
    mPacketNumber   = 0;
    mCancel         = false;
    mFileIsOpen     = false;

    if ( mPusher->getUpdatePeriod() > 100 && mPusher->getUpdatePeriod() < 1000000 )
        mThreadSleepMsec = ( mPusher->getUpdatePeriod() / 1000 ) + 1;
}


CardThread::~CardThread()
{
    delete mPacket;
}


void CardThread::run()
{
    while ( !mCancel )
    {
        if ( mPusher->isMulticast() )
        {
            if ( !mPusher->isMulticastPrimary() )
            {
                ci::sleep(1000);
                continue;           // we just sleep until we're primary
            }
        }
        
      
        int         bytesSent;
        uint64_t    startTime = ci::app::getElapsedSeconds() * 1000000000; // in nano seconds
        
        // check to see if we're supposed to be recording.
        if ( mPusher->isAmRecording())
        {
            if ( !mFileIsOpen )
            {
                try
                {
                    recordFile      = new FileOutputStream(new File( mPusher->getFilename()));
                    mFileIsOpen     = true;
                    firstSendTime   = ci::app::getElapsedSeconds() * 1000000000; // in nano seconds
                }
                catch ( Exception e )
                {
                    ci::app::console() << "Failed to open recording file " << mPusher->getFilename() << std:endl;
                    mPusher->setAmRecording(false);
                }
            }
        }
        
        
        bytesSent = sendPacketToPusher( mPusher );
      
        int requestedStripsPerPacket  = mPusher->getMaxStripsPerPacket();
        int stripPerPacket            = std::min( requestedStripsPerPacket, mPusher->stripsAttached );
      
      if (bytesSent == 0) {
        try {
          long estimatedSleep = (System.nanoTime() - lastWorkTime)/1000000;
          estimatedSleep = Math.min(estimatedSleep, ((1000/registry.getFrameLimit()) 
                                      / (mPusher->stripsAttached / stripPerPacket)));
          
          Thread.sleep(estimatedSleep);
        } catch (InterruptedException e) {
          // Don't care if we get interrupted.
        }
        }
      else {
        lastWorkTime = System.nanoTime();
      }
      long endTime = System.nanoTime();
      long duration = ((endTime - startTime) / 1000000);
      if (duration > 0)
        bandwidthEstimate = bytesSent / duration;
    }
    
    mIsTerminated = true;
}


void CardThread::shutDown()
{
    mPusher->shutDown();
    
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
    
    mCancel = true;
    
    while ( !mIsTerminated )
    {
        try
        {
            ci::sleep( 10 );
        }
        catch ( Exception e )
        {
            ci::app::console() << "Interrupted terminating CardThread " << mPusher->getMacAddress() << std::endl;
            ci::app::console() << e.what() << std::endl;
        }
    }
}


bool CardThread::hasTouchedStrips()
{
    std::vector<StripRef> strips = mPusher->getStrips();
    for( size_t k=0; k < strips.size(); k++ )
        if ( strips[k]->isTouched() )
            return true;

    return false;
}


void sCardThread::etAntiLog( bool antiLog )
{
    useAntiLog = antiLog;
    for (Strip strip: mPusher->getStrips())
        strip.useAntiLog(useAntiLog);
}


int CardThread::sendPacketToPusher( PixelPusher pusher )
{
    int         packetLength;
    uint64_t    totalDelay;
    bool        payload;
    int         stripsInDatagram;
    int         totalLength = 0;
    double      powerScale  = registry.getPowerScale();

    
    std::vector<StripRef> remainingStrips;

    if ( !mPusher->hasTouchedStrips() )
    {
        // System.out.println("Yielding because no touched strips.");
        if ( mPusher->commandQueue.isEmpty() )
            return 0;
    }

    if ( mPusher->isBusy() )
    {
    // System.out.println("Yielding because pusher is busy.");
    return 0;
    }

    mPusher->makeBusy();
    // System.out.println("Making pusher busy.");

      remainingStrips = new CopyOnWriteArrayList<Strip>(pusher.getStrips());
      stripsInDatagram = 0;

      int requestedStripsPerPacket = pusher.getMaxStripsPerPacket();
      int stripPerPacket = Math.min(requestedStripsPerPacket, pusher.stripsAttached);

      while (!remainingStrips.isEmpty()) {
        packetLength = 0;
        payload = false;
        if (pusher.getUpdatePeriod() > 1000) {
          this.threadSleepMsec = (pusher.getUpdatePeriod() / 1000) + 1;
        } else {
          // Shoot for the framelimit.
          this.threadSleepMsec = ((1000/registry.getFrameLimit()) / (pusher.stripsAttached / stripPerPacket));
        }

        // Handle errant delay calculation in the firmware.
        if (pusher.getUpdatePeriod() > 100000)
          this.threadSleepMsec = (16 / (pusher.stripsAttached / stripPerPacket));

        totalDelay = threadSleepMsec + threadExtraDelayMsec + pusher.getExtraDelay();

        byte[] packetNumberArray = ByteUtils.unsignedIntToByteArray(packetNumber, true);
        for(int i = 0; i < packetNumberArray.length; i++) {
          this.packet[packetLength++] = packetNumberArray[i];
        }

        // first check to see if we have an outstanding command.

        boolean commandSent;

        if (!(pusher.commandQueue.isEmpty())) {
          commandSent = true;
          System.out.println("Pusher "+pusher.getMacAddress()+" has a PusherCommand outstanding.");
          PusherCommand pc = pusher.commandQueue.remove();
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
          if ((pusher.getPusherFlags() & pusher.PFLAG_FIXEDSIZE) != 0) {
             packetLength = 4 + ((1 + 3 * pusher.getPixelsPerStrip()) * stripPerPacket);
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

        if (!commandSent) {
          int i;
          // Now loop over remaining strips.
          for (i = 0; i < stripPerPacket; i++) {
            if (remainingStrips.isEmpty()) {
              break;
            }
            Strip strip = remainingStrips.remove(0);
            // Don't weed untouched strips if we are recording.
            if (!fileIsOpen) {
              if (!strip.isTouched() && ((pusher.getPusherFlags() & pusher.PFLAG_FIXEDSIZE) == 0))
                continue;
            }

            stripsInDatagram++;
            
            strip.setPowerScale(powerScale);
            byte[] stripPacket;
            
            if (!DeviceRegistry.useOverallBrightnessScale) {
              stripPacket = strip.serialize();
            } else {
              stripPacket = strip.serialize(DeviceRegistry.getOverallBrightnessScale());
            }
            strip.setPushedAt(System.nanoTime());
            strip.markClean();
            this.packet[packetLength++] = (byte) strip.getStripNumber();
            if (fileIsOpen) {
              try {
                // we need to make the pusher wait on playback the same length of time between strips as we wait between packets
                // this number is in microseconds.
                if (stripsInDatagram > 1) { // only write the delay for the first strip in a datagram.
                  if ((System.nanoTime() - firstSendTime / 1000) < (25 * 60 * 1000000)) {
                    recordFile.write(ByteUtils.unsignedIntToByteArray((int)0, true));
                  } else {
                    // write the timer reset magic - we do this into a sentence that would otherwise have no timing info
                    recordFile.write(ByteUtils.unsignedIntToByteArray((int)0xdeadb33f, true));
                    // and reset the timer
                    firstSendTime = System.nanoTime();
                  }
                } else {
                  if (firstSendTime != 0) { // this is not the first send to this pusher, we know how long it's been
                    recordFile.write(ByteUtils.unsignedIntToByteArray((int)((System.nanoTime() - firstSendTime) / 1000), true));
                  } else { // fall back to the update period.
                    recordFile.write(ByteUtils.unsignedIntToByteArray((int)pusher.getUpdatePeriod(), true));
                  }
                }
                recordFile.write((byte) strip.getStripNumber());
                recordFile.write(stripPacket);
              } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
              }
            }
            for (int j = 0; j < stripPacket.length; j++) {
              this.packet[packetLength + j] = stripPacket[j];
            }
            packetLength += stripPacket.length;
            payload = true;
          }
          if (payload) {
            //System.out.println("Got a payload to send to "+cardAddress);
            packetNumber++;
            /* System.err.println(" Packet number array = length "+ packetLength +
             *      " seq "+ packetNumber +" data " + String.format("%02x, %02x, %02x, %02x",
             *          packetNumberArray[0], packetNumberArray[1], packetNumberArray[2], packetNumberArray[3]));
             */
            udppacket = new DatagramPacket(packet, packetLength, cardAddress,
                pusherPort);
            try {
              udpsocket.send(udppacket);
              //System.out.println("Sent it.");
            } catch (IOException ioe) {
              System.err.println("IOException: " + ioe.getMessage());
            }
          }
          totalLength += packetLength;
        }

        packetLength = 0;
        payload = false;
        try {
          Thread.sleep(totalDelay);
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
      }
      //System.out.println("Clearing busy.");
      pusher.clearBusy();
      return totalLength;
  }



  private long threadSleepMsec = 4;
  private long threadExtraDelayMsec = 0;
  private long bandwidthEstimate = 0;
  private int maxPacketSize = 1460;
  private PixelPusher pusher;
  private byte[] packet;
  private DatagramPacket udppacket;
  private DatagramSocket udpsocket;
  private boolean cancel;
  private int pusherPort;
  private InetAddress cardAddress;
  private long packetNumber;
  private DeviceRegistry registry;
  private boolean useAntiLog;
  private boolean fileIsOpen;
  FileOutputStream recordFile;
  private long lastWorkTime;
  private long firstSendTime;
  public boolean terminated=false;
};
