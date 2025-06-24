#include <Arduino.h>
#include <epd_driver.h>
#include <PNGdec.h>

#include "display.h"
#include "image.h"

#include "firasans.h"

const uint8_t color_unpack_2_to_4[4] = { 0, 2, 4, 7 };

#define UPDATE_DELAY 500
#define DONT_UPDATE (0xFFFFFFFFu)

typedef struct my_private_struct
{
  int xoff, yoff; // corner offset
} PRIVATE;
PNG thePng;
PNG* png = &thePng;

unsigned char gamma_lut[256];
void generateGammaTable()
{
    for (int i = 0; i < 256; i++) {
        // Normalize 8-bit value to 0.0 - 1.0
        double normalized_value = i / 255.0;
        // Apply gamma correction (1 / 1.9)
        double corrected_value = pow(normalized_value, 1.0 / 1.9);
        // Scale back to 0-255 and round
        gamma_lut[i] = (uint8_t)(corrected_value * 255);
    }
}

static uint8_t Convert8BitTo4Bit(uint8_t value)
{
  uint16_t newvalue = value + random() % 16;
  if (newvalue > 255) {
    newvalue = 255;
  }
  return newvalue >> 4;
}

static void GetPngPixels(PNGDRAW *pDraw, uint8_t *pPixels)
{
  int x, j;
  uint16_t usPixel;
  uint8_t ubPixel, *pDest = pPixels;
  uint8_t c=0, a, *pPal, *s = pDraw->pPixels;
  
  switch (pDraw->iPixelType) {
  case PNG_PIXEL_GRAY_ALPHA:
    for (x=0; x<pDraw->iWidth; x++) {
      c = *s++; // gray level
      a = *s++;
      ubPixel = Convert8BitTo4Bit(gamma_lut[c]);
      if(x % 2 == 1) {
        *pDest = *pDest | ubPixel;
        pDest++;
      } else {
        *pDest = ubPixel << 4;
      }
    }
    break;
  case PNG_PIXEL_GRAYSCALE:
    switch (pDraw->iBpp) {
      case 8:
      for (x=0; x<pDraw->iWidth; x++) {
        c = *s++;
        ubPixel = Convert8BitTo4Bit(gamma_lut[c]);
        if(x % 2 == 1) {
          *pDest = *pDest | ubPixel;
          pDest++;
        } else {
          *pDest = ubPixel << 4;
        }
      }
      break;
      case 1:
        for (x=0; x<pDraw->iWidth; x++) {
          if ((x & 7) == 0) {
              c = *s++;
          }
          if (c & 0x80) {
              ubPixel = 0xf;
          } else {
              ubPixel = 0;
          }
          if(x % 2 == 1) {
            *pDest = *pDest | ubPixel;
            pDest++;
          } else {
            *pDest = ubPixel << 4;
          }
          c <<= 1;
        }
      break;
    } // switch on bpp
    break;
  }
}

void PNGDraw(PNGDRAW *pDraw)
{
  uint8_t* ucMask = (uint8_t*)alloca((pDraw->iWidth + 7) / 8);
  PRIVATE *pPriv = (PRIVATE *)pDraw->pUser;
  assert(pDraw->iPixelType != PNG_PIXEL_TRUECOLOR);
  assert(pDraw->iPixelType != PNG_PIXEL_TRUECOLOR_ALPHA);
  assert(pDraw->iPixelType != PNG_PIXEL_INDEXED);

  if (png->getAlphaMask(pDraw, ucMask, 255)) {
    uint8_t buffer[pDraw->iWidth/2];
    
    GetPngPixels(pDraw, buffer);

    //Serial.printf("Draw w=%d alpha=%d pitch=%d bpp=%d type=%d\r\n", pDraw->iWidth, pDraw->iHasAlpha, pDraw->iPitch, pDraw->iBpp, pDraw->iPixelType);
    for(int x = 0; x < pDraw->iWidth; x++)
    {
      // one bit per pixel mask
      if(x > 0 && x % 8 == 0)
        ucMask++;
      if(*ucMask & 0x01)
      {
        if(x % 2 == 0)
          Display.SetPixel(pPriv->xoff + x, pPriv->yoff + pDraw->y, buffer[x/2] >> 4);
        else
          Display.SetPixel(pPriv->xoff + x, pPriv->yoff + pDraw->y, buffer[x/2] & 0xF);
      }
      *ucMask >>= 1;
    }
  }
}

void Display::Initialize()
{
  png = new(ps_malloc(sizeof(PNG))) PNG();

  Width = EPD_HEIGHT;
  Height = EPD_WIDTH;
  framebuffer.width = EPD_HEIGHT;
  framebuffer.height = EPD_WIDTH;
  devicebuffer.width = EPD_WIDTH;
  devicebuffer.height = EPD_HEIGHT;
  framebuffer.data = (uint8_t*)ps_malloc(framebuffer.width * framebuffer.height / 2);
  devicebuffer.data = (uint8_t*)ps_malloc(devicebuffer.width * devicebuffer.height / 2);
  generateGammaTable();
  epd_init();
  Clear(0xF);
  Flush();
//  CursorX = 0;
//  CursorY = 100;
//  DrawText("Display Initialized");
}

void Display::Flush()
{
  Region source = {
    .x = 0,
    .y = 0,
    .width = framebuffer.width,
    .height = framebuffer.height
  };
  Region destination = {
    .x = 0,
    .y = 0,
    .width = devicebuffer.width,
    .height = devicebuffer.height
  };
  BitBlt(source, destination, framebuffer, devicebuffer, Rotation::ROTATE_MINUS90, NO_TRANSPARENCY);
  epd_poweron();
  epd_clear();
  volatile uint32_t t1 = millis();
  epd_draw_grayscale_image(epd_full_screen(), devicebuffer.data);
  volatile uint32_t t2 = millis();
  Serial.printf("EPD draw took %dms.\r\n", t2 - t1);
  epd_poweroff();

  invalidArea.x = EPD_WIDTH;
  invalidArea.y = EPD_HEIGHT;
  invalidArea.width = -EPD_WIDTH;
  invalidArea.height = -EPD_HEIGHT;
  nextUpdateMillis = DONT_UPDATE;
}

void Display::ClearImmediate(uint8_t color)
{
  epd_poweron();
  epd_clear();
  epd_poweroff();
}

void Display::Clear(uint8_t color)
{
  uint8_t fourBits = color & 0x0F;
  memset(framebuffer.data, fourBits | (fourBits << 4), EPD_WIDTH * EPD_HEIGHT / 2);
  nextUpdateMillis = millis()+UPDATE_DELAY;
}

void Display::DrawText(const char* text)
{
  write_string((GFXfont *)&FiraSans, text, &CursorX, &CursorY, framebuffer.data, framebuffer.width, framebuffer.height, NULL);
  nextUpdateMillis = millis()+UPDATE_DELAY;
}

void Display::DrawTextImmediate(const char* text)
{
  write_string((GFXfont *)&FiraSans, text, &CursorX, &CursorY, NULL, EPD_WIDTH, EPD_HEIGHT, NULL);
}

void Display::DrawPng(const unsigned char* pngData, const unsigned int length, int x, int y)
{
  int rc = png->openRAM((uint8_t*)pngData, length, PNGDraw);
  if (rc == PNG_SUCCESS) {
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png->getWidth(), png->getHeight(), png->getBpp(), png->getPixelType());
    Serial.printf("  drawing at %d,%d\r\n", x, y);
    PRIVATE priv;
    priv.xoff = x + OffsetX;
    priv.yoff = y + OffsetY;
    rc = png->decode((void *)&priv, 0);
    Region region;
    region.x = x + OffsetX;
    region.y = y + OffsetY;
    region.width = (uint16_t)png->getWidth();
    region.height = (uint16_t)png->getHeight();
    InvalidateRegion(region);
    png->close();
  }
}

void Display::DrawImage(Image* image, int x, int y)
{
  Rect_t area = {
      .x = x,
      .y = y,
      .width = image->width,
      .height = image->height
  };
  //epd_copy_to_framebuffer(area, image->data, framebuffer);
  DrawPng(image->data, image->size, x + OffsetX, y + OffsetY);
  nextUpdateMillis = millis()+UPDATE_DELAY;
}

void Display::InvalidateAll()
{
  invalidArea.x = 0;
  invalidArea.y = 0;
  invalidArea.width = EPD_WIDTH;
  invalidArea.height = EPD_HEIGHT;
  nextUpdateMillis = millis()+UPDATE_DELAY;
}

void Display::InvalidateRegion(const Region& region)
{
  uint16_t x1 = invalidArea.x;
  uint16_t y1 = invalidArea.y;
  uint16_t x2 = invalidArea.x + invalidArea.width;
  uint16_t y2 = invalidArea.y + invalidArea.height;

  x1 = x1 < region.x ? x1 : region.x;
  y1 = y1 < region.y ? y1 : region.y;
  x2 = x2 < (region.x + region.width) ? x2 : (region.x + region.width);
  y2 = y2 < (region.y + region.height) ? y2 : (region.y + region.height);
  
  invalidArea.x = x1;
  invalidArea.y = y1;
  invalidArea.width = x2 - x1;
  invalidArea.height = y2 - y1;
  nextUpdateMillis = millis()+UPDATE_DELAY;
}

void Display::SetPixel(int x, int y, uint8_t color)
{
  //Serial.printf("SetPixel(%d, %d, %x)\r\n", x, y, color);
  if (x < 0 || x >= framebuffer.width)
  {
    return;
  }
  if (y < 0 || y >= framebuffer.height)
  {
    return;
  }
  uint8_t *buf_ptr = &framebuffer.data[(y * framebuffer.width + x) / 2];
  if (x % 2)
  {
    *buf_ptr = (*buf_ptr & 0x0F) | (color << 4);
  }
  else
  {
    *buf_ptr = (*buf_ptr & 0xF0) | (color);
  }
}

Module::UpdateResponse Display::Update()
{
  if(nextUpdateMillis != DONT_UPDATE){
    if(millis() > nextUpdateMillis){
      Flush();
    }
  }

  return SLEEP_OKAY;
}

void Display::DrawText(const Region& area, Justification just, Alignment align, String text, bool transparent_background) 
{
  FontProperties props;
  props.fg_color = 0;
  props.bg_color = 15;
  props.fallback_glyph = 0;
  props.flags = 0;//DRAW_BACKGROUND;
//  if (transparent_background) {
//      props.flags |= TRANSPARENT_BACKGROUND;
//  }

  // Variables to store text metrics
  int32_t x1, y1, w, h;
  int32_t line_height = 0;
  
  // Get the line height by measuring a sample character
  int32_t temp_x = 0, temp_y = 0;
  get_text_bounds((GFXfont *)&FiraSans, "M", &temp_x, &temp_y, &x1, &y1, &w, &h, &props);
  line_height = h * 1.2; // Add 20% spacing between lines
  
  // Extract text as C string for processing
  const char* str = text.c_str();
  int str_len = text.length();
  
  // Count the number of lines required
  int line_count = 0;
  int last_space = -1;
  int line_start = 0;
  int i = 0;
  int current_width = 0;
  
  // First pass: count lines to calculate vertical positioning
  while (i < str_len) {
      // Find the next space or end of string
      if (str[i] == ' ' || i == str_len - 1) {
          // Measure this word
          int32_t word_x = 0, word_y = 0;
          int word_end = (i == str_len - 1) ? i + 1 : i;
          char word_buf[256] = {0};
          memcpy(word_buf, &str[line_start], word_end - line_start);
          word_buf[word_end - line_start] = '\0';
          
          get_text_bounds((GFXfont *)&FiraSans, word_buf, &word_x, &word_y, &x1, &y1, &w, &h, &props);
          
          // Check if adding this word would exceed the width
          if (current_width + w > area.width && last_space != -1) {
              // Start new line
              line_count++;
              line_start = last_space + 1;
              current_width = 0;
              last_space = -1;
              // Reprocess current word
              i = line_start;
              continue;
          }
          
          current_width = w;
          last_space = i;
      }
      i++;
  }
  
  // Add the final line
  line_count++;
  
  // Calculate starting Y position based on alignment
  int32_t start_y;
  switch (align) {
      case ALIGN_TOP:
          start_y = area.y + line_height + OffsetY;
          break;
      case ALIGN_BOTTOM:
          start_y = area.y + area.height - (line_count - 1) * line_height + OffsetY;
          break;
      case ALIGN_CENTER:
          start_y = area.y + (area.height - line_count * line_height) / 2 + line_height + OffsetY;
          break;
      default:
          start_y = area.y + line_height + OffsetY;
  }
  
  // Reset for second pass
  line_start = 0;
  last_space = -1;
  i = 0;
  current_width = 0;
  int32_t cursor_x, cursor_y;
  cursor_y = start_y;
  
  // Create an array to hold line segments
  char line_buf[256] = {0};
  int line_pos = 0;
  
  // Second pass: render each line
  while (i < str_len) {
      if (str[i] == ' ' || i == str_len - 1) {
          // Get this word
          int word_end = (i == str_len - 1) ? i + 1 : i;
          char word_buf[256] = {0};
          memcpy(word_buf, &str[line_start], word_end - line_start);
          word_buf[word_end - line_start] = '\0';
          
          // Truncate word from the right if it's too long to fit the area width
          int32_t word_w_val, word_h_val, word_x1_val, word_y1_val;
          // Use local temp_x, temp_y for get_text_bounds for word_buf
          int32_t local_temp_x = 0, local_temp_y = 0; 
          get_text_bounds((GFXfont *)&FiraSans, word_buf, &local_temp_x, &local_temp_y, &word_x1_val, &word_y1_val, &word_w_val, &word_h_val, &props);

          while (word_w_val > area.width && strlen(word_buf) > 0) {
              word_buf[strlen(word_buf) - 1] = '\0'; // Remove last character
              if (strlen(word_buf) == 0) {
                  word_w_val = 0; // Word is now empty
                  break;
              }
              // Recalculate bounds for the truncated word
              get_text_bounds((GFXfont *)&FiraSans, word_buf, &local_temp_x, &local_temp_y, &word_x1_val, &word_y1_val, &word_w_val, &word_h_val, &props);
          }
          // Now word_buf is potentially truncated from the right.

          // Measure the current line with this (potentially truncated) word
          // temp_x, temp_y are used for the global get_text_bounds call for test_buf
          temp_x = 0; temp_y = 0; 
          
          // Create test buffer with current line + new word
          char test_buf[512] = {0};
          strcpy(test_buf, line_buf);
          if (strlen(word_buf) > 0) { // Only add word and space if word is not empty
            if (line_pos > 0) { // If line_buf is not empty, add a space before the new word
                strcat(test_buf, " ");
            }
            strcat(test_buf, word_buf);
          } else if (line_pos == 0) { // If line_buf is empty and word_buf is empty, test_buf is effectively empty
            // test_buf is already an empty string, w will be 0.
          }
          // If word_buf is empty and line_buf is not, test_buf is just line_buf.
          
          // Use global x1, y1, w, h for the width of test_buf
          get_text_bounds((GFXfont *)&FiraSans, test_buf, &temp_x, &temp_y, &x1, &y1, &w, &h, &props); 
          
          // Check if adding this word would exceed the width
          if (w > area.width && line_pos > 0) { // If test_buf is too wide, and line_buf was not empty
              // The line to be rendered is line_buf. We need its actual width for justification.
              // Recalculate w, h, x1, y1 using line_buf. temp_x, temp_y are dummy inputs here.
              get_text_bounds((GFXfont *)&FiraSans, line_buf, &temp_x, &temp_y, &x1, &y1, &w, &h, &props); // Now, w is the correct width of line_buf
              
              // Set cursor_x based on justification
              switch (just) {
                  case JUSTIFY_LEFT:
                      cursor_x = area.x + OffsetX;
                      break;
                  case JUSTIFY_RIGHT:
                      cursor_x = area.x + area.width - w + OffsetX; // Uses updated w
                      break;
                  case JUSTIFY_CENTER:
                      cursor_x = area.x + (area.width - w) / 2 + OffsetX; // Uses updated w
                      break;
                  default:
                      cursor_x = area.x + OffsetX;
              }
              
              // Render current line
              Serial.printf("Drawing line: %s @ %d %d\n", line_buf, cursor_x, cursor_y);
              writeln((GFXfont *)&FiraSans, line_buf, &cursor_x, &cursor_y, framebuffer.data, framebuffer.width, framebuffer.height, &props);
              
              // Reset line buffer
              memset(line_buf, 0, sizeof(line_buf));
              // Start new line with the current word_buf (if it's not empty after truncation)
              if (strlen(word_buf) > 0) {
                strcpy(line_buf, word_buf);
                line_pos = strlen(word_buf);
              } else {
                line_pos = 0; // Word was truncated to empty or was originally empty
              }
              
              // Move cursor to next line
              cursor_y += line_height;
          } else { // test_buf fits, or line_buf was empty
              // Add word to current line (if word_buf is not empty)
              if (strlen(word_buf) > 0) {
                  if (line_pos > 0) { // If line_buf is not empty, add a space
                      strcat(line_buf, " ");
                      line_pos++; // For the space
                  }
                  strcat(line_buf, word_buf);
                  line_pos += strlen(word_buf); // For the word
              }
              // If word_buf is empty, line_buf and line_pos remain unchanged (or are empty if it's the start)
          }
          
          line_start = i + 1;
      }
      i++;
  }
  
  // Render the last line if not empty
  if (line_pos > 0) {
      int32_t temp_x = 0, temp_y = 0;
      get_text_bounds((GFXfont *)&FiraSans, line_buf, &temp_x, &temp_y, &x1, &y1, &w, &h, &props);
      
      // Set cursor_x based on justification
      switch (just) {
          case JUSTIFY_LEFT:
              cursor_x = area.x + OffsetX;
              break;
          case JUSTIFY_RIGHT:
              cursor_x = area.x + area.width - w + OffsetX;
              break;
          case JUSTIFY_CENTER:
              cursor_x = area.x + (area.width - w) / 2 + OffsetX;
              break;
          default:
              cursor_x = area.x + OffsetX;
      }
      
      Serial.printf("Drawing last line: %s at %d %d\n", line_buf, cursor_x, cursor_y);
      writeln((GFXfont *)&FiraSans, line_buf, &cursor_x, &cursor_y, framebuffer.data, framebuffer.width, framebuffer.height, &props);
  }
}

void Display::BitBlt(
  const Region& sourceRegion, 
  const Region& targetRegion, 
  const Buffer& sourceBuffer, 
  const Buffer& destinationBuffer, 
  Rotation rotation, 
  uint8_t transparentColor) 
{
  int pixelCnt = 0;
  if (sourceBuffer.data == nullptr || destinationBuffer.data == nullptr) 
  {
    return; // Handle null buffer pointers
  }

  for (int16_t destY = 0; destY < targetRegion.height; ++destY) 
  {
    for (int16_t destX = 0; destX < targetRegion.width; ++destX) 
    {
      int16_t srcX, srcY;

      // Calculate corresponding source coordinates based on rotation and scaling (simple)
      switch (rotation) 
      {
      case NO_ROTATION:
        srcX = sourceRegion.x + (int16_t)(((float)destX / targetRegion.width) * sourceRegion.width);
        srcY = sourceRegion.y + (int16_t)(((float)destY / targetRegion.height) * sourceRegion.height);
        break;
      case ROTATE_PLUS90:
        srcX = sourceRegion.x + (int16_t)(((float)destY / targetRegion.height) * sourceRegion.width);
        srcY = sourceRegion.y + sourceRegion.height - 1 - (int16_t)(((float)destX / targetRegion.width) * sourceRegion.height);
        break;
      case ROTATE_MINUS90:
        srcX = sourceRegion.x + sourceRegion.width - 1 - (int16_t)(((float)destY / targetRegion.height) * sourceRegion.width);
        srcY = sourceRegion.y + (int16_t)(((float)destX / targetRegion.width) * sourceRegion.height);
        break;
      case ROTATE_180:
        srcX = sourceRegion.x + sourceRegion.width - 1 - (int16_t)(((float)destX / targetRegion.width) * sourceRegion.width);
        srcY = sourceRegion.y + sourceRegion.height - 1 - (int16_t)(((float)destY / targetRegion.height) * sourceRegion.height);
        break;
      default:
        return; // Invalid rotation
      }

      // Check if source coordinates are within bounds
      if (srcX < 0 || srcX >= sourceBuffer.width || srcY < 0 || srcY >= sourceBuffer.height) {
        continue;
      }

      // Calculate byte and bit positions in the source buffer (4bpp)
      int32_t srcByteIndex = ((int32_t)(srcY * sourceBuffer.width + srcX)) / 2;
      uint8_t srcPixelValue;
      if ((srcX % 2) == 0) 
      { // Low nibble
        srcPixelValue = sourceBuffer.data[srcByteIndex] & 0x0F;
      } 
      else 
      { // High nibble
        srcPixelValue = (sourceBuffer.data[srcByteIndex] >> 4) & 0x0F;
      }

      // Transparency check
      if (srcPixelValue <= 15 && srcPixelValue == transparentColor) 
      {
        continue; // Skip transparent pixel
      }

      // Calculate target coordinates in the destination buffer
      int16_t finalDestX = targetRegion.x + destX;
      int16_t finalDestY = targetRegion.y + destY;

      // Calculate byte and bit positions in the destination buffer
      int32_t destByteIndex = ((int32_t)finalDestY * destinationBuffer.width + finalDestX) / 2;
      if (destByteIndex < 0 || destByteIndex >= (int32_t)destinationBuffer.width * destinationBuffer.height / 2) 
      {
        continue;
      }

      if ((finalDestX % 2) == 0) 
      { // Low nibble
        destinationBuffer.data[destByteIndex] = (destinationBuffer.data[destByteIndex] & 0xF0) | srcPixelValue;
      } 
      else 
      { // High nibble
        destinationBuffer.data[destByteIndex] = (destinationBuffer.data[destByteIndex] & 0x0F) | (srcPixelValue << 4);
      }
    }
  }
}