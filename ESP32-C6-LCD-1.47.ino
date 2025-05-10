// Tutorial : 
// Use board "ESP32C6 Dev Module" (last tested on v3.2.0)
//

// Install "GFX Library for Arduino" with the Library Manager (last tested on v1.5.9)

#include <AnimatedGIF.h> // Install this library with the Arduino IDE Library Manager (last tested on v2.2.0)
#include "SD.h"          // Included with the Espressif Arduino Core (last tested on v3.2.0)
#include "FS.h"          // Included with the Espressif Arduino Core (last tested on v3.2.0)
#include "PINS_ESP32-C6-LCD-1_47.h"

#define GFX_BRIGHTNESS 255
#define LCD_WIDTH 172  // LCD width
#define LCD_HEIGHT 320 // LCD height

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
    DEV_DEVICE_INIT();
    delay(2000); // Give time for the board to reconnect to com port
    Serial.begin(115200);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    // Init Display
    if (!gfx->begin(GFX_SPEED))
    {
        Serial.println("gfx->begin() failed!");
        while (true)
        {
            /* code */
        }
    }
    gfx->fillScreen(RGB565_BLACK);
    gfx->setRotation(0);
    ledcAttachChannel(GFX_BL, 1000, 8, 1);
    ledcWrite(GFX_BL, GFX_BRIGHTNESS);
    if (!SD.begin(SD_CS, SPI, 80000000, "/sd"))
    {
        Serial.println("ERROR: File system mount failed!");
        while (true)
        {
            /* code */
        }
    }
    gif.begin(BIG_ENDIAN_PIXELS);
    loadGifFilesList();
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
                {        // done, stop
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
                gfx->draw16bitBeRGBBitmap(pDraw->iX + x, y, usTemp, iCount, 1);
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
        gfx->draw16bitBeRGBBitmap(pDraw->iX, y, usTemp, iWidth, 1);
    }
}

// Play a gif directly from the SD card
void gifPlayFromSDCard(char *gifPath)
{
    if (!gif.open(gifPath,
                  GIFOpenFile, GIFCloseFile,
                  GIFReadFile, GIFSeekFile, GIFDraw))
    {
        Serial.printf("Could not open gif %s\n", gifPath);
        return;
    }

    Serial.printf("Starting playing gif %s\n", gifPath);

    /* ----------  timing variables  ---------- */
    uint32_t tGifStart = millis(); // whole‑GIF timer
    uint32_t frameIndex = 0;

    /* ----------  play loop  ---------- */
    while (true)
    {
        uint32_t tFrameStart = micros(); // per‑frame timer

        bool more = gif.playFrame(false /*use internal delay?*/, nullptr);

        uint32_t frameTime = micros() - tFrameStart; // µs for this frame
        Serial.printf("Frame %u rendered in %u µs (%.2f ms)\n",
                      frameIndex++, frameTime, frameTime / 1000.0f);

        if (!more) // last frame done?
            break;
    }

    gif.close();

    /* ----------  final stats  ---------- */
    uint32_t gifElapsed = millis() - tGifStart; // ms
    Serial.printf("Finished %s: %u frames in %u ms (%.2f s)\n\n",
                  gifPath, frameIndex, gifElapsed, gifElapsed / 1000.0f);
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

// Read the gif file list in the gif folder
void loadGifFilesList()
{
    File gifDir = SD.open(GIF_FOLDER);
    if (!gifDir)
    {
        Serial.printf("Failed to open %s folder\n", GIF_FOLDER);
        while (true)
        {
            /* code */
        }
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