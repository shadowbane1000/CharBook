#ifndef __SERVICE_H__
#define __SERVICE_H__

class Module {
public:
  enum UpdateResponse {
    SLEEP_OKAY,
    STAY_AWAKE,
    REQUEST_SLEEP
  };

  virtual UpdateResponse Update() = 0;
};

#endif // __SERVICE_H__
