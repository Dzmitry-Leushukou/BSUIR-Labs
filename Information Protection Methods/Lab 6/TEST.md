```powershell
g++ -std=c++17 -O2 src/main.cpp -o Lab6.exe -lbcrypt

.\Lab6.exe keygen private.key public.key
.\Lab6.exe sign private.key test.txt test.sig
.\Lab6.exe verify public.key test.txt test.sig

Copy-Item test.txt tampered.txt
Add-Content tampered.txt 'Changed content'
.\Lab6.exe verify public.key tampered.txt test.sig
```
