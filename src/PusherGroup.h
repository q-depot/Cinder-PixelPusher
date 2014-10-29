/*
 *  PusherGroup.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2014 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef PP_GROUP
#define PP_GROUP

#pragma once

#include "PixelPusher.h"
#include "Strip.h"

class PusherGroup;
typedef std::shared_ptr<PusherGroup>    PusherGroupRef;


class PusherGroup {

public:
    
    static PusherGroupRef create( uint32_t groupId )
    {
        return PusherGroupRef( new PusherGroup(groupId) );
    }
    
    ~PusherGroup() {}
    
    uint32_t getId() { return mId; }
    
    std::vector<PixelPusherRef> getPushers() { return mPushers; }
    
    size_t getNumPushers() { return mPushers.size(); }

    bool hasPusher( PixelPusherRef pusher )
    {
        for( size_t k=0; k < mPushers.size(); k++ )
            if ( mPushers[k] == pusher )
                return true;
        
        return false;
    }
    
    void addPusher( PixelPusherRef pusher ) { mPushers.push_back(pusher); }
    
    void removePusher( PixelPusherRef pusher )
    {
        for( size_t k=0; k < mPushers.size(); k++ )
            if ( mPushers[k] == pusher )
            {
                mPushers.erase( mPushers.begin() + k );
                return;
            }
    }
    
private:
    
    PusherGroup( uint32_t groupId ) : mId(groupId) {}
    
private:
    
    std::vector<PixelPusherRef>     mPushers;
    uint32_t                        mId;
    
};

#endif