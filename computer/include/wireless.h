#ifndef __WIRELESS_H__
#define __WIRELESS_H__

#include <vector>

#include "module.h"

// maybe see munet package

class Wireless : public Module
{
public:
  void Initialize();
  virtual Module::UpdateResponse Update();
  
  // WIFI
  enum State {
    WIFI_INITIALIZE = 0,
    WIFI_SCANNING,
    WIFI_UNAVAILABLE,
    WIFI_AVAILABLE,
    WIFI_CONNECTING,
    WIFI_CONNECTED
  } State;

  bool WifiPrivate();
  bool WifiConnect(const char* networkName);
  void WifiDisconnect();
  bool WifiScan(std::vector<String>& ScannedNetworks);
  String GetIPAddress();

  // states
  UpdateResponse StateStartup();
  UpdateResponse StateUnavailable();
  UpdateResponse StateAvailable();
  UpdateResponse StateScanning();
  UpdateResponse StateConnecting();
  UpdateResponse StateConnected();

  String currentSSID;
  String currentPassword;
private:
  bool checkedConfig;
};

extern class Wireless Wireless;

#endif // __WIRELESS_H__