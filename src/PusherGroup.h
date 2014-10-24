

class PusherGroup {

public:

  PusherGroup( std::vector<PixelPusherRef> pushers) 
  {
    mPushers = pushers;
  }

  PusherGroup() {}

  std::vector<PixelPusherRef> getPushers() {
    return mPushers;
  }
  
  int size() {
    return mPushers.size();
  }

  std::std::vector<Strip> getStrips() {
    vector<Strip> strips;
    for( size_t k=0; k < mPushers.size(); k++ )
      strips.push_back( mPushers[k]->getStrips() );
    return strips;
  }

  void removePusher( PixelPusherRef pusher ) {
    for( size_t k=0; k < mPushers.size(); k++ )
      if ( mPushers[k] == pusher )
      {
        mPushers.erase( mPushers.begin() + k );
        return;
      }
  }

  void addPusher( PixelPusherRef pusher ) {
    mPushers.push_back(pusher);
  }
  
  private:
    std::vector<PixelPusherRef> mPushers;
};
