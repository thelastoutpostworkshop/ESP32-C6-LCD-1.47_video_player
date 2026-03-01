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

## 🎬 How to Use These FFmpeg Commands

Each of the following commands generates a `.mjpeg` file — a Motion JPEG video format — from an input `.mp4` or `.mov` video, optimized for use in frame-by-frame playback with an SD card reader.

Make sure you have [FFmpeg](https://ffmpeg.org/download.html) installed and accessible from your terminal or command prompt.

---

### Convert MP4/MOV to MJPEG — Maintain Aspect Ratio

This command rescales the width to **172 pixels**, automatically adjusting height to keep the **original aspect ratio**.
> Use when you want a smaller width and you don’t want to distort the original video (Height can vary)
```cmd
ffmpeg -y -i .\human_scan.mp4 -pix_fmt yuvj420p -q:v 7 -vf "fps=24,scale=172:-1:flags=lanczos" human_scan.mjpeg
```
### Convert MP4/MOV to MJPEG — Force 172×320
> Use when you want to target 172x320 (Stretching is acceptable)
```cmd
ffmpeg -y -i .\human_scan.mp4 -pix_fmt yuvj420p -q:v 7 -vf "fps=24,scale=172:320:flags=lanczos" human_scan.mjpeg
```
### Convert MP4/MOV to MJPEG — Rotate 16:9 to Portrait 9:16 
> Use when your source video is landscape (16:9)
```cmd
ffmpeg -y -i .\enterprise.mp4 -pix_fmt yuvj420p -q:v 7 -vf "transpose=1,fps=24,scale=172:320:flags=lanczos" enterprise.mjpeg
```
### Options explained
- -pix_fmt yuvj420p: Ensures JPEG-compatible pixel format
- -q:v 7: Controls image quality (lower is better; 1 = best, 31 = worst)
- -vf: Specifies the video filters:
- fps=24: Extracts 24 frames per second
- scale: Resizes the video
- transpose=1: Rotates the video 90° clockwise
- .mjpeg: Output format used when streaming or storing a series of JPEG frames as a video

