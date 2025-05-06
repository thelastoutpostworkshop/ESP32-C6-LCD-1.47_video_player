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
int gifCount = 0;
static int currentGifIndex = 0;
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
  gif.begin(BIG_ENDIAN_PIXELS);

  // tft.setTextColor(TFT_GREEN);
  // tft.setFont(FONT_12x16);
  // tft.println("Generic SPI display");
  // tft.println("Using bb_spi_lcd");
}

void loop()
{
  playSelectedGif(currentGifIndex);
  currentGifIndex++;
  if (currentGifIndex >= gifCount)
  {
    currentGifIndex = 0;
  }
}

// Callback function to open a gif file from the SD card
static void *GIFOpenFile(const char *fname, int32_t *pSize)
{
  Serial.printf("Opening %s from SD\n", fname);
  FSGifFile = SD.open(fname);
  if (FSGifFile)
  {
    *pSize = FSGifFile.size();
    return (void *)&FSGifFile;
  }
  return NULL;
}

// Callback function to close a gif file from the SD card
static void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
    f->close();
}

// Callback function to read a gif file from the SD card
static int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
  int32_t iBytesRead;
  iBytesRead = iLen;
  File *f = static_cast<File *>(pFile->fHandle);
  // Note: If you read a file all the way to the last byte, seek() stops working
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
  if (iBytesRead <= 0)
    return 0;
  iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();
  return iBytesRead;
}

// Callback function to seek a gif file from the SD card
static int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
  // log_d("Seek time = %d us\n", i);
  return pFile->iPos;
}

// Callback function to Draw a line of image directly on the screen
// void GIFDraw(GIFDRAW *pDraw)
// {
//   uint8_t *s;
//   uint16_t *d, *usPalette, usTemp[LCD_WIDTH];
//   int x, y, iWidth;

//   iWidth = pDraw->iWidth;
//   if (iWidth + pDraw->iX > LCD_WIDTH)
//   {
//     iWidth = LCD_WIDTH - pDraw->iX;
//   }
//   usPalette = pDraw->pPalette;
//   y = pDraw->iY + pDraw->y; // current line
//   if (y >= LCD_HEIGHT || pDraw->iX >= LCD_WIDTH || iWidth < 1)
//   {
//     return;
//   }
//   s = pDraw->pPixels;
//   if (pDraw->ucDisposalMethod == 2) // restore to background color
//   {
//     for (x = 0; x < iWidth; x++)
//     {
//       if (s[x] == pDraw->ucTransparent)
//       {
//         s[x] = pDraw->ucBackground;
//       }
//     }
//     pDraw->ucHasTransparency = 0;
//   }

//   // Apply the new pixels to the main image
//   if (pDraw->ucHasTransparency) // if transparency used
//   {
//     uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
//     int x, iCount;
//     pEnd = s + iWidth;
//     x = 0;
//     iCount = 0; // count non-transparent pixels
//     while (x < iWidth)
//     {
//       c = ucTransparent - 1;
//       d = usTemp;
//       while (c != ucTransparent && s < pEnd)
//       {
//         c = *s++;
//         if (c == ucTransparent) // done, stop
//         {
//           s--; // back up to treat it like transparent
//         }
//         else // opaque
//         {
//           *d++ = usPalette[c];
//           iCount++;
//         }
//       } // while looking for opaque pixels
//       if (iCount) // any opaque pixels?
//       {
//         LCD_addWindow(
//             pDraw->iX + x,              // Xstart
//             y,                          // Ystart
//             pDraw->iX + x + iCount - 1, // Xend  (inclusive)
//             y,                          // Yend   (only one scan‑line tall)
//             usTemp                      // pixel buffer (RGB565, big‑endian)
//         );
//         x += iCount;
//         iCount = 0;
//       }
//       // no, look for a run of transparent pixels
//       c = ucTransparent;
//       while (c == ucTransparent && s < pEnd)
//       {
//         c = *s++;
//         if (c == ucTransparent)
//         {
//           iCount++;
//         }
//         else
//         {
//           s--;
//         }
//       }
//       if (iCount)
//       {
//         x += iCount; // skip these
//         iCount = 0;
//       }
//     }
//   }
//   else
//   {
//     s = pDraw->pPixels;
//     // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
//     for (x = 0; x < iWidth; x++)
//     {
//       usTemp[x] = usPalette[*s++];
//     }
//     LCD_addWindow(
//         pDraw->iX,              // Xstart
//         y,                      // Ystart
//         pDraw->iX + iWidth - 1, // Xend
//         y,                      // Yend
//         usTemp                  // pixel buffer
//     );
//   }
// }

// Callback function to Draw a line of image directly on the screen
void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > LCD_WIDTH)
    iWidth = LCD_WIDTH;
  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2)
  { // restore to background color
    for (x = 0; x < iWidth; x++)
    {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency)
  { // if transparency used
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < iWidth)
    {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
        {      // done, stop
          s--; // back up to treat it like transparent
        }
        else
        { // opaque
          *d++ = usPalette[c];
          iCount++;
        }
      } // while looking for opaque pixels
      if (iCount)
      { // any opaque pixels?
        LCD_addWindow(
            pDraw->iX + x,              // Xstart
            y,                          // Ystart
            pDraw->iX + x + iCount - 1, // Xend  (inclusive)
            y,                          // Yend   (only one scan‑line tall)
            usTemp                      // pixel buffer (RGB565, big‑endian)
        );
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
          iCount++;
        else
          s--;
      }
      if (iCount)
      {
        x += iCount; // skip these
        iCount = 0;
      }
    }
  }
  else
  {
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < iWidth; x++)
      usTemp[x] = usPalette[*s++];
    LCD_addWindow(
        pDraw->iX,              // Xstart
        y,                      // Ystart
        pDraw->iX + iWidth - 1, // Xend
        y,                      // Yend
        usTemp                  // pixel buffer
    );
  }
} 

// Play a gif directly from the SD card
void gifPlayFromSDCard(char *gifPath)
{

  if (!gif.open(gifPath, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
  {
    Serial.printf("Could not open gif %s", gifPath);
  }
  else
  {
    Serial.printf("Starting playing gif %s\n", gifPath);

    while (gif.playFrame(false /*change to true to use the internal gif frame duration*/, NULL))
    {
    }

    gif.close();
  }
}

void playSelectedGif(int gifIndex)
{
  // Build the full path for the selected GIF.
  String fullPath = String(GIF_FOLDER) + "/" + gifFileList[gifIndex];
  char gifFilename[128];
  fullPath.toCharArray(gifFilename, sizeof(gifFilename));

  Serial.printf("Playing %s\n", gifFilename);
  gifPlayFromSDCard(gifFilename);
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
  gifCount = 0;
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
        gifFileList[gifCount] = name;
        gifFileSizes[gifCount] = file.size(); // Save file size (in bytes)
        gifCount++;
        if (gifCount >= MAX_FILES)
          break;
      }
    }
    file.close();
  }
  gifDir.close();
  Serial.printf("%d gif files read\n", gifCount);
  // Optionally, print out each file's size for debugging:
  for (int i = 0; i < gifCount; i++)
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