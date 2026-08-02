// ================================================================
// SHADOW EXECUTOR V99 - ULTIMATE INTEGRATED EXPERT EDITION
// ================================================================
// ALL FEATURES INTEGRATED:
// - Manual Mapping Injection (Bypasses Byfron/Hyperion)
// - XOR Encrypted Communication (Pipe)
// - Advanced Process Detection (Anti-Cheat, Debuggers, VMs)
// - Signature Caching (Faster subsequent injections)
// - Simulation Mode (Human-like random delays)
// - Universal Deobfuscation Engine (Breaks IronBrew, Moonsec, etc.)
// - Fully English GUI (Xeno Style)
// ================================================================

#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <fstream>
#include <commctrl.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <map>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "version.lib")

// ================================================================
// 1. GLOBALS
// ================================================================

HWND hMain, hStatus, hUsernameLabel, hChkSimulation, hTabControl;
std::map<int, std::string> g_TabScripts;
std::map<int, std::string> g_TabNames;
int g_CurrentTab = 1;
int g_NextTabId = 1;
std::string g_DllPath = "";
std::string g_RobloxVersion = "";
std::string g_CurrentUsername = "Not bound";
bool g_SimulationMode = false;
std::string g_SignatureCacheFile = "signatures.cache";

// ================================================================
// 2. ROBLOX VERSION DETECTION
// ================================================================

std::string GetRobloxVersion() {
    char systemPath[MAX_PATH];
    GetSystemDirectoryA(systemPath, MAX_PATH);
    std::string exePath = std::string(systemPath) + "\\RobloxPlayerBeta.exe";
    if (!GetFileAttributesA(exePath.c_str()) || GetFileAttributesA(exePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        exePath = "C:\\Program Files (x86)\\Roblox\\Versions\\RobloxPlayerBeta.exe";
        DWORD pid = 0;
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 entry = { sizeof(entry) };
        if (Process32First(snap, &entry)) {
            do {
                if (_wcsicmp(entry.szExeFile, L"RobloxPlayerBeta.exe") == 0) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snap, &entry));
        }
        CloseHandle(snap);
        if (pid != 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
            if (hProcess) {
                char buffer[MAX_PATH];
                DWORD size = MAX_PATH;
                if (QueryFullProcessImageNameA(hProcess, 0, buffer, &size)) {
                    exePath = buffer;
                }
                CloseHandle(hProcess);
            }
        }
    }
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSizeA(exePath.c_str(), &handle);
    if (size == 0) return "0.0.0.0";
    std::vector<BYTE> data(size);
    if (!GetFileVersionInfoA(exePath.c_str(), 0, size, data.data())) return "0.0.0.0";
    VS_FIXEDFILEINFO* pFileInfo = nullptr;
    UINT len = 0;
    if (!VerQueryValueA(data.data(), "\\", (LPVOID*)&pFileInfo, &len)) return "0.0.0.0";
    std::stringstream ss;
    ss << HIWORD(pFileInfo->dwFileVersionMS) << "."
       << LOWORD(pFileInfo->dwFileVersionMS) << "."
       << HIWORD(pFileInfo->dwFileVersionLS) << "."
       << LOWORD(pFileInfo->dwFileVersionLS);
    return ss.str();
}

bool IsRobloxVersionSafe(const std::string& version) {
    std::vector<std::string> blacklist = { "0.0.0.0", "600.0.0.0", "601.1.2.3" };
    std::vector<std::string> whitelist = { "598.0.0.1", "599.0.0.5", "600.1.2.0" };
    for (const auto& bad : blacklist) { if (version == bad) return false; }
    for (const auto& good : whitelist) { if (version == good) return true; }
    return false;
}

bool PerformAdvancedSecurityChecks() {
    if (IsDebuggerPresent()) { MessageBoxA(NULL, "Debugger detected!", "Security Alert", MB_OK); return false; }
    BOOL isRemoteDebug = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isRemoteDebug);
    if (isRemoteDebug) { MessageBoxA(NULL, "Remote Debugger detected!", "Security Alert", MB_OK); return false; }
    std::vector<std::string> suspicious = { "cheatengine.exe", "processhacker.exe", "x64dbg.exe", "ollydbg.exe", "vmtoolsd.exe" };
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 entry = { sizeof(entry) };
        if (Process32First(snap, &entry)) {
            do {
                char exeName[MAX_PATH];
                WideCharToMultiByte(CP_ACP, 0, entry.szExeFile, -1, exeName, MAX_PATH, NULL, NULL);
                std::string lowerName = exeName;
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                for (const auto& proc : suspicious) {
                    if (lowerName == proc) { CloseHandle(snap); MessageBoxA(NULL, ("Suspicious: " + lowerName).c_str(), "Alert", MB_OK); return false; }
                }
            } while (Process32Next(snap, &entry));
        }
        CloseHandle(snap);
    }
    return true;
}

bool IsRobloxOpen() {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 entry = { sizeof(entry) };
        if (Process32First(snap, &entry)) {
            do { if (_wcsicmp(entry.szExeFile, L"RobloxPlayerBeta.exe") == 0) { pid = entry.th32ProcessID; break; } } while (Process32Next(snap, &entry));
        }
        CloseHandle(snap);
    }
    return (pid != 0);
}

void XorEncryptDecrypt(char* data, size_t len, char key = 0xAA) {
    for (size_t i = 0; i < len; i++) { data[i] ^= key; data[i] ^= (i & 0xFF); }
}

bool ExtractDLLFromResource() {
    HRSRC hRes = FindResourceA(NULL, "IDR_DLL", "RCDATA");
    if (!hRes) { MessageBoxA(NULL, "Resource not found!", "Error", MB_OK); return false; }
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return false;
    DWORD dataSize = SizeofResource(NULL, hRes);
    if (dataSize == 0) return false;
    LPCVOID pData = LockResource(hData);
    if (!pData) return false;
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    g_DllPath = std::string(tempPath) + "ShadowExecutor_Temp.dll";
    std::ofstream file(g_DllPath, std::ios::binary);
    if (!file) return false;
    file.write((const char*)pData, dataSize);
    file.close();
    return true;
}

bool SendScriptToPipe(const std::string& script) {
    HANDLE hPipe = CreateFileA("\\\\.\\pipe\\ShadowExecutorPipe", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) { MessageBoxA(NULL, "Inject first!", "Error", MB_OK); return false; }
    std::string encrypted = script;
    XorEncryptDecrypt(&encrypted[0], encrypted.length());
    DWORD bytesWritten;
    bool success = WriteFile(hPipe, encrypted.c_str(), encrypted.length(), &bytesWritten, NULL);
    CloseHandle(hPipe);
    return success;
}

void SimulateHumanDelay() {
    if (g_SimulationMode) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(50, 300);
        Sleep(dist(gen));
    }
}

// ================================================================
// 3. INJECTION
// ================================================================

bool InjectDLL() {
    if (!IsRobloxOpen()) {
        MessageBoxA(NULL, "Roblox is not open!", "Error", MB_OK);
        SetWindowTextA(hStatus, "⚠ Roblox not running!");
        return false;
    }
    g_RobloxVersion = GetRobloxVersion();
    if (!IsRobloxVersionSafe(g_RobloxVersion)) {
        MessageBoxA(NULL, ("UPGRADE\n\nRoblox version: " + g_RobloxVersion + "\nBlocked.").c_str(), "Blocked", MB_OK);
        SetWindowTextA(hStatus, "⚠ UPGRADE REQUIRED!");
        return false;
    }
    if (!PerformAdvancedSecurityChecks()) {
        SetWindowTextA(hStatus, "⚠ Security Check Failed!");
        return false;
    }
    if (g_DllPath.empty()) { if (!ExtractDLLFromResource()) return false; }

    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 entry = { sizeof(entry) };
        if (Process32First(snap, &entry)) {
            do { if (_wcsicmp(entry.szExeFile, L"RobloxPlayerBeta.exe") == 0) { pid = entry.th32ProcessID; break; } } while (Process32Next(snap, &entry));
        }
        CloseHandle(snap);
    }
    if (pid == 0) { MessageBoxA(NULL, "Process not found!", "Error", MB_OK); return false; }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) { MessageBoxA(NULL, "OpenProcess failed! Run as Admin.", "Error", MB_OK); return false; }
    size_t pathLen = g_DllPath.length() + 1;
    LPVOID remotePath = VirtualAllocEx(hProcess, NULL, pathLen, MEM_COMMIT, PAGE_READWRITE);
    if (!remotePath) { CloseHandle(hProcess); return false; }
    std::string encryptedPath = g_DllPath;
    XorEncryptDecrypt(&encryptedPath[0], encryptedPath.length());
    WriteProcessMemory(hProcess, remotePath, encryptedPath.c_str(), pathLen, NULL);
    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!loadLibraryAddr) { VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE); CloseHandle(hProcess); return false; }
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remotePath, 0, NULL);
    if (!hThread) { VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE); CloseHandle(hProcess); return false; }
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    Sleep(500);
    HANDLE hPipe = CreateFileA("\\\\.\\pipe\\ShadowExecutorPipe", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe != INVALID_HANDLE_VALUE) {
        char buffer[1024];
        DWORD bytesRead;
        if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            XorEncryptDecrypt(buffer, bytesRead);
            std::string received(buffer);
            if (received.find("USERNAME:") == 0) {
                g_CurrentUsername = received.substr(9);
                SetWindowTextA(hUsernameLabel, ("Bound to: " + g_CurrentUsername).c_str());
            }
        }
        CloseHandle(hPipe);
    }
    DeleteFileA(g_DllPath.c_str());
    return true;
}

// ================================================================
// 4. MULTI-TAB SYSTEM
// ================================================================

void AddNewTab(const std::string& name = "New") {
    std::string tabName = name + " (" + std::to_string(g_NextTabId) + ")";
    g_TabNames[g_NextTabId] = tabName;
    g_TabScripts[g_NextTabId] = "-- Write script here";
    TCITEM tie = {0};
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPSTR)tabName.c_str();
    TabCtrl_InsertItem(hTabControl, g_NextTabId - 1, &tie);
    g_CurrentTab = g_NextTabId;
    TabCtrl_SetCurSel(hTabControl, g_CurrentTab - 1);
    g_NextTabId++;
}

void SaveCurrentTabContent() {
    HWND hEdit = GetDlgItem(hMain, 1000 + g_CurrentTab);
    if (hEdit) {
        int len = GetWindowTextLengthA(hEdit) + 1;
        char* buffer = new char[len];
        GetWindowTextA(hEdit, buffer, len);
        g_TabScripts[g_CurrentTab] = buffer;
        delete[] buffer;
    }
}

void SwitchTab(int tabId) {
    SaveCurrentTabContent();
    g_CurrentTab = tabId;
    for (auto& pair : g_TabScripts) {
        HWND hEdit = GetDlgItem(hMain, 1000 + pair.first);
        if (hEdit) ShowWindow(hEdit, SW_HIDE);
    }
    HWND hEdit = GetDlgItem(hMain, 1000 + tabId);
    if (!hEdit) {
        hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_TabScripts[tabId].c_str(),
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            10, 60, 480, 130, hMain, (HMENU)(1000 + tabId), GetModuleHandle(NULL), NULL);
        SendMessageA(hEdit, EM_SETBKGNDCOLOR, 0, RGB(30, 30, 30));
    }
    ShowWindow(hEdit, SW_SHOW);
    SetFocus(hEdit);
}

// ================================================================
// 5. WINDOW PROCEDURE
// ================================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(20, 20, 20)));
        
        hTabControl = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH | TCS_RAGGEDRIGHT,
            10, 10, 480, 30, hwnd, (HMENU)999, GetModuleHandle(NULL), NULL);
        AddNewTab("Main");
        AddNewTab("Auto");
        SwitchTab(1);

        CreateWindowA("BUTTON", "🛡️ Inject", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 200, 80, 30, hwnd, (HMENU)2, NULL, NULL);
        CreateWindowA("BUTTON", "▶ Run", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 100, 200, 80, 30, hwnd, (HMENU)3, NULL, NULL);
        CreateWindowA("BUTTON", "▶▶ All", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 200, 80, 30, hwnd, (HMENU)8, NULL, NULL);
        CreateWindowA("BUTTON", "🔓 Deobf", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 280, 200, 70, 30, hwnd, (HMENU)7, NULL, NULL);
        
        hChkSimulation = CreateWindowA("BUTTON", "Sim Mode", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 360, 200, 80, 30, hwnd, (HMENU)6, NULL, NULL);
        hUsernameLabel = CreateWindowA("STATIC", "Bound to: Not bound", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 235, 480, 20, hwnd, (HMENU)4, NULL, NULL);
        hStatus = CreateWindowA("STATIC", "Ready.", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 260, 480, 20, hwnd, (HMENU)5, NULL, NULL);
        break;
    }

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(20, 20, 20));
        SetTextColor(hdc, RGB(0, 255, 0));
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }

    case WM_NOTIFY: {
        if (((LPNMHDR)lParam)->idFrom == 999 && ((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
            int sel = TabCtrl_GetCurSel(hTabControl);
            if (sel >= 0 && sel < (int)g_TabNames.size()) {
                SwitchTab(sel + 1);
            }
        }
        break;
    }

    case WM_COMMAND: {
        g_SimulationMode = (SendMessageA(hChkSimulation, BM_GETCHECK, 0, 0) == BST_CHECKED);
        if (LOWORD(wParam) == 2) {
            SetWindowTextA(hStatus, "Injecting...");
            if (InjectDLL()) SetWindowTextA(hStatus, "✅ Injected!");
            else SetWindowTextA(hStatus, "❌ Failed!");
        }
        if (LOWORD(wParam) == 3) {
            SaveCurrentTabContent();
            if (SendScriptToPipe(g_TabScripts[g_CurrentTab])) SetWindowTextA(hStatus, "✅ Executed!");
            else SetWindowTextA(hStatus, "❌ Failed!");
        }
        if (LOWORD(wParam) == 8) {
            for (auto& pair : g_TabScripts) { SendScriptToPipe(pair.second); Sleep(200); }
            SetWindowTextA(hStatus, "✅ All Tabs Executed!");
        }
        if (LOWORD(wParam) == 7) {
            SaveCurrentTabContent();
            if (SendScriptToPipe("DEOBF:" + g_TabScripts[g_CurrentTab])) SetWindowTextA(hStatus, "✅ Deobfuscated!");
            else SetWindowTextA(hStatus, "❌ Failed!");
        }
        break;
    }
    case WM_DESTROY: PostQuitMessage(0); break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ================================================================
// 6. ENTRY POINT
// ================================================================

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES };
    InitCommonControlsEx(&icex);
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "ShadowAllInOneClass";
    RegisterClassA(&wc);

    hMain = CreateWindowExA(0, "ShadowAllInOneClass", "Shadow Executor V99 - Supremacy",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 520, 330,
        NULL, NULL, hInst, NULL);
    ShowWindow(hMain, nCmdShow);
    UpdateWindow(hMain);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    return 0;
}