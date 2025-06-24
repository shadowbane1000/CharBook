#ifndef __OTA_H__
#define __OTA_H__

#include "src/module.h"

class Ota : public Module
{
  static void DownloadUpdateListTask(void* arg);
public:
  void Initialize();
  virtual Module::UpdateResponse Update();

  enum State {
    // show no icon
    IDLE,
    NO_UPDATE,
    ERROR,
    GETTING_VERSIONS_FILE,

    // show download button
    UPDATE_AVAILABLE,

    // show downloading
    INITIALIZING,
    DOWNLOADING,

    // show flash button
    UPDATE_READY,
  } State;

  void DownloadUpdate();
  void ApplyUpdate();

  const char* GetVersion();
};

extern Ota Ota;

#endif // __OTA_H__