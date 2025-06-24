#include "Widgets/TopBarWidget.h"
#include "display.h"

TopBarWidget::TopBarWidget() : popButtonVisible(false)
{
    SetX(0);
    SetY(0);
    SetWidth(Display.Width);
    SetHeight(80);
    
    // Add common widgets
    Children.push_back(&wirelessButton);
    Children.push_back(&batteryWidget);
    Children.push_back(&settingsWidget);

    // Initial layout (without pop button)
    Layout();
}

void TopBarWidget::Layout()
{
    // Position widgets from left to right, or right to left as needed
    // Example positioning (adjust x, y coordinates as needed)
    int16_t currentX = 0;

    if (popButtonVisible) {
        popScreenWidget.SetX(currentX);
        popScreenWidget.SetY(0);
        currentX += popScreenWidget.GetWidth() + 2; // Add some spacing
    }

    wirelessButton.SetX(currentX);
    wirelessButton.SetY(0);
    currentX += wirelessButton.GetWidth() + 2;

    // Position battery and settings from the right edge
    int16_t rightX = Display.Width;
    settingsWidget.SetX(rightX - settingsWidget.GetWidth());
    settingsWidget.SetY(0);
    rightX -= (settingsWidget.GetWidth() + 2);

    batteryWidget.SetX(rightX - batteryWidget.GetWidth());
    batteryWidget.SetY(0);

    ContainerWidget::Layout(); // Call base layout if needed
}


void TopBarWidget::ShowPopButton()
{
    if (!popButtonVisible) {
        // Check if it's already there to avoid duplicates (optional, depends on Remove implementation)
        bool found = false;
        for(const auto& child : Children) {
            if (child == &popScreenWidget) {
                found = true;
                break;
            }
        }
        if (!found) {
             // Insert at the beginning for left alignment
            Children.insert(Children.begin(), &popScreenWidget);
        }
        popButtonVisible = true;
        Layout(); // Re-layout with the pop button
    }
}

void TopBarWidget::HidePopButton()
{
    if (popButtonVisible) {
        // Find and remove the popScreenWidget
        for (auto it = Children.begin(); it != Children.end(); ++it) {
            if (*it == &popScreenWidget) {
                Children.erase(it);
                break;
            }
        }
        popButtonVisible = false;
        Layout(); // Re-layout without the pop button
    }
}
