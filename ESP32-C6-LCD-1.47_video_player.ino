// Tutorial :
// Use board "ESP32C6 Dev Module" (last tested on v3.2.0)

#include "PINS_ESP32-C6-LCD-1_47.h" // Install "GFX Library for Arduino" with the Library Manager (last tested on v1.5.9)
#include "MjpegClass.h"             // Included in this project
#include "SD.h"                     // Included with the Espressif Arduino Core (last tested on v3.2.0)
#include "Arduino.h"

#define GFX_BRIGHTNESS 255

const char *MJPEG_FOLDER = "/mjpeg";

// Storage for files to read on the SD card, adjust maximum value as needed
#define MAX_FILES 20 // Adjust as needed
String mjpegFileList[MAX_FILES];
uint32_t mjpegFileSizes[MAX_FILES] = {0}; // Store each GIF file's size in bytes
int mjpegCount = 0;
static int currentMjpegIndex = 0;
static File mjpegFile; // temp gif file holder

// Global variables for mjpeg
MjpegClass mjpeg;
int total_frames;
unsigned long total_read_video;
unsigned long total_decode_video;
unsigned long total_show_video;
unsigned long start_ms, curr_ms;
long output_buf_size, estimateBufferSize;
uint8_t *mjpeg_buf;
uint16_t *output_buf;

// Interrupt to skip to the next mjpeg when the boot button is pressed
volatile bool skipRequested = false;
volatile uint32_t lastPress = 0;
void IRAM_ATTR onButtonPress()
{
    uint32_t now = millis();
    if (now - lastPress > 300)
    { // 300 ms debounce
        skipRequested = true;
        lastPress = now;
    }
}

void setup()
{
    Serial.begin(115200);
    DEV_DEVICE_INIT();
    delay(2000); // For debugging, give time for the board to reconnect to com port

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    // Display initialization
    if (!gfx->begin(GFX_SPEED))
    {
        Serial.println("Display initialization failed!");
        while (true)
        {
            /* no need to continue */
        }
    }
    gfx->setRotation(0);
    gfx->fillScreen(RGB565_BLACK);
    setDisplayBrigthness();

    // SD card initialization
    if (!SD.begin(SD_CS, SPI, 80000000, "/sd"))
    {
        Serial.println("ERROR: File system mount failed!");
        while (true)
        {
            /* no need to continue */
        }
    }

    // Buffer allocation for mjpeg playing
    output_buf_size = gfx->width() * 4 * 2;
    output_buf = (uint16_t *)heap_caps_aligned_alloc(16, output_buf_size * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!output_buf)
    {
        Serial.println("output_buf aligned_alloc failed!");
        while (true)
        {
            /* no need to continue */
        }
    }
    estimateBufferSize = gfx->width() * gfx->height() * 2 / 5;
    mjpeg_buf = (uint8_t *)heap_caps_malloc(estimateBufferSize, MALLOC_CAP_8BIT);

    loadMjpegFilesList();

    // Set the boot button to skip the current mjpeg playing and go to the next
    pinMode(BTN_A, INPUT);                        // active‑low
    attachInterrupt(digitalPinToInterrupt(BTN_A), // fast ISR
                    onButtonPress, FALLING);      // press == LOW
}

// Set the brightness of the display to GFX_BRIGHTNESS
void setDisplayBrigthness()
{
    ledcAttachChannel(GFX_BL, 1000, 8, 1);
    ledcWrite(GFX_BL, GFX_BRIGHTNESS);
}
void loop()
{
    playSelectedMjpeg(currentMjpegIndex);
    currentMjpegIndex++;
    if (currentMjpegIndex >= mjpegCount)
    {
        currentMjpegIndex = 0;
    }
}

// Play the current mjpeg
void playSelectedMjpeg(int mjpegIndex)
{
    // Build the full path for the selected mjpeg
    String fullPath = String(MJPEG_FOLDER) + "/" + mjpegFileList[mjpegIndex];
    char mjpegFilename[128];
    fullPath.toCharArray(mjpegFilename, sizeof(mjpegFilename));

    Serial.printf("Playing %s\n", mjpegFilename);
    mjpegPlayFromSDCard(mjpegFilename);
}

// Callback function to draw a JPEG
int jpegDrawCallback(JPEGDRAW *pDraw)
{
    unsigned long s = millis();
    gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
    total_show_video += millis() - s;
    return 1;
}

// Play a mjpeg stored on the SD card
void mjpegPlayFromSDCard(char *mjpegFilename)
{
    File mjpegFile = SD.open(mjpegFilename, "r");

    if (!mjpegFile || mjpegFile.isDirectory())
    {
        Serial.printf("ERROR: Failed to open %s file for reading\n", mjpegFilename);
    }
    else
    {
        Serial.println(F("MJPEG start"));
        gfx->fillScreen(RGB565_BLACK);

        start_ms = millis();
        curr_ms = millis();
        total_frames = 0;
        total_read_video = 0;
        total_decode_video = 0;
        total_show_video = 0;

        mjpeg.setup(
            &mjpegFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
            0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);

        while (!skipRequested && mjpegFile.available() && mjpeg.readMjpegBuf())
        {
            // Read video
            total_read_video += millis() - curr_ms;
            curr_ms = millis();

            // Play video
            mjpeg.drawJpg();
            total_decode_video += millis() - curr_ms;

            curr_ms = millis();
            total_frames++;
        }
        int time_used = millis() - start_ms;
        Serial.println(F("MJPEG end"));
        mjpegFile.close();
        skipRequested = false; // ready for next video
        float fps = 1000.0 * total_frames / time_used;
        total_decode_video -= total_show_video;
        Serial.printf("Total frames: %d\n", total_frames);
        Serial.printf("Time used: %d ms\n", time_used);
        Serial.printf("Average FPS: %0.1f\n", fps);
        Serial.printf("Read MJPEG: %lu ms (%0.1f %%)\n", total_read_video, 100.0 * total_read_video / time_used);
        Serial.printf("Decode video: %lu ms (%0.1f %%)\n", total_decode_video, 100.0 * total_decode_video / time_used);
        Serial.printf("Show video: %lu ms (%0.1f %%)\n", total_show_video, 100.0 * total_show_video / time_used);
    }
}

// Read the mjpeg file list in the mjpeg folder
void loadMjpegFilesList()
{
    File mjpegDir = SD.open(MJPEG_FOLDER);
    if (!mjpegDir)
    {
        Serial.printf("Failed to open %s folder\n", MJPEG_FOLDER);
        while (true)
        {
            /* code */
        }
    }
    mjpegCount = 0;
    while (true)
    {
        File file = mjpegDir.openNextFile();
        if (!file)
            break;
        if (!file.isDirectory())
        {
            String name = file.name();
            if (name.endsWith(".mjpeg"))
            {
                mjpegFileList[mjpegCount] = name;
                mjpegFileSizes[mjpegCount] = file.size(); // Save file size (in bytes)
                mjpegCount++;
                if (mjpegCount >= MAX_FILES)
                    break;
            }
        }
        file.close();
    }
    mjpegDir.close();
    Serial.printf("%d mjpeg files read\n", mjpegCount);
    // Optionally, print out each file's size for debugging:
    for (int i = 0; i < mjpegCount; i++)
    {
        Serial.printf("File %d: %s, Size: %lu bytes\n", i, mjpegFileList[i].c_str(), mjpegFileSizes[i]);
    }
}