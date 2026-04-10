# ESP32-C6-LCD-1.47 Video Player

<a href="https://www.buymeacoffee.com/thelastoutpostworkshop" target="_blank">
<img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee">
</a>

## Youtube Tutorial
[<img src="https://github.com/thelastoutpostworkshop/images/blob/main/videoplayer.png" width="500">](https://youtu.be/JqQEG0eipic)

## Project Description
ESP32-C6-LCD-1.47 Video Player turns the ESP32-C6-LCD-1.47 board into a standalone MJPEG media player. The firmware scans the SD card for `.mjpeg` files in `/mjpeg`, decodes them frame-by-frame, and renders video on the LCD in a continuous loop. A hardware button lets you skip to the next clip during playback.

## Features
- Automatically discovers `.mjpeg` files from the SD card (`/mjpeg` folder)
- Plays all discovered videos in sequence and loops back to the first file
- Supports quick skip to the next video using the board button (with debounce)
- Uses SD streaming + JPEG decoding for frame-by-frame MJPEG playback
- Renders directly to the display in RGB565 format
- Prints runtime playback metrics over Serial (frames, FPS, read/decode/display time)
- Includes ready-to-use FFmpeg conversion commands for `.mp4`/`.mov` sources

## What You Can Customize
- `GFX_BRIGHTNESS`: display backlight brightness
- `MJPEG_FOLDER`: SD directory used to discover videos
- `MAX_FILES`: maximum number of indexed `.mjpeg` files

## Convert Videos with Video Conversion Studio

<p align="center">
  <a href="https://thelastoutpostworkshop.github.io/video_conversion/">
    <img src="https://img.shields.io/badge/Open-Video%20Conversion%20Studio-FF6A00?style=for-the-badge" alt="Open Video Conversion Studio">
  </a>
</p>

<p align="center">
  FFmpeg is no longer necessary. There are no complex commands to learn. Just use <a href="https://thelastoutpostworkshop.github.io/video_conversion/"><strong>Video Conversion Studio</strong></a> to prepare MJPEG files for this player.
</p>
