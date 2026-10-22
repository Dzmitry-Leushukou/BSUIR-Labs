Build in Windows PowerShell with MinGW `g++`:

```powershell
g++ -std=c++20 -O2 -Wall -Wextra src\main.cpp -o Lab4.exe -lbcrypt
```

Generate a key pair, encrypt the test file, and decrypt it:

```powershell
.\Lab4.exe keygen private.key public.key
.\Lab4.exe e public.key test.txt encrypted.bin
.\Lab4.exe d private.key encrypted.bin decrypted.txt
fc /b test.txt decrypted.txt
```

The last command must report that the files are identical.
