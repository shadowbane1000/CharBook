#ifndef __NFC_H__
#define __NFC_H__

#include "service.h"

class Nfc : public Module
{
public:
  void Initialize();
  virtual Module::UpdateResponse Update();
};

extern Nfc Nfc;

#endif // __NFC_H__
