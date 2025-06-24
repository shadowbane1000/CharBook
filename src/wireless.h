#ifndef __WIRELESS_H__
#define __WIRELESS_H__

#include "service.h"

// maybe see munet package


class Wireless : public Module
{
public:
  void Initialize();
  virtual Module::UpdateResponse Update();
  
  // WIFI
  enum State {
    WIFI_UNAVAILABLE = 0,
    WIFI_AVAILABLE = 1,
    WIFI_CONNECTING = 2,
    WIFI_CONNECTED = 3
  } State;

  void WifiConnect();
  void WifiDisconnect();

private:
  bool checkedConfig;

};

extern Wireless Wireless;

#endif // __WIRELESS_H__