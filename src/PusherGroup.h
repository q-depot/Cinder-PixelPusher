

#pragma once

#include "PixelPusher.h"
#include "Strip.h"


class PusherGroup {

public:

    PusherGroup( std::vector<PixelPusherRef> pushers)
    {
        mPushers = pushers;
    }

    PusherGroup() {}

    ~PusherGroup() {}
    
    std::vector<PixelPusherRef> getPushers() { return mPushers; }
  
    int size() { return mPushers.size(); }

//    std::vector<StripRef> getStrips()
//    {
//        std::vector<StripRef>  strips;
//        std::vector<StripRef>  pusherStrips;
//        
//        for( size_t k=0; k < mPushers.size(); k++ )
//        {
//            pusherStrips = mPushers[k]->getStrips();
//
//            for( size_t i=0; i < pusherStrips.size(); i++ )
//                strips.push_back( pusherStrips[i] );
//        }
//        
//        return strips;
//    }
//    
    void addPusher( PixelPusherRef pusher ) { mPushers.push_back(pusher); }
  
private:
    
    std::vector<PixelPusherRef> mPushers;
    
};
