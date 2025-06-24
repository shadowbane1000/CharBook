#ifndef __SCREEN_TEMPLATE_SETTINGS_H__
#define __SCREEN_TEMPLATE_SETTINGS_H__

#include "ScreenTemplate.h"
#include "Widgets/TileWidget.h"
#include "Widgets/TileHolderWidget.h"
#include "Widgets/ImageWidget.h"
#include "artwork/artwork.h"
#include "Widgets/TopBarWidget.h"

class AsyncWebServer;
class AsyncWebServerRequest;

class ScreenTemplate_Settings : public ScreenTemplate
{
public:
  ScreenTemplate_Settings() :
    ScreenTemplate(),
    privateWifiTile(PRIVWIFI_png, PRIVWIFI_png_len),
    publicWifiTile(PUBWIFI_png, PUBWIFI_png_len),
    updateWifiTile(UPDATE_png, UPDATE_png_len)
  {
    Children.push_back(&topBarWidget);
    Children.push_back(&tileHolder);
    tileHolder.AddTile(&privateWifiTile, 0, 0);
    tileHolder.AddTile(&publicWifiTile, 1, 0);
    tileHolder.AddTile(&updateWifiTile, 2, 0);
  }
  virtual const char* Name(){ return "Settings"; }
  virtual void Initialize(JsonDocument& screen);
  virtual void Deinitialize(){}

private:
  class PrivateWifiWidget : public TileWidget
  {
  public:
    PrivateWifiWidget(uint8_t* imageData, uint16_t imageSize);
    virtual bool HandleTouch(uint16_t x, uint16_t y) override;
  private:
    ImageWidget* m_imageWidget;
  };
  class PublicWifiWidget : public TileWidget
  {
  public:
    PublicWifiWidget(uint8_t* imageData, uint16_t imageSize);
    virtual bool HandleTouch(uint16_t x, uint16_t y) override;
  private:
    ImageWidget* m_imageWidget;
  };
  class UpdateWidget : public TileWidget
  {
  public:
    UpdateWidget(uint8_t* imageData, uint16_t imageSize);
    virtual bool HandleTouch(uint16_t x, uint16_t y) override;
  private:
    ImageWidget* m_imageWidget;
  };

  TopBarWidget topBarWidget;
  PrivateWifiWidget privateWifiTile;
  PublicWifiWidget publicWifiTile;
  UpdateWidget updateWifiTile;
  TileHolderWidget tileHolder;

  static AsyncWebServer* webServer;

  typedef void IdleCallback(void);

  static bool ConnectToWifi(std::vector<String>& scannedNetworks);
  static void ErrorAndTouch(const char* error);
  static void RunWebServer(std::vector<String>& ScannedNetworks, std::function<IdleCallback> idleCallback);
  static void TouchToExit();
  static void handleNotFound(AsyncWebServerRequest* request);
  static void ApplyArchive(const char* updateTarget);
};

#endif // __SCREEN_TEMPLATE_SETTINGS_H__
