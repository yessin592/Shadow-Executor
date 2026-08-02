// ================================================================
// SHADOW EXECUTOR V99 - FIXED MAIN (NO UNICODE ISSUES)
// ================================================================

#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <fstream>
#include <commctrl.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "version.lib")

#ifndef EM_SETBKGNDCOLOR
#define EM_SETBKGNDCOLOR (WM_USER + 1)
#endif

HWND hMain, hStatus, hUsernameLabel, hTabControl;
std::map<int, std::string> g_TabScripts;
std::map<int, std::string> g_TabNames;
int g_CurrentTab = 1;
int g_NextTabId = 1;
std::string g_CurrentUsername = "Not bound";

bool IsRobloxOpen() {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 entry = { sizeof(entry) };
        if (Process32First(snap, &entry)) {
            do {
                if (_stricmp(entry.szExeFile, "RobloxPlayerBeta.exe") == 0) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snap, &entry));
        }
        CloseHandle(snap);
    }
    return (pid != 0);
}

void AddNewTab(const std::string& name = "New") {
    // ... (باقي الكود كما هو)
}

// باقي الكود (WndProc, WinMain) كما في النسخة السابقة بعد إزالة WideCharToMultiByte.
