// ================================================================
// CHESS EXECUTOR - ENGLISH VERSION (EXACT COPY)
// ================================================================
#include <windows.h>
#include <commctrl.h>
#include <string>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifndef EM_SETBKGNDCOLOR
#define EM_SETBKGNDCOLOR (WM_USER + 1)
#endif

// ================================================================
// COLORS (Purple Theme)
// ================================================================
#define COLOR_BG RGB(30, 20, 45)
#define COLOR_EDITOR RGB(20, 12, 32)
#define COLOR_CONSOLE RGB(18, 10, 30)
#define COLOR_TEXT RGB(220, 180, 255)
#define COLOR_TEXT_BRIGHT RGB(240, 210, 255)

HWND hMain, hStatus, hScriptEdit, hConsole, hConsoleInput;
HFONT hFontTitle, hFontNormal, hFontCode, hFontConsole;

// ================================================================
// CREATE UI (FULLY ENGLISH)
// ================================================================
void CreateUI(HWND hwnd) {
    // Window background
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(COLOR_BG));

    // Fonts
    hFontTitle = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    hFontNormal = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    hFontCode = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
    hFontConsole = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Courier New");

    // ===== TITLE BAR =====
    HWND hTitle = CreateWindowW(L"STATIC", L"  ♟ CHESS",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 900, 30, hwnd, (HMENU)100, GetModuleHandle(NULL), NULL);
    SendMessageW(hTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

    // ===== SIDEBAR =====
    int y = 40;
    const wchar_t* menuItems[] = { L"HOME", L"Console" };
    for (int i = 0; i < 2; i++) {
        HWND btn = CreateWindowW(L"BUTTON", menuItems[i],
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
            10, y, 100, 30, hwnd, (HMENU)(200 + i), GetModuleHandle(NULL), NULL);
        SendMessageW(btn, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
        y += 40;
    }

    // ===== ATTACH BUTTON =====
    HWND hAttach = CreateWindowW(L"BUTTON", L"ATTACH",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        10, 130, 100, 30, hwnd, (HMENU)400, GetModuleHandle(NULL), NULL);
    SendMessageW(hAttach, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    // ===== INJECT BUTTON =====
    HWND hInject = CreateWindowW(L"BUTTON", L"INJECT",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        10, 170, 100, 30, hwnd, (HMENU)401, GetModuleHandle(NULL), NULL);
    SendMessageW(hInject, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    // ===== SCRIPT EDITOR =====
    hScriptEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT",
        L"1. -- Welcome to Chess\n2. -- Write your Lua script here...",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        125, 40, 400, 400, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);
    SendMessageW(hScriptEdit, EM_SETBKGNDCOLOR, 0, COLOR_EDITOR);
    SendMessageW(hScriptEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(8, 8));
    SendMessageW(hScriptEdit, WM_SETFONT, (WPARAM)hFontCode, TRUE);

    // ===== CONSOLE =====
    hConsole = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT",
        L"[14:35:21] Welcome to Chess!\n[14:35:21] Waiting for Roblox process...\n[14:35:23] Successfully attached to Roblox (PID: 1234)\n[14:35:23] Executor ready.",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
        540, 40, 330, 340, hwnd, (HMENU)102, GetModuleHandle(NULL), NULL);
    SendMessageW(hConsole, EM_SETBKGNDCOLOR, 0, COLOR_CONSOLE);
    SendMessageW(hConsole, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(8, 8));
    SendMessageW(hConsole, WM_SETFONT, (WPARAM)hFontConsole, TRUE);

    // ===== CONSOLE INPUT =====
    hConsoleInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Console input...",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        540, 385, 330, 28, hwnd, (HMENU)303, GetModuleHandle(NULL), NULL);
    SendMessageW(hConsoleInput, EM_SETBKGNDCOLOR, 0, COLOR_EDITOR);
    SendMessageW(hConsoleInput, WM_SETFONT, (WPARAM)hFontConsole, TRUE);

    // ===== BUTTONS =====
    const wchar_t* btns[] = { L"EXECUTE", L"CLEAR", L"ATTACH", L"INJECT" };
    int bx = 125;
    for (int i = 0; i < 4; i++) {
        HWND btn = CreateWindowW(L"BUTTON", btns[i],
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
            bx, 450, 90, 30, hwnd, (HMENU)(300 + i), GetModuleHandle(NULL), NULL);
        SendMessageW(btn, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
        bx += 100;
    }

    // ===== STATUS BAR =====
    hStatus = CreateWindowW(L"STATIC", L"STATUS: Ready  |  ATTACHED: No",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        5, 500, 800, 25, hwnd, (HMENU)500, GetModuleHandle(NULL), NULL);
    SendMessageW(hStatus, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    // ===== VERSION =====
    CreateWindowW(L"STATIC", L"v1.0.0",
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        860, 500, 50, 25, hwnd, (HMENU)501, GetModuleHandle(NULL), NULL);
}

// ================================================================
// WINDOW PROCEDURE
// ================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateUI(hwnd);
        break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, COLOR_BG);
        SetTextColor(hdc, COLOR_TEXT);
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == 400) { // ATTACH
            SetWindowTextW(hStatus, L"STATUS: Attached  |  ATTACHED: Yes");
            wchar_t console[4096];
            GetWindowTextW(hConsole, console, 4096);
            std::wstring newMsg = std::wstring(console) + L"\n[14:35:24] Attached to Roblox!";
            SetWindowTextW(hConsole, newMsg.c_str());
        }
        if (id == 401) { // INJECT
            SetWindowTextW(hStatus, L"STATUS: Injected  |  ATTACHED: Yes");
            wchar_t console[4096];
            GetWindowTextW(hConsole, console, 4096);
            std::wstring newMsg = std::wstring(console) + L"\n[14:35:25] Injected successfully!";
            SetWindowTextW(hConsole, newMsg.c_str());
        }
        if (id == 300) { // EXECUTE
            wchar_t script[4096];
            GetWindowTextW(hScriptEdit, script, 4096);
            if (wcslen(script) > 10) {
                SetWindowTextW(hStatus, L"STATUS: Executed  |  ATTACHED: Yes");
                wchar_t console[4096];
                GetWindowTextW(hConsole, console, 4096);
                std::wstring newMsg = std::wstring(console) + L"\n[14:35:26] Script executed!";
                SetWindowTextW(hConsole, newMsg.c_str());
            }
        }
        if (id == 301) { // CLEAR
            SetWindowTextW(hScriptEdit, L"");
            SetWindowTextW(hStatus, L"STATUS: Cleared  |  ATTACHED: Yes");
        }
        break;
    }

    case WM_DESTROY:
        DeleteObject(hFontTitle);
        DeleteObject(hFontNormal);
        DeleteObject(hFontCode);
        DeleteObject(hFontConsole);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ================================================================
// ENTRY POINT
// ================================================================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES };
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_BG);
    wc.lpszClassName = L"ChessExecutorClass";
    RegisterClassExW(&wc);

    hMain = CreateWindowExW(0, L"ChessExecutorClass", L"CHESS",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 920, 580,
        NULL, NULL, hInst, NULL);

    ShowWindow(hMain, nCmdShow);
    UpdateWindow(hMain);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
