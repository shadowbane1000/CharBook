#include "screen.h"
#include "Widgets/PopScreenWidget.h"

void PopScreenWidget::Update()
{
}

void PopScreenWidget::Render(int16_t parentx, int16_t parenty)
{
}

bool PopScreenWidget::HandleTouch(uint16_t x, uint16_t y)
{
    Screen.Pop();
    return true;
}

