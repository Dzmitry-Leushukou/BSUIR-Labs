```
cmake --build build/release --config Release
```

```powershell
.\build\release\Lab1.exe encrypt simple test_input.txt test_simple.bin 0102030411121314212223243132333441424344515253546162636471727374
```

```powershell
.\build\release\Lab1.exe decrypt simple test_simple.bin test_simple_decrypted.txt 0102030411121314212223243132333441424344515253546162636471727374
```

```powershell
.\build\release\Lab1.exe encrypt gamma test_input.txt test_gamma.bin 0102030411121314212223243132333441424344515253546162636471727374 1234567890abcdef
```

```powershell
.\build\release\Lab1.exe decrypt gamma test_gamma.bin test_gamma_decrypted.txt 0102030411121314212223243132333441424344515253546162636471727374 1234567890abcdef
```

```powershell
.\build\release\Lab1.exe encrypt feedback test_input.txt test_feedback.bin 0102030411121314212223243132333441424344515253546162636471727374 1234567890abcdef
```

```powershell
.\build\release\Lab1.exe decrypt feedback test_feedback.bin test_feedback_decrypted.txt 0102030411121314212223243132333441424344515253546162636471727374 1234567890abcdef
```

```powershell
.\build\release\Lab1.exe imito test_input.txt 0102030411121314212223243132333441424344515253546162636471727374
```
