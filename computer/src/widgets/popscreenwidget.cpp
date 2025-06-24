#include "widgets/popscreenwidget.h"
#include "artwork/artwork.h"
#include "display.h"
#include "screen.h"

void PopScreenWidget::Update()
{
}

void PopScreenWidget::Render()
{
  Display.DrawPng(PREV_png, PREV_png_len, x, y);
}

void PopScreenWidget::HandleTouch(uint16_t x, uint16_t y)
{
  Screen.Pop();
}
