```powershell
cmake --preset ninja-release
cmake --build --preset release
.\build\release\Lab5.exe sha1 .\test.txt
.\build\release\Lab5.exe gost256 .\test.txt
.\build\release\Lab5.exe gost512 .\test.txt
```
