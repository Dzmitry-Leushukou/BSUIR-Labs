```powershell
g++ -std=c++17 -O2 src/main.cpp -o Lab7.exe -lbcrypt

.\Lab7.exe keygen private.key public.key
.\Lab7.exe encrypt public.key test.txt encrypted.bin
.\Lab7.exe decrypt private.key encrypted.bin decrypted.txt

fc.exe /b test.txt decrypted.txt
```

Expected output:

```text
Key pair generated
File encrypted
File decrypted
FC: no differences encountered
```
