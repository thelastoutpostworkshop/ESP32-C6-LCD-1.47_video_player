#pragma once
#include <Arduino.h>
#include <SPI.h>
#define LCD_WIDTH 172  // LCD width
#define LCD_HEIGHT 320 // LCD height

#define SPIFreq 80000000
#define MISO_PIN 5
#define MOSI_PIN 6
#define CLK_PIN 7
#define CS_PIN 14
#define DC_PIN 15
#define RESET_PIN 21
#define LED_PIN 22
#define Frequency 1000
#define Resolution 10

#define VERTICAL 0
#define HORIZONTAL 1

#define Offset_X 34
#define Offset_Y 0

void SPI_Init();

void LCD_Init(void);
void LCD_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend);
void LCD_addWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t *color);

void Backlight_Init(void);
void Set_Backlight(uint8_t Light);
