#include "screentemplate.h"
#include "widgets/batterywidget.h"
#include "widgets/cardwidget.h"
#include "widgets/wirelesswidget.h"
#include "widgets/cardholderwidget.h"
#include "artwork/artwork.h"
#include "widgets/popscreenwidget.h"

class AsyncWebServer;
class AsyncWebServerRequest;

class ScreenTemplate_Settings : public ScreenTemplate
{
public:
  ScreenTemplate_Settings()
  {
    Children.push_back(&wirelessWidget);
    Children.push_back(&popScreenWidget);
    Children.push_back(&batteryWidget);
    Children.push_back(&cardHolder);
    cardHolder.Children.push_back(&privateWifiCard);
    cardHolder.Children.push_back(&publicWifiCard);
    cardHolder.Children.push_back(&updateWifiCard);
    cardHolder.Layout();
  }
  virtual const char* Name(){ return "Settings"; }

private:
  class PrivateWifiWidget : public CardWidget 
  {
  public:
    PrivateWifiWidget();
    virtual void HandleTouch(uint16_t x, uint16_t y);
  };
  class PublicWifiWidget : public CardWidget 
  {
  public:
    PublicWifiWidget();
    virtual void HandleTouch(uint16_t x, uint16_t y);
  };
  class UpdateWidget : public CardWidget 
  {
  public:
    UpdateWidget();
    virtual void HandleTouch(uint16_t x, uint16_t y);
  };

  WirelessWidget wirelessWidget;
  BatteryWidget batteryWidget;
  PrivateWifiWidget privateWifiCard;
  PublicWifiWidget publicWifiCard;
  UpdateWidget updateWifiCard;
  CardHolderWidget cardHolder;
  PopScreenWidget popScreenWidget;

  static AsyncWebServer* webServer;

  typedef void IdleCallback(void);

  static bool ConnectToWifi(std::vector<String>& scannedNetworks);
  static void ErrorAndTouch(const char* error);
  static void RunWebServer(std::vector<String>& ScannedNetworks, std::function<IdleCallback> idleCallback);
  static void TouchToExit();
  static void handleNotFound(AsyncWebServerRequest* request);
};
