# ESP32-C6-LCD-1.47 Movie Viewer

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

