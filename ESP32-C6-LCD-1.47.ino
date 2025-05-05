#include <AnimatedGIF.h> // Install this library with the Arduino IDE Library Manager (last tested on v2.2.0)
#include "SD.h"          // Included with the Espressif Arduino Core (last tested on v3.2.0)
#include "FS.h"          // Included with the Espressif Arduino Core (last tested on v3.2.0)

// Display pins
#define CS_PIN 14
#define DC_PIN 15
#define RESET_PIN 21
#define LED_PIN 22
#define MISO_PIN 5
#define MOSI_PIN 6
#define CLK_PIN 7

// SD Card pins
#define SD_CS 4
uint16_t SDCard_Size;
uint16_t Flash_Size;

void setup()
{
  delay(2000); // Give time for the board to reconnect to com port
  Serial.begin(115200);
  SD_Init();

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
bool File_Search(const char *directory, const char *fileName)
{
  File Path = SD.open(directory);
  if (!Path)
  {
    Serial.printf("Path: <%s> does not exist\r\n", directory);
    return false;
  }
  File file = Path.openNextFile();
  while (file)
  {
    if (strcmp(file.name(), fileName) == 0)
    {
      if (strcmp(directory, "/") == 0)
        Serial.printf("File '%s%s' found in root directory.\r\n", directory, fileName);
      else
        Serial.printf("File '%s/%s' found in root directory.\r\n", directory, fileName);
      Path.close();
      return true;
    }
    file = Path.openNextFile();
  }
  if (strcmp(directory, "/") == 0)
    Serial.printf("File '%s%s' not found in root directory.\r\n", directory, fileName);
  else
    Serial.printf("File '%s/%s' not found in root directory.\r\n", directory, fileName);
  Path.close();
  return false;
}
uint16_t Folder_retrieval(const char *directory, const char *fileExtension, char File_Name[][100], uint16_t maxFiles)
{
  File Path = SD.open(directory);
  if (!Path)
  {
    Serial.printf("Path: <%s> does not exist\r\n", directory);
    return false;
  }

  uint16_t fileCount = 0;
  char filePath[100];
  File file = Path.openNextFile();
  while (file && fileCount < maxFiles)
  {
    if (!file.isDirectory() && strstr(file.name(), fileExtension))
    {
      strncpy(File_Name[fileCount], file.name(), sizeof(File_Name[fileCount]));
      if (strcmp(directory, "/") == 0)
      {
        snprintf(filePath, 100, "%s%s", directory, file.name());
      }
      else
      {
        snprintf(filePath, 100, "%s/%s", directory, file.name());
      }
      Serial.printf("File found: %s\r\n", filePath);
      fileCount++;
    }
    file = Path.openNextFile();
  }
  Path.close();
  if (fileCount > 0)
  {
    Serial.printf(" %d <%s> files were retrieved\r\n", fileCount, fileExtension);
    return fileCount;
  }
  else
  {
    Serial.printf("No files with extension '%s' found in directory: %s\r\n", fileExtension, directory);
    return 0;
  }
}
