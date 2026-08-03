// ================================================================
// CHESS EXECUTOR - EXACT COPY FROM IMAGE
// ================================================================
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifndef EM_SETBKGNDCOLOR
#define EM_SETBKGNDCOLOR (WM_USER + 1)
#endif

#define ID_SCRIPT_EDITOR 101
#define ID_CONSOLE 102
#define ID_STATUS 103
#define ID_VERSION 104

HWND hMain, hStatus, hScriptEdit, hConsole, hConsoleInput;
HFONT hFont;

// ================================================================
// 1. إنشاء عناصر الواجهة (مطابقة للصورة)
// ================================================================

void CreateUI(HWND hwnd) {
    // خلفية النافذة
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(20, 20, 30)));

    // ===== شريط العنوان =====
    CreateWindowA("STATIC", "  ♟ CHESS",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 900, 28, hwnd, (HMENU)100, GetModuleHandle(NULL), NULL);

    // ===== القائمة الجانبية =====
    int y = 35;
    const char* menuItems[] = { "HOME", "Console" };
    for (int i = 0; i < 2; i++) {
        CreateWindowA("BUTTON", menuItems[i],
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
            10, y, 100, 30, hwnd, (HMENU)(200 + i), NULL, NULL);
        y += 40;
    }

    // ===== زر ATTACH =====
    CreateWindowA("BUTTON", "ATTACH", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        10, 130, 100, 30, hwnd, (HMENU)400, NULL, NULL);

    // ===== زر INJECT =====
    CreateWindowA("BUTTON", "INJECT", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        10, 170, 100, 30, hwnd, (HMENU)401, NULL, NULL);

    // ===== Script Editor (محرر النصوص مع أرقام الأسطر) =====
    hScriptEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", 
        "1 -- Welcome to Chess\n2 -- Write your Lua script here...",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        125, 35, 400, 420, hwnd, (HMENU)ID_SCRIPT_EDITOR, GetModuleHandle(NULL), NULL);
    SendMessageA(hScriptEdit, EM_SETBKGNDCOLOR, 0, RGB(10, 10, 20));
    SendMessageA(hScriptEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(8, 8));

    // ===== Console (نافذة الكونسول) =====
    hConsole = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", 
        "[14:35:21] Welcome to Chess!\n[14:35:21] Waiting for Roblox process...\n[14:35:23] Successfully attached to Roblox (PID: 1234)\n[14:35:23] Executor ready.",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
        540, 35, 330, 340, hwnd, (HMENU)ID_CONSOLE, GetModuleHandle(NULL), NULL);
    SendMessageA(hConsole, EM_SETBKGNDCOLOR, 0, RGB(10, 10, 20));

    // ===== Console Input =====
    hConsoleInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Console input...",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        540, 380, 330, 28, hwnd, (HMENU)303, GetModuleHandle(NULL), NULL);
    SendMessageA(hConsoleInput, EM_SETBKGNDCOLOR, 0, RGB(15, 15, 25));

    // ===== أزرار EXECUTE, CLEAR =====
    CreateWindowA("BUTTON", "EXECUTE", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        125, 465, 100, 30, hwnd, (HMENU)300, NULL, NULL);
    CreateWindowA("BUTTON", "CLEAR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        235, 465, 100, 30, hwnd, (HMENU)301, NULL, NULL);

    // ===== شريط الحالة =====
    hStatus = CreateWindowA("STATIC", "● STATUS: Ready  |  ATTACHED: No",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        5, 520, 850, 25, hwnd, (HMENU)ID_STATUS, GetModuleHandle(NULL), NULL);

    // ===== رقم الإصدار =====
    CreateWindowA("STATIC", "v1.0.0",
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        860, 520, 40, 25, hwnd, (HMENU)ID_VERSION, GetModuleHandle(NULL), NULL);
}

// ================================================================
// 2. معالجة الأحداث
// ================================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        CreateUI(hwnd);
        break;
    }

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(20, 20, 30));
        SetTextColor(hdc, RGB(0, 255, 170));
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);

        // أزرار القائمة الجانبية
        if (id >= 200 && id < 202) {
            char buf[64];
            GetWindowTextA((HWND)lParam, buf, 64);
            std::string status = "● STATUS: ";
            status += buf;
            SetWindowTextA(hStatus, status.c_str());
        }

        // ATTACH
        if (id == 400) {
            SetWindowTextA(hStatus, "● STATUS: Attaching to Roblox...");
            char console[4096];
            GetWindowTextA(hConsole, console, 4096);
            std::string newMsg = std::string(console) + "\n[14:35:24] Attaching to Roblox...";
            SetWindowTextA(hConsole, newMsg.c_str());
        }

        // INJECT
        if (id == 401) {
            SetWindowTextA(hStatus, "● STATUS: Injected  |  ATTACHED: Yes");
            char console[4096];
            GetWindowTextA(hConsole, console, 4096);
            std::string newMsg = std::string(console) + "\n[14:35:25] Injected successfully!";
            SetWindowTextA(hConsole, newMsg.c_str());
        }

        // EXECUTE
        if (id == 300) {
            char script[2048];
            GetWindowTextA(hScriptEdit, script, 2048);
            if (strlen(script) > 10) {
                SetWindowTextA(hStatus, "● STATUS: Script Executed  |  ATTACHED: Yes");
                char console[4096];
                GetWindowTextA(hConsole, console, 4096);
                std::string newMsg = std::string(console) + "\n[14:35:26] Script executed!";
                SetWindowTextA(hConsole, newMsg.c_str());
            } else {
                SetWindowTextA(hStatus, "● STATUS: Please write a valid script");
            }
        }

        // CLEAR
        if (id == 301) {
            SetWindowTextA(hScriptEdit, "");
            SetWindowTextA(hStatus, "● STATUS: Cleared  |  ATTACHED: Yes");
        }

        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ================================================================
// 3. نقطة الدخول
// ================================================================

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES };
    InitCommonControlsEx(&icex);

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(20, 20, 30));
    wc.lpszClassName = "ChessExecutorClass";
    RegisterClassA(&wc);

    hMain = CreateWindowExA(
        0, 
        "ChessExecutorClass", 
        "♟ CHESS",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        920, 
        580, 
        NULL, 
        NULL, 
        hInst, 
        NULL
    );

    ShowWindow(hMain, nCmdShow);
    UpdateWindow(hMain);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
