#ifndef __SCREEN_TEMPLATE_H__
#define __SCREEN_TEMPLATE_H__

class ScreenTemplate
{
public:
  // name as chosen in the SCN file.
  virtual const char* Name() = 0;

  // this is called in the update loop and can do minor updates
  virtual void Update() = 0;

  // this should redraw the whole screen
  virtual void Render() = 0;
};

#endif // __SCREEN_TEMPLATE_H__