#ifndef __WIRELESS_H__
#define __WIRELESS_H__

#include "src/module.h"
#include "screentemplate.h"

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

class WirelessButton : public FixedSizeWidget
{
  void SelectImage(unsigned char** image, unsigned int* size);
  unsigned char* currentImage;
  unsigned int currentImageSize;
public:
  WirelessButton();
  virtual uint16_t GetWidth() { return 80; }
  virtual uint16_t GetHeight() { return 80; }

  virtual void Update();
  virtual void Render();
  virtual void HandleTouch(uint16_t x, uint16_t y);
};


extern Wireless Wireless;

#endif // __WIRELESS_H__