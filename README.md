# ESP32-C6-LCD-1.47

ffmpeg -y -i .\human_scan.mp4 -pix_fmt yuvj420p -q:v 7 -vf "fps=24,scale=172:-1:flags=lanczos" human_scan.mjpeg

ffmpeg -y -i .\human_scan.mp4 -pix_fmt yuvj420p -q:v 7 -vf "fps=24,scale=172:320:flags=lanczos" human_scan.mjpeg

16:9 to 9:16
ffmpeg -y -i .\enterprise.mp4 -ss 0:00:01 -pix_fmt yuvj420p -q:v 7 -vf "transpose=1,fps=24,scale=172:320:flags=lanczos" enterprise.mjpeg

