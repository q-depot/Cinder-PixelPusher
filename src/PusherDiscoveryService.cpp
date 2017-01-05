/*
 *  PusherDiscoveryService.cpp
 *
 *  Created by Andrea Cuius
 *  The MIT License (MIT)
 *  Copyright (c) 2015 Nocte Studio Ltd.
 *
 *  www.nocte.co.uk
 *
 */

//#include "cinder/app/AppNative.h"
#include "PusherDiscoveryService.h"
#include "PixelPusher.h"
#include "PusherGroup.h"

double  PusherDiscoveryService::GlobalBrightness       = 1.0;
int     PusherDiscoveryService::TotalPower             = 0;
int     PusherDiscoveryService::TotalPowerLimit        = -1;
double  PusherDiscoveryService::PowerScale             = 1.0;
bool    PusherDiscoveryService::IsAutoThrottle         = true;
bool    PusherDiscoveryService::IsColorCorrection      = false;
int     PusherDiscoveryService::FrameLimit             = 60;

using namespace ci;
using namespace ci::app;
using namespace std;


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


PusherDiscoveryService::PusherDiscoveryService( asio::io_service& ioService ) : mIoService(ioService)
{
 	mServer = UdpServer::create( mIoService );
    
	// Add callbacks to work with the server asynchronously.
	mServer->connectAcceptEventHandler( &PusherDiscoveryService::onAccept,  this );
	mServer->connectErrorEventHandler(  &PusherDiscoveryService::onError,   this );

    if ( mServer )
    {
        mServer->accept( (uint16_t)PP_DISCOVERY_SERVICE_PORT );
        console() << "PusherDiscoveryService Listening on port: " << PP_DISCOVERY_SERVICE_PORT << endl;
    }
    else
    {
        console() << "PusherDiscoveryService failed to create a server" << endl;
    }
    
    // keep groups and pushers lists up to date
    mUpdateGroupsThread = thread( &PusherDiscoveryService::updateGroups, this );
}


PusherDiscoveryService::~PusherDiscoveryService()
{
    shutdown();
}


void PusherDiscoveryService::shutdown()
{
    mRunUpdateGroupsThread = false;
    if ( mUpdateGroupsThread.joinable() )
        mUpdateGroupsThread.join();
    
    console() << "PusherDiscoveryService::shutdown()" << endl;
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


void PusherDiscoveryService::onError( string err, size_t bytesTransferred )
{
    console() << "PusherDiscoveryService socket error!" << endl;
}


void PusherDiscoveryService::onRead( BufferRef buffer )
{
//    mPushersMutex.lock();
    
    uint8_t         *data   =(uint8_t*)buffer->getData();
    DeviceHeader    header  = DeviceHeader( data, buffer->getSize() );
    string          macAddr = header.getMacAddressString();
    PixelPusherRef  thisDevice, incomingDevice;
    
    // ignore non-PixelPusher devices
    if ( header.getDeviceType() != DeviceType::PIXELPUSHER )
        return;
    
    incomingDevice = PixelPusher::create( header );
    
    for( size_t k=0; k < mPushersInternal.size(); k++ )
    {
        if ( !mPushersInternal[k] )
            continue;

        if ( mPushersInternal[k]->getMacAddress() == incomingDevice->getMacAddress() )
        {
            thisDevice = mPushersInternal[k];
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
    thisDevice->setLastPing( getElapsedSeconds() );
    
    // update the power limit variables
    if ( TotalPowerLimit > 0 )
    {
        TotalPower = 0;
        
        for( size_t k=0; k < mPushersInternal.size(); k++ )
            TotalPower += mPushersInternal[k]->getPowerTotal();

        PowerScale = ( TotalPower > TotalPowerLimit ) ? ( TotalPowerLimit / TotalPower ) : 1.0;
    }
   
//    mPushersMutex.unlock();
    
    // continue reading
	mSession->read();
}

    
void PusherDiscoveryService::addNewPusher( PixelPusherRef pusher )
{
    mPushersInternal.push_back( pusher );
    
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
        
        vector<PixelPusherRef> members = getPushersByIp( pusher->getIp() );
        
        bool groupHasPrimary = false;
        
        for( size_t k=0; k < members.size(); k++ )
            if ( members[k]->isMulticastPrimary() )
                groupHasPrimary = true;
        
        if ( !groupHasPrimary )
            pusher->setMulticastPrimary(true);
    }
    
    pusher->createCardThread( mIoService );
}


void PusherDiscoveryService::removePusher( size_t idx )
{
    if ( idx >= mPushersInternal.size() )
        return;

    console() << "PusherDiscoveryService remove pusher: " << mPushersInternal[idx]->mLastPingAt << " " << getElapsedSeconds() << endl;
     
    PusherGroupRef group = getGroupById( mPushersInternal[idx]->getGroupId() );

    if ( group )
        group->removePusher( mPushersInternal[idx] );                 // remove from group
                
    mPushersInternal.erase( mPushersInternal.begin() + idx );                 // remove from sorted list                
}


vector<PixelPusherRef> PusherDiscoveryService::getPushersByIp( string ip )
{
    vector<PixelPusherRef> pushers;
    
    for( size_t k=0; k < mPushersPublic.size(); k++ )
        if ( mPushersPublic[k]->getIp() == ip )
            pushers.push_back( mPushersPublic[k] );
    
    return pushers;
}


void PusherDiscoveryService::updateGroups()
{
    mRunUpdateGroupsThread = true;
    
    vector<PixelPusherRef>  pushers;
    double                  timeNow;
    
    while( mRunUpdateGroupsThread )
    {        
        timeNow = getElapsedSeconds();
        
        // remove devices
        for( size_t k=0; k < mPushersInternal.size(); )
        {
            if ( !mPushersInternal[k]->isAlive( timeNow ) )
                removePusher(k);
            else
                k++;
        }
        
        // remove empty groups
        for( size_t k=0; k < mGroups.size(); )
        {
            if ( mGroups[k]->getNumPushers() == 0 )                     // remove empty group
            {
                mGroups.erase( mGroups.begin() + k );
                
                console() << "PusherDiscoveryService remove group" << endl;
            }
            else
                k++;
        }

        mDataMutex.lock();
        mPushersPublic = mPushersInternal;
        mDataMutex.unlock();

        this_thread::sleep_for( chrono::milliseconds( 1000 ) );
    }
}

