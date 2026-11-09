```powershell
g++ -std=c++17 -O2 src/main.cpp src/EllipticCurve.cpp src/ElGamal.cpp src/CipherFile.cpp -o Lab7.exe -lbcrypt

.\Lab7.exe keygen private.key public.key
.\Lab7.exe encrypt public.key test.txt encrypted.bin
.\Lab7.exe decrypt private.key encrypted.bin decrypted.txt

fc.exe /b test.txt decrypted.txt
```
