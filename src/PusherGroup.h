

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
  
    size_t getNumPushers() { return mPushers.size(); }

    void addPusher( PixelPusherRef pusher ) { mPushers.push_back(pusher); }
  
private:
    
    std::vector<PixelPusherRef> mPushers;
    
};
