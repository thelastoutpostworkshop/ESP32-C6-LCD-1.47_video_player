#include <AnimatedGIF.h>    // Install this library with the Arduino IDE Library Manager (last tested on v2.2.0)
#include "SD.h"             // Included with the Espressif Arduino Core (last tested on v3.2.0)
#include "FS.h"             // Included with the Espressif Arduino Core (last tested on v3.2.0)
#include "Display_ST7789.h" // Included in this project

#define SD_CS 4 // SD Card CS pin
uint16_t SDCard_Size;
uint16_t Flash_Size;

#define BOOT_KEY_PIN 9 // Boot switch used as input

const char *GIF_FOLDER = "/gif";
AnimatedGIF gif;

// Storage for files to read on the SD card, adjust maximum value as needed
#define MAX_FILES 20 // Adjust as needed
String gifFileList[MAX_FILES];
uint32_t gifFileSizes[MAX_FILES] = {0}; // Store each GIF file's size in bytes
int fileCount = 0;
static int currentFile = 0;
static File FSGifFile; // temp gif file holder

void setup()
{
  delay(2000); // Give time for the board to reconnect to com port
  Serial.begin(115200);
  flash_size();
  LCD_Init();
  SD_Init();
  loadGifFilesList();
  pinMode(BOOT_KEY_PIN, INPUT);   

  // tft.setTextColor(TFT_GREEN);
  // tft.setFont(FONT_12x16);
  // tft.println("Generic SPI display");
  // tft.println("Using bb_spi_lcd");
}

void loop()
{
  delay(1);
}

void SD_Init()
{
  // SD
  if (SD.begin(SD_CS, SPI, 80000000, "/sd", 5, true))
  {
    Serial.printf("SD card initialization successful!\r\n");
  }
  else
  {
    Serial.printf("SD card initialization failed!\r\n");
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.printf("No SD card attached\r\n");
    return;
  }
  else
  {
    Serial.printf("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
      Serial.printf("MMC\r\n");
    }
    else if (cardType == CARD_SD)
    {
      Serial.printf("SDSC\r\n");
    }
    else if (cardType == CARD_SDHC)
    {
      Serial.printf("SDHC\r\n");
    }
    else
    {
      Serial.printf("UNKNOWN\r\n");
    }
    uint64_t totalBytes = SD.totalBytes();
    uint64_t usedBytes = SD.usedBytes();
    SDCard_Size = totalBytes / (1024 * 1024);
    Serial.printf("Total space: %llu\n", totalBytes);
    Serial.printf("Used space: %llu\n", usedBytes);
    Serial.printf("Free space: %llu\n", totalBytes - usedBytes);
  }
}

// Read the gif file list in the gif folder
void loadGifFilesList()
{
  File gifDir = SD.open(GIF_FOLDER);
  if (!gifDir)
  {
    Serial.println("Failed to open GIF folder");
    return;
  }
  fileCount = 0;
  while (true)
  {
    File file = gifDir.openNextFile();
    if (!file)
      break;
    if (!file.isDirectory())
    {
      String name = file.name();
      if (name.endsWith(".gif") || name.endsWith(".GIF"))
      {
        gifFileList[fileCount] = name;
        gifFileSizes[fileCount] = file.size(); // Save file size (in bytes)
        fileCount++;
        if (fileCount >= MAX_FILES)
          break;
      }
    }
    file.close();
  }
  gifDir.close();
  Serial.printf("%d gif files read\n", fileCount);
  // Optionally, print out each file's size for debugging:
  for (int i = 0; i < fileCount; i++)
  {
    Serial.printf("File %d: %s, Size: %lu bytes\n", i, gifFileList[i].c_str(), gifFileSizes[i]);
  }
}

void flash_size()
{
  // Get Flash size
  uint32_t flashSize = ESP.getFlashChipSize();
  Flash_Size = flashSize / 1024 / 1024;
  Serial.printf("Flash size: %d MB \r\n", flashSize / 1024 / 1024);
}