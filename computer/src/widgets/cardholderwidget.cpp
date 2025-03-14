#include "widgets/cardholderwidget.h"

void CardHolderWidget::Layout()
{
  const int width = 4;
  const int height = 2;
  const int spacing = 5;
  int item_x = 0;
  int item_y = 0;
  for(auto child : Children)
  {
    child->SetX(spacing + x + item_x*(200+spacing));
    child->SetY(spacing + y + item_y*(200+spacing));

    item_x++;
    if(item_x >= width) item_x = 0, item_y++;
  }
}
