```powershell
g++ -std=c++17 -O2 src/main.cpp src/JpegSteganography.cpp -I vcpkg_installed/x64-mingw-dynamic/include -L vcpkg_installed/x64-mingw-dynamic/lib -ljpeg -lshell32 -o Lab8.exe
Copy-Item vcpkg_installed/x64-mingw-dynamic/bin/libjpeg-62.dll .

.\Lab8.exe image.jpg
.\Lab8.exe hide image.jpg "Секретное сообщение Lab 8" stego.jpg
.\Lab8.exe extract stego.jpg

.\Lab8.exe hide-file image.jpg test.txt stego.jpg
.\Lab8.exe extract stego.jpg restored.txt
fc.exe /b test.txt restored.txt

.\Lab8.exe extract image.jpg

Remove-Item stego.jpg, restored.txt
```
