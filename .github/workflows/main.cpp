// ================================================================
// SHADOW EXECUTOR V99 - FINAL MAIN
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

bool InjectDLL() {
    if (!IsRobloxOpen()) {
        MessageBoxA(NULL, "Roblox is not open!", "Error", MB_OK);
        SetWindowTextA(hStatus, "⚠ Roblox not running!");
        return false;
    }

    HMODULE hDll = LoadLibraryA("Executor.dll");
    if (!hDll) {
        MessageBoxA(NULL, "Failed to load Executor.dll!", "Error", MB_OK);
        SetWindowTextA(hStatus, "❌ DLL not found!");
        return false;
    }

    typedef bool (*StartFunction)();
    StartFunction Start = (StartFunction)GetProcAddress(hDll, "StartExecutor");
    if (Start) {
        Start();
        SetWindowTextA(hStatus, "✅ Injected!");
        g_CurrentUsername = "Bound_User";
        SetWindowTextA(hUsernameLabel, ("Bound to: " + g_CurrentUsername).c_str());
        return true;
    }

    SetWindowTextA(hStatus, "❌ Injection failed!");
    return false;
}

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
    }
    ShowWindow(hEdit, SW_SHOW);
    SetFocus(hEdit);
}

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
        if (LOWORD(wParam) == 2) { InjectDLL(); }
        if (LOWORD(wParam) == 3) {
            SaveCurrentTabContent();
            HANDLE hPipe = CreateFileA("\\\\.\\pipe\\ShadowExecutorPipe", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (hPipe != INVALID_HANDLE_VALUE) {
                std::string script = g_TabScripts[g_CurrentTab];
                DWORD bytesWritten;
                WriteFile(hPipe, script.c_str(), script.length(), &bytesWritten, NULL);
                CloseHandle(hPipe);
                SetWindowTextA(hStatus, "✅ Executed!");
            } else {
                SetWindowTextA(hStatus, "❌ Inject first!");
            }
        }
        if (LOWORD(wParam) == 8) {
            for (auto& pair : g_TabScripts) {
                HANDLE hPipe = CreateFileA("\\\\.\\pipe\\ShadowExecutorPipe", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
                if (hPipe != INVALID_HANDLE_VALUE) {
                    std::string script = pair.second;
                    DWORD bytesWritten;
                    WriteFile(hPipe, script.c_str(), script.length(), &bytesWritten, NULL);
                    CloseHandle(hPipe);
                    Sleep(200);
                }
            }
            SetWindowTextA(hStatus, "✅ All Executed!");
        }
        break;
    }
    case WM_DESTROY: PostQuitMessage(0); break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

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