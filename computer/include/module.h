#ifndef __MODULE_H__
#define __MODULE_H__

class Module {
public:
  enum UpdateResponse {
    SLEEP_OKAY,
    STAY_AWAKE,
    REQUEST_SLEEP
  };

  virtual UpdateResponse Update() = 0;
};

#endif // __MODULE_H__
