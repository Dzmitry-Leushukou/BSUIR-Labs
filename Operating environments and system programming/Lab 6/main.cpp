#include <windows.h>
#include <winreg.h>
#include <iphlpapi.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <psapi.h>
#include <tlhelp32.h>
#include <iomanip>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")

class SystemInfo {
public:
    static void DisplayAll() {
        std::wcout << L"=== SYSTEM INFORMATION ===\n\n";
        DisplayOSInfo();
        DisplayCPUInfo();
        DisplayMemoryInfo();
        DisplayDiskInfo();
        DisplayNetworkInfo();
        DisplayBIOSInfo();
        DisplayUptime();
        DisplayProcessInfo();
        DisplayDisplayInfo();
        DisplayPerformanceInfo();
    }

    static void DisplayOSInfo() {
        std::wcout << L"--- Operating System ---\n";

        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            wchar_t productName[256];
            wchar_t currentBuild[256];
            wchar_t displayVersion[256];
            wchar_t buildLabEx[256];
            DWORD size = sizeof(productName);

            if (RegQueryValueEx(hKey, L"ProductName", nullptr, nullptr,
                (LPBYTE)productName, &size) == ERROR_SUCCESS) {
                std::wcout << L"Name: " << productName << L"\n";
            }

            size = sizeof(displayVersion);
            if (RegQueryValueEx(hKey, L"DisplayVersion", nullptr, nullptr,
                (LPBYTE)displayVersion, &size) == ERROR_SUCCESS) {
                std::wcout << L"Version: " << displayVersion << L"\n";
            }

            size = sizeof(currentBuild);
            if (RegQueryValueEx(hKey, L"CurrentBuild", nullptr, nullptr,
                (LPBYTE)currentBuild, &size) == ERROR_SUCCESS) {
                std::wcout << L"Build: " << currentBuild << L"\n";
            }

            size = sizeof(buildLabEx);
            if (RegQueryValueEx(hKey, L"BuildLabEx", nullptr, nullptr,
                (LPBYTE)buildLabEx, &size) == ERROR_SUCCESS) {
                std::wcout << L"Lab: " << buildLabEx << L"\n";
            }

            DWORD installDate;
            size = sizeof(installDate);
            if (RegQueryValueEx(hKey, L"InstallDate", nullptr, nullptr,
                (LPBYTE)&installDate, &size) == ERROR_SUCCESS) {
                time_t time = installDate;
                struct tm timeinfo;
                localtime_s(&timeinfo, &time);
                wchar_t buffer[80];
                wcsftime(buffer, sizeof(buffer), L"%Y-%m-%d %H:%M:%S", &timeinfo);
                std::wcout << L"Install Date: " << buffer << L"\n";
            }

            RegCloseKey(hKey);
        }

        SYSTEM_INFO sysInfo;
        GetNativeSystemInfo(&sysInfo);
        std::wcout << L"Architecture: ";
        switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: std::wcout << L"x64 (AMD or Intel)\n"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: std::wcout << L"x86\n"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: std::wcout << L"ARM64\n"; break;
        case PROCESSOR_ARCHITECTURE_ARM: std::wcout << L"ARM (32-bit)\n"; break;
        default: std::wcout << L"Unknown\n";
        }

        std::wcout << L"Page Size: " << sysInfo.dwPageSize << L" bytes\n";
        std::wcout << L"Minimum App Address: " << sysInfo.lpMinimumApplicationAddress << L"\n";
        std::wcout << L"Maximum App Address: " << sysInfo.lpMaximumApplicationAddress << L"\n";

        std::wcout << L"\n";
    }

    static void DisplayCPUInfo() {
        std::wcout << L"--- Processor ---\n";

        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            wchar_t processorName[256];
            DWORD mhz, vendorId;
            DWORD size = sizeof(processorName);

            if (RegQueryValueEx(hKey, L"ProcessorNameString", nullptr, nullptr,
                (LPBYTE)processorName, &size) == ERROR_SUCCESS) {
                std::wcout << L"Model: " << processorName << L"\n";
            }

            size = sizeof(mhz);
            if (RegQueryValueEx(hKey, L"~MHz", nullptr, nullptr,
                (LPBYTE)&mhz, &size) == ERROR_SUCCESS) {
                std::wcout << L"Frequency: " << mhz << L" MHz\n";
            }

            size = sizeof(vendorId);
            if (RegQueryValueEx(hKey, L"VendorIdentifier", nullptr, nullptr,
                (LPBYTE)&vendorId, &size) == ERROR_SUCCESS) {
                std::wcout << L"Vendor ID: " << std::hex << vendorId << std::dec << L"\n";
            }

            RegCloseKey(hKey);
        }

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        std::wcout << L"Number of Cores: " << sysInfo.dwNumberOfProcessors << L"\n";
        std::wcout << L"Processor Type: " << sysInfo.dwProcessorType << L"\n";
        std::wcout << L"Processor Level: " << sysInfo.wProcessorLevel << L"\n";
        std::wcout << L"Processor Revision: " << sysInfo.wProcessorRevision << L"\n";

        // Get CPU usage
        static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
        static int numProcessors;
        static bool firstRun = true;

        if (firstRun) {
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            numProcessors = sysInfo.dwNumberOfProcessors;

            FILETIME ftime, fsys, fuser;
            GetSystemTimeAsFileTime(&ftime);
            memcpy(&lastCPU, &ftime, sizeof(FILETIME));

            GetProcessTimes(GetCurrentProcess(), &ftime, &ftime, &fsys, &fuser);
            memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
            memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
            firstRun = false;
        }

        std::wcout << L"\n";
    }

    static void DisplayMemoryInfo() {
        std::wcout << L"--- Memory ---\n";

        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(memInfo);

        if (GlobalMemoryStatusEx(&memInfo)) {
            std::wcout << L"Total Physical Memory: "
                << (memInfo.ullTotalPhys / (1024 * 1024)) << L" MB\n";
            std::wcout << L"Available Physical Memory: "
                << (memInfo.ullAvailPhys / (1024 * 1024)) << L" MB\n";
            std::wcout << L"Memory Load: " << memInfo.dwMemoryLoad << L"%\n";
        }

        // Get pagefile information
        GetPageFileInfo();

        // Get process memory usage
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            std::wcout << L"Current Process Working Set: " << pmc.WorkingSetSize / (1024 * 1024) << L" MB\n";
            std::wcout << L"Current Process Private Bytes: " << pmc.PrivateUsage / (1024 * 1024) << L" MB\n";
        }

        std::wcout << L"\n";
    }

    static void GetPageFileInfo() {
        // Get pagefile size from registry
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            DWORD pagefileSize = 0;
            DWORD size = sizeof(pagefileSize);

            if (RegQueryValueEx(hKey, L"PagingFiles", nullptr, nullptr, nullptr, &size) == ERROR_SUCCESS) {
                wchar_t* pagefileData = new wchar_t[size / sizeof(wchar_t)];
                if (RegQueryValueEx(hKey, L"PagingFiles", nullptr, nullptr, (LPBYTE)pagefileData, &size) == ERROR_SUCCESS) {
                    // Parse pagefile information (format: "C:\\pagefile.sys 2048 4096")
                    std::wstring pagefileStr(pagefileData);
                    size_t firstSpace = pagefileStr.find(L' ');
                    if (firstSpace != std::wstring::npos) {
                        size_t secondSpace = pagefileStr.find(L' ', firstSpace + 1);
                        if (secondSpace != std::wstring::npos) {
                            std::wstring minSizeStr = pagefileStr.substr(firstSpace + 1, secondSpace - firstSpace - 1);
                            std::wstring maxSizeStr = pagefileStr.substr(secondSpace + 1);

                            DWORD minSize = _wtoi(minSizeStr.c_str());
                            DWORD maxSize = _wtoi(maxSizeStr.c_str());

                            std::wcout << L"Pagefile (min/max): " << minSize << L" MB / " << maxSize << L" MB\n";

                            // Get actual pagefile usage
                            MEMORYSTATUSEX memInfo;
                            memInfo.dwLength = sizeof(memInfo);
                            if (GlobalMemoryStatusEx(&memInfo)) {
                                ULONGLONG totalPageFile = memInfo.ullTotalPageFile / (1024 * 1024);
                                ULONGLONG availPageFile = memInfo.ullAvailPageFile / (1024 * 1024);
                                ULONGLONG usedPageFile = totalPageFile - availPageFile;

                                std::wcout << L"Pagefile Total: " << totalPageFile << L" MB\n";
                                std::wcout << L"Pagefile Available: " << availPageFile << L" MB\n";
                                std::wcout << L"Pagefile Used: " << usedPageFile << L" MB\n";
                            }
                        }
                    }
                }
                delete[] pagefileData;
            }

            RegCloseKey(hKey);
        }
    }

    static void DisplayDiskInfo() {
        std::wcout << L"--- Disk Drives ---\n";

        DWORD drives = GetLogicalDrives();
        wchar_t drive[] = L"A:\\";

        for (int i = 0; i < 26; i++) {
            if (drives & (1 << i)) {
                drive[0] = L'A' + i;
                UINT driveType = GetDriveType(drive);

                std::wcout << L"Drive " << drive[0] << L": (";
                switch (driveType) {
                case DRIVE_UNKNOWN: std::wcout << L"Unknown"; break;
                case DRIVE_NO_ROOT_DIR: std::wcout << L"No root"; break;
                case DRIVE_REMOVABLE: std::wcout << L"Removable"; break;
                case DRIVE_FIXED: std::wcout << L"Fixed"; break;
                case DRIVE_REMOTE: std::wcout << L"Network"; break;
                case DRIVE_CDROM: std::wcout << L"CD-ROM"; break;
                case DRIVE_RAMDISK: std::wcout << L"RAM Disk"; break;
                }
                std::wcout << L")\n";

                ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
                if (GetDiskFreeSpaceEx(drive, &freeBytes, &totalBytes, &totalFreeBytes)) {
                    double totalGB = totalBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
                    double freeGB = freeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
                    double usedGB = totalGB - freeGB;
                    double usagePercent = (usedGB / totalGB) * 100.0;

                    std::wcout << L"  Total: " << std::fixed << std::setprecision(2) << totalGB << L" GB\n";
                    std::wcout << L"  Free: " << std::fixed << std::setprecision(2) << freeGB << L" GB\n";
                    std::wcout << L"  Used: " << std::fixed << std::setprecision(2) << usedGB << L" GB\n";
                    std::wcout << L"  Usage: " << std::fixed << std::setprecision(1) << usagePercent << L"%\n";

                    wchar_t fsName[MAX_PATH];
                    wchar_t volumeName[MAX_PATH];
                    DWORD serialNumber, maxComponentLength, fsFlags;
                    if (GetVolumeInformation(drive, volumeName, MAX_PATH, &serialNumber,
                        &maxComponentLength, &fsFlags, fsName, MAX_PATH)) {
                        std::wcout << L"  File System: " << fsName << L"\n";
                        if (wcslen(volumeName) > 0) {
                            std::wcout << L"  Volume Name: " << volumeName << L"\n";
                        }
                        std::wcout << std::hex << L"  Serial Number: 0x" << serialNumber << std::dec << L"\n";
                    }
                }
                std::wcout << L"\n";
            }
        }
    }

    static void DisplayNetworkInfo() {
        std::wcout << L"--- Network Interfaces ---\n";

        ULONG bufferSize = 0;
        GetAdaptersInfo(nullptr, &bufferSize);

        PIP_ADAPTER_INFO adapterInfo = (IP_ADAPTER_INFO*)malloc(bufferSize);

        if (adapterInfo && GetAdaptersInfo(adapterInfo, &bufferSize) == NO_ERROR) {
            PIP_ADAPTER_INFO adapter = adapterInfo;
            int adapterCount = 0;

            while (adapter) {
                adapterCount++;
                std::wcout << L"Adapter #" << adapterCount << L": " << adapter->Description << L"\n";
                std::wcout << L"  Type: ";
                switch (adapter->Type) {
                case MIB_IF_TYPE_ETHERNET: std::wcout << L"Ethernet"; break;
                case MIB_IF_TYPE_TOKENRING: std::wcout << L"Token Ring"; break;
                case MIB_IF_TYPE_FDDI: std::wcout << L"FDDI"; break;
                case MIB_IF_TYPE_PPP: std::wcout << L"PPP"; break;
                case MIB_IF_TYPE_LOOPBACK: std::wcout << L"Loopback"; break;
                case MIB_IF_TYPE_SLIP: std::wcout << L"SLIP"; break;
                case IF_TYPE_IEEE80211: std::wcout << L"Wireless"; break;
                default: std::wcout << L"Other"; break;
                }
                std::wcout << L"\n";

                std::wcout << L"  MAC: ";
                for (UINT i = 0; i < adapter->AddressLength; i++) {
                    std::wcout << std::hex << std::setw(2) << std::setfill(L'0')
                        << (int)adapter->Address[i];
                    if (i < adapter->AddressLength - 1) std::wcout << L":";
                }
                std::wcout << std::dec << L"\n";

                IP_ADDR_STRING* ipAddr = &adapter->IpAddressList;
                bool hasIP = false;
                while (ipAddr) {
                    if (strlen(ipAddr->IpAddress.String) > 0 && strcmp(ipAddr->IpAddress.String, "0.0.0.0") != 0) {
                        std::wcout << L"  IP Address: " << ipAddr->IpAddress.String << L"\n";
                        std::wcout << L"  Subnet Mask: " << ipAddr->IpMask.String << L"\n";
                        if (adapter->GatewayList.IpAddress.String[0] != '\0' &&
                            strcmp(adapter->GatewayList.IpAddress.String, "0.0.0.0") != 0) {
                            std::wcout << L"  Gateway: " << adapter->GatewayList.IpAddress.String << L"\n";
                        }
                        hasIP = true;
                    }
                    ipAddr = ipAddr->Next;
                }

                if (!hasIP) {
                    std::wcout << L"  IP Address: Not connected\n";
                }

                if (adapter->DhcpEnabled) {
                    std::wcout << L"  DHCP Enabled: Yes\n";
                    if (adapter->DhcpServer.IpAddress.String[0] != '\0' &&
                        strcmp(adapter->DhcpServer.IpAddress.String, "0.0.0.0") != 0) {
                        std::wcout << L"  DHCP Server: " << adapter->DhcpServer.IpAddress.String << L"\n";
                    }
                }
                else {
                    std::wcout << L"  DHCP Enabled: No\n";
                }

                adapter = adapter->Next;
                if (adapter) std::wcout << L"\n";
            }

            if (adapterCount == 0) {
                std::wcout << L"No network adapters found.\n";
            }
        }
        else {
            std::wcout << L"Failed to retrieve network information.\n";
        }

        free(adapterInfo);
        std::wcout << L"\n";
    }

    static void DisplayBIOSInfo() {
        std::wcout << L"--- BIOS Information ---\n";

        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"HARDWARE\\DESCRIPTION\\System\\BIOS",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            wchar_t biosVendor[256];
            wchar_t biosVersion[256];
            wchar_t biosReleaseDate[256];
            wchar_t systemManufacturer[256];
            wchar_t systemProductName[256];
            DWORD size = sizeof(biosVendor);

            if (RegQueryValueEx(hKey, L"BIOSVendor", nullptr, nullptr,
                (LPBYTE)biosVendor, &size) == ERROR_SUCCESS) {
                std::wcout << L"Vendor: " << biosVendor << L"\n";
            }

            size = sizeof(biosVersion);
            if (RegQueryValueEx(hKey, L"BIOSVersion", nullptr, nullptr,
                (LPBYTE)biosVersion, &size) == ERROR_SUCCESS) {
                std::wcout << L"Version: " << biosVersion << L"\n";
            }

            size = sizeof(biosReleaseDate);
            if (RegQueryValueEx(hKey, L"BIOSReleaseDate", nullptr, nullptr,
                (LPBYTE)biosReleaseDate, &size) == ERROR_SUCCESS) {
                std::wcout << L"Release Date: " << biosReleaseDate << L"\n";
            }

            size = sizeof(systemManufacturer);
            if (RegQueryValueEx(hKey, L"SystemManufacturer", nullptr, nullptr,
                (LPBYTE)systemManufacturer, &size) == ERROR_SUCCESS) {
                std::wcout << L"System Manufacturer: " << systemManufacturer << L"\n";
            }

            size = sizeof(systemProductName);
            if (RegQueryValueEx(hKey, L"SystemProductName", nullptr, nullptr,
                (LPBYTE)systemProductName, &size) == ERROR_SUCCESS) {
                std::wcout << L"System Product Name: " << systemProductName << L"\n";
            }

            RegCloseKey(hKey);
        }
        else {
            std::wcout << L"BIOS information not available.\n";
        }
        std::wcout << L"\n";
    }

    static void DisplayUptime() {
        std::wcout << L"--- System Uptime ---\n";

        ULONGLONG uptimeMillis = GetTickCount64();
        DWORD days = (DWORD)(uptimeMillis / (1000 * 60 * 60 * 24));
        DWORD hours = (DWORD)((uptimeMillis % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60));
        DWORD minutes = (DWORD)((uptimeMillis % (1000 * 60 * 60)) / (1000 * 60));
        DWORD seconds = (DWORD)((uptimeMillis % (1000 * 60)) / 1000);

        std::wcout << L"System Uptime: "
            << days << L" days, "
            << hours << L" hours, "
            << minutes << L" minutes, "
            << seconds << L" seconds\n";

        std::wcout << L"\n";
    }

    static void DisplayProcessInfo() {
        std::wcout << L"--- Process Information ---\n";

        DWORD processID = GetCurrentProcessId();
        std::wcout << L"Current Process ID: " << processID << L"\n";

        HANDLE hProcess = GetCurrentProcess();
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            std::wcout << L"Working Set Size: " << pmc.WorkingSetSize / 1024 << L" KB\n";
            std::wcout << L"Peak Working Set Size: " << pmc.PeakWorkingSetSize / 1024 << L" KB\n";
            std::wcout << L"Private Bytes: " << pmc.PrivateUsage / 1024 << L" KB\n";
        }

        // Get number of running processes
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32W pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32W);

            int processCount = 0;
            if (Process32FirstW(hSnapshot, &pe32)) {
                do {
                    processCount++;
                } while (Process32NextW(hSnapshot, &pe32));
            }

            std::wcout << L"Total Running Processes: " << processCount << L"\n";
            CloseHandle(hSnapshot);
        }

        std::wcout << L"\n";
    }

    static void DisplayDisplayInfo() {
        std::wcout << L"--- Display Information ---\n";

        // Get number of monitors
        int monitors = GetSystemMetrics(SM_CMONITORS);
        std::wcout << L"Number of Monitors: " << monitors << L"\n";

        // Get DPI and scaling
        HDC hdc = GetDC(nullptr);
        int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(nullptr, hdc);

        // Get logical resolution (with scaling)
        int logicalWidth = GetSystemMetrics(SM_CXSCREEN);
        int logicalHeight = GetSystemMetrics(SM_CYSCREEN);

        // Get physical resolution via EnumDisplaySettings
        DEVMODE devMode;
        devMode.dmSize = sizeof(devMode);
        devMode.dmDriverExtra = 0;

        int physicalWidth = 0;
        int physicalHeight = 0;
        int refreshRate = 0;
        int colorDepth = 0;

        if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode)) {
            physicalWidth = devMode.dmPelsWidth;
            physicalHeight = devMode.dmPelsHeight;
            refreshRate = devMode.dmDisplayFrequency;
            colorDepth = devMode.dmBitsPerPel;
        }

        std::wcout << L"Physical Resolution: " << physicalWidth << L" x " << physicalHeight << L"\n";
        std::wcout << L"Logical Resolution: " << logicalWidth << L" x " << logicalHeight << L"\n";
        std::wcout << L"Color Depth: " << colorDepth << L" bits per pixel\n";
        std::wcout << L"DPI: " << dpiX << L" x " << dpiY << L"\n";
        std::wcout << L"Refresh Rate: " << refreshRate << L" Hz\n";

        // Calculate scale
        float scaleX = (float)logicalWidth / physicalWidth * 100.0f;
        float scaleY = (float)logicalHeight / physicalHeight * 100.0f;
        std::wcout << L"Scale: " << std::fixed << std::setprecision(0) << scaleX << L"%\n";

        std::wcout << L"\n";
    }

    static void DisplayPerformanceInfo() {
        std::wcout << L"--- Performance Information ---\n";

        // Get system performance information
        PERFORMANCE_INFORMATION perfInfo;
        perfInfo.cb = sizeof(perfInfo);

        if (GetPerformanceInfo(&perfInfo, sizeof(perfInfo))) {
            std::wcout << L"Commit Total: " << perfInfo.CommitTotal << L" pages\n";
            std::wcout << L"Commit Limit: " << perfInfo.CommitLimit << L" pages\n";
            std::wcout << L"Commit Peak: " << perfInfo.CommitPeak << L" pages\n";
            std::wcout << L"Physical Total: " << perfInfo.PhysicalTotal << L" pages\n";
            std::wcout << L"Physical Available: " << perfInfo.PhysicalAvailable << L" pages\n";
            std::wcout << L"System Cache: " << perfInfo.SystemCache << L" pages\n";
            std::wcout << L"Handle Count: " << perfInfo.HandleCount << L"\n";
            std::wcout << L"Process Count: " << perfInfo.ProcessCount << L"\n";
            std::wcout << L"Thread Count: " << perfInfo.ThreadCount << L"\n";
        }

        std::wcout << L"\n";
    }
};

int wmain(int argc, wchar_t* argv[]) {
    SetConsoleOutputCP(CP_UTF8);

    bool showHelp = false;
    bool minimal = false;

    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"/?") == 0 || wcscmp(argv[i], L"-h") == 0) {
            showHelp = true;
        }
        else if (wcscmp(argv[i], L"/min") == 0 || wcscmp(argv[i], L"-m") == 0) {
            minimal = true;
        }
    }

    if (showHelp) {
        std::wcout << L"System Information Utility\n";
        std::wcout << L"Usage: SystemInfo.exe [options]\n";
        std::wcout << L"Options:\n";
        std::wcout << L"  /?, -h      Show this help message\n";
        std::wcout << L"  /min, -m    Minimal output (OS, CPU and Memory only)\n";
        std::wcout << L"\n";
        return 0;
    }

    if (minimal) {
        std::wcout << L"=== MINIMAL SYSTEM INFORMATION ===\n\n";
        SystemInfo::DisplayOSInfo();
        SystemInfo::DisplayCPUInfo();
        SystemInfo::DisplayMemoryInfo();
    }
    else {
        SystemInfo::DisplayAll();
    }

    std::wcout << L"\nPress Enter to exit...";
    std::wcin.get();

    return 0;
}