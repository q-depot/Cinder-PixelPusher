/*
 *  PusherDiscoveryService.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2014 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "cinder/app/AppNative.h"
#include "PusherDiscoveryService.h"
#include "PixelPusher.h"
#include "PusherGroup.h"

double  PusherDiscoveryService::GlobalBrightness       = 1.0;
bool    PusherDiscoveryService::IsGlobalBrightness     = false;
int     PusherDiscoveryService::TotalPower             = 0;
int     PusherDiscoveryService::TotalPowerLimit        = -1;
double  PusherDiscoveryService::PowerScale             = 1.0;
bool    PusherDiscoveryService::IsAutoThrottle         = true;
bool    PusherDiscoveryService::IsAntiLog              = false;
int     PusherDiscoveryService::FrameLimit             = 60;


bool sortPushers( PixelPusherRef a, PixelPusherRef b )
{
    int group_a = a->getGroupId();
    int group_b = b->getGroupId();

    if ( group_a != group_b )
    {
        if ( group_a < group_b )
            return false;
        
        return true;
    }
    
    int ord_a = a->getControllerId();
    int ord_b = b->getControllerId();
    
    if (ord_a != ord_b)
    {
        if (ord_a < ord_b)
            return false;
        
        return true;
    }
    
    return ( a->getMacAddress() < b->getMacAddress() );
}


PusherDiscoveryService::PusherDiscoveryService( boost::asio::io_service& ioService ) : mIoService(ioService)
{
 	mServer = UdpServer::create( mIoService );
    
	// Add callbacks to work with the server asynchronously.
	mServer->connectAcceptEventHandler( &PusherDiscoveryService::onAccept,  this );
	mServer->connectErrorEventHandler(  &PusherDiscoveryService::onError,   this );

    if ( mServer )
    {
        mServer->accept( (uint16_t)PP_DISCOVERY_SERVICE_PORT );
        ci::app::console() << "PusherDiscoveryService Listening on port: " << PP_DISCOVERY_SERVICE_PORT << std::endl;
    }
    else
    {
        ci::app::console() << "PusherDiscoveryService failed to create a server" << std::endl;
    }
    
    // keep groups and pushers lists up to date
    mUpdateGroupsThread = std::thread( &PusherDiscoveryService::updateGroups, this );
}


PusherDiscoveryService::~PusherDiscoveryService()
{
    mRunUpdateGroupsThread = false;
    if ( mUpdateGroupsThread.joinable() )
        mUpdateGroupsThread.join();
}


void PusherDiscoveryService::onAccept( UdpSessionRef session )
{
    // Get the session from the argument and set callbacks.
    mSession = session;
    mSession->connectErrorEventHandler( &PusherDiscoveryService::onError,   this );
    mSession->connectReadEventHandler(  &PusherDiscoveryService::onRead,    this );
    
    // Start reading data from the client.
    mSession->read();
}


void PusherDiscoveryService::onError( std::string err, size_t bytesTransferred )
{
    ci::app::console() << "PusherDiscoveryService socket error!" << std::endl;
}


void PusherDiscoveryService::onRead( ci::Buffer buffer )
{
    mPushersMutex.lock();
    
    uint8_t         *data   =(uint8_t*)buffer.getData();
    DeviceHeader    header  = DeviceHeader( data, buffer.getDataSize() );
    std::string     macAddr = header.getMacAddressString();
    PixelPusherRef  thisDevice, incomingDevice;
    
    // ignore non-PixelPusher devices
    if ( header.getDeviceType() != DeviceType::PIXELPUSHER )
        return;
    
    incomingDevice = PixelPusher::create( header );
    
    for( size_t k=0; k < mPushers.size(); k++ )
    {
        if ( mPushers[k]->getMacAddress() == incomingDevice->getMacAddress() )
        {
            thisDevice = mPushers[k];
            break;
        }
    }

    if ( !thisDevice )
    {
        thisDevice = incomingDevice;
        addNewPusher( thisDevice );
    }
    else
    {
        if ( !thisDevice->isEqual( incomingDevice ) )
        {
            thisDevice->copyHeader( incomingDevice );
        }
        else
        {
            // The device is identical, nothing important has changed
            thisDevice->updateVariables( incomingDevice );
            
            // if we dropped more than occasional packets, slow down a little
            if ( incomingDevice->getDeltaSequence() > 3 )
                thisDevice->increaseExtraDelay(5);
            
            if ( incomingDevice->getDeltaSequence() < 1 )
                thisDevice->decreaseExtraDelay(1);
        }
    }
    
    // Set the timestamp for the last time this device checked in
    thisDevice->setLastPing( ci::app::getElapsedSeconds() );
    
    // update the power limit variables
    if ( TotalPowerLimit > 0 )
    {
        TotalPower = 0;
        
        for( size_t k=0; k < mPushers.size(); k++ )
            TotalPower += mPushers[k]->getPowerTotal();

        PowerScale = ( TotalPower > TotalPowerLimit ) ? ( TotalPowerLimit / TotalPower ) : 1.0;
    }
   
    mPushersMutex.unlock();
    
    // continue reading
	mSession->read();
}

    
void PusherDiscoveryService::addNewPusher( PixelPusherRef pusher )
{
    mPushers.push_back( pusher );
    
    PusherGroupRef group;
    
    for( size_t k=0; k < mGroups.size(); k++ )
    {
        if ( mGroups[k]->getId() == pusher->getGroupId() )
        {
            group = mGroups[k];
            break;
        }
    }
    
    if ( !group )
    {
        group = PusherGroup::create( pusher->getGroupId() );
        mGroups.push_back( group );
    }
    
    if ( !group->hasPusher( pusher ) )
        group->addPusher( pusher );
    
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


std::vector<PixelPusherRef> PusherDiscoveryService::getPushersByIp( std::string ip )
{
    std::vector<PixelPusherRef> pushers;
    
    for( size_t k=0; k < mPushers.size(); k++ )
        if ( mPushers[k]->getIp() == ip )
            pushers.push_back( mPushers[k] );
    
    return pushers;
}


void PusherDiscoveryService::updateGroups()
{
    mRunUpdateGroupsThread = true;
    
    std::vector<PixelPusherRef> pushers;
    PusherGroupRef              group;
    double                      timeNow;
    
    while( mRunUpdateGroupsThread )
    {
        mPushersMutex.lock();
        
        timeNow = ci::app::getElapsedSeconds();
        
        // remove devices
        for( size_t k=0; k < mPushers.size(); )
        {
            if ( !mPushers[k]->isAlive( timeNow ) )
            {
                ci::app::console() << "PusherDiscoveryService remove pusher: " << mPushers[k]->mLastPingAt << " " << timeNow << std::endl;
                
                group = getGroupById( mPushers[k]->getGroupId() );

                if ( group )
                    group->removePusher( mPushers[k] );                 // remove from group
                
                mPushers.erase( mPushers.begin() + k );                 // remove from sorted list
                
            }
            else
                k++;
        }
        
        // remove empty groups
        for( size_t k=0; k < mGroups.size(); )
        {
            if ( mGroups[k]->getNumPushers() == 0 )                     // remove empty group
            {
                mGroups.erase( mGroups.begin() + k );
                
                ci::app::console() << "PusherDiscoveryService remove group" << std::endl;
            }
            else
                k++;
        }
        
        mPushersMutex.unlock();
        
        std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
    }
}

