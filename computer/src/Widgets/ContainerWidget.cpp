#include "Widgets/ContainerWidget.h"
#include "display.h"

void ContainerWidget::RenderChildren(int16_t parentx, int16_t parenty)
{
  Serial.printf("ContainerWidget::RenderChildren: %d children  (%d,%d)(%dx%d)\n", Children.size(),
    GetX(),GetY(),GetWidth(),GetHeight());
  for(Widget* child : Children)
  {
    child->Render(parentx+GetX(), parenty+GetY());
  }
}
