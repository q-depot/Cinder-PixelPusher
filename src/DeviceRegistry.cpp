
#include "cinder/app/AppNative.h"
#include "DeviceRegistry.h"
#include "PixelPusher.h"
#include "PusherGroup.h"

double   DeviceRegistry::OverallBrightnessScale     = 1.0;
bool     DeviceRegistry::UseOverallBrightnessScale  = false;
uint64_t DeviceRegistry::TotalPower                 = 0;
uint64_t DeviceRegistry::TotalPowerLimit            = -1;
double   DeviceRegistry::PowerScale                 = 1.0;
bool     DeviceRegistry::AutoThrottle               = false;
bool     DeviceRegistry::AntiLog                    = false;
int      DeviceRegistry::FrameLimit                 = 85;
bool     DeviceRegistry::HasDiscoveryListener       = false;
bool     DeviceRegistry::AlreadyExist               = false;


bool sortPushers( PixelPusherRef a, PixelPusherRef b )
{
    int group_a = a->getGroupOrdinal();
    int group_b = b->getGroupOrdinal();

    if ( group_a != group_b )
    {
        if ( group_a < group_b )
            return false;
        
        return true;
    }
    
    int ord_a = a->getControllerOrdinal();
    int ord_b = b->getControllerOrdinal();
    
    if (ord_a != ord_b)
    {
        if (ord_a < ord_b)
            return false;
        
        return true;
    }
    
    return ( a->getMacAddress() < b->getMacAddress() );
}


DeviceRegistry::DeviceRegistry( boost::asio::io_service& ioService ) : mIoService(ioService)
{
    // Initialize a server by passing a boost::asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it
	// manually (i.e., call poll(), poll_one(), run(), etc).
	mServer = UdpServer::create( mIoService );
    
	// Add callbacks to work with the server asynchronously.
	mServer->connectAcceptEventHandler( &DeviceRegistry::onAccept,  this );
	mServer->connectErrorEventHandler(  &DeviceRegistry::onError,   this );

    if ( mServer )
    {
        // This is how you start listening for a connection. Once
        // a connection occurs, a session will be created and passed
        // in through the onAccept method.
        mServer->accept( (uint16_t)DR_DISCOVERY_PORT );
        
        ci::app::console() << "Listening on port: " << DR_DISCOVERY_PORT << std::endl;
    }
    else
    {
        ci::app::console() << "DeviceRegistry failed to create a server" << std::endl;
    }
}


void DeviceRegistry::onAccept( UdpSessionRef session )
{
    // Get the session from the argument and set callbacks.
    mSession = session;
    mSession->connectErrorEventHandler( &DeviceRegistry::onError,   this );
    mSession->connectReadEventHandler(  &DeviceRegistry::onRead,    this );
    
    // Start reading data from the client.
    mSession->read();
}


void DeviceRegistry::onError( std::string err, size_t bytesTransferred )
{
    ci::app::console() << "DeviceRegistry socket error!" << std::endl;
}


void DeviceRegistry::onRead( ci::Buffer buffer )
{
    uint8_t         *data   =(uint8_t*)buffer.getData();
    DeviceHeader    header  = DeviceHeader( data, buffer.getDataSize() );
    std::string     macAddr = header.getMacAddressString();
    
    // ignore non-PixelPusher devices
    if ( header.getDeviceType() != DeviceType::PIXELPUSHER )
        return;
    
    PixelPusherRef device = PixelPusher::create( header );
    device->setAntiLog( AntiLog );
    
    // Set the timestamp for the last time this device checked in
    mPusherLastSeenMap[macAddr] = ci::app::getElapsedSeconds(); //, System.nanoTime());
    
    if ( mPusherMap.find( macAddr ) == mPusherMap.end() )
    {
        addNewPusher( device );
    }
    
    else
    {
        PixelPusherRef thisDevice = mPusherMap[ macAddr ];
        
//        if (!pusherMap.get(macAddr).equals(device)) { // we already saw it but it's changed.
        if ( !thisDevice->isEqual( device ) )
        {
            thisDevice->copyHeader( device );

//            setChanged();
//            notifyObservers(device);
        }
        else
        {
            // The device is identical, nothing important has changed
            thisDevice->updateVariables( device );
            
            // if we dropped more than occasional packets, slow down a little
            if ( device->getDeltaSequence() > 3)
                thisDevice->increaseExtraDelay(5);
            
            if (device->getDeltaSequence() < 1)
                thisDevice->decreaseExtraDelay(1);
        }
    }
    
    // update the power limit variables
    if ( TotalPowerLimit > 0 )
    {
        TotalPower = 0;
        
        for( size_t k=0; k < mPushersSorted.size(); k++ )
            TotalPower += mPushersSorted[k]->getPowerTotal();

        PowerScale = ( TotalPower > TotalPowerLimit) ? ( TotalPowerLimit / TotalPower ) : 1.0;
    }
}

    
void DeviceRegistry::addNewPusher( PixelPusherRef pusher )
{
    ci::app::console() << "New device: " << pusher->getMacAddress() << " has group ordinal " << pusher->getGroupOrdinal() << std::endl;
    
    mPusherMap[ pusher->getMacAddress() ] = pusher;
    
    mPushersSorted.push_back( pusher );
    
        
    if ( mGroupMap.find( pusher->getGroupOrdinal() ) != mGroupMap.end() )
        mGroupMap[ pusher->getGroupOrdinal() ].addPusher( pusher );
    
    else
    {
        // we need to create a PusherGroup since it doesn't exist yet.
        PusherGroup pg = PusherGroup();

        ci::app::console() << "Creating group and adding pusher to group " << pusher->getGroupOrdinal() << std::endl;

        pg.addPusher(pusher);
        mGroupMap[ pusher->getGroupOrdinal() ] = pg;
    }
    
    pusher->setAutoThrottle( AutoThrottle );
    
    if ( pusher->isIpAddrMulticast() )
    {
        pusher->setMulticast(true);
        
        std::vector<PixelPusherRef> members = getPushersByIp( pusher->getIp() );
        
        bool groupHasPrimary = false;
        
        for( size_t k=0; k < members.size(); k++ )
            if ( members[k]->isMulticastPrimary() )
                groupHasPrimary = true;
        
        if ( !groupHasPrimary )
            pusher->setMulticastPrimary(true);
    }
    
    pusher->createCardThread( mIoService );
}



std::vector<PixelPusherRef> DeviceRegistry::getPushersByIp( std::string ip )
{
    std::vector<PixelPusherRef> pushers;
    
    for( size_t k=0; k < mPushersSorted.size(); k++ )
        if ( mPushersSorted[k]->getIp() == ip )
            pushers.push_back( mPushersSorted[k] );
    
    return pushers;
}

