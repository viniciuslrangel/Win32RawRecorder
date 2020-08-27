# Win32RawRecorder
Record raw pixels from Windows' window


ffmpeg usage example:
```commandline
win32rawrecorder.exe --title "@cmd.exe" | ffmpeg -f rawvideo -pixel_format bgra -video_size 600x300 -framerate 60 -i - -vf vflip -c:a copy output.avi
```
Video size can be queried from `win32rawrecorder.exe --title "@cmd.exe" --info` 
