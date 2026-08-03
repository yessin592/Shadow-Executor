// ================================================================
// CHESS EXECUTOR - ULTIMATE ENHANCED UI
// ================================================================
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <map>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifndef EM_SETBKGNDCOLOR
#define EM_SETBKGNDCOLOR (WM_USER + 1)
#endif

// ================================================================
// 1. إعدادات التصميم
// ================================================================
#define APP_NAME L"♟ CHESS EXECUTOR"
#define APP_VERSION L"v2.1.0"
#define WINDOW_WIDTH 1050
#define WINDOW_HEIGHT 700

// الألوان الأساسية
#define COLOR_BG_DARK RGB(16, 16, 24)
#define COLOR_BG_PANEL RGB(22, 22, 34)
#define COLOR_BG_EDITOR RGB(10, 10, 18)
#define COLOR_BG_CONSOLE RGB(8, 8, 16)
#define COLOR_TEXT_MAIN RGB(0, 255, 180)
#define COLOR_TEXT_SUB RGB(150, 200, 255)
#define COLOR_BTN_HOVER RGB(40, 40, 60)
#define COLOR_BTN_ACTIVE RGB(0, 200, 150)

// عناصر التحكم
HWND g_hMain, g_hStatus, g_hScriptEdit, g_hConsole, g_hConsoleInput, g_hRecentList;
HWND g_hBtnClose, g_hBtnMinimize, g_hBtnMaximize;
HFONT g_hFontTitle, g_hFontNormal, g_hFontCode, g_hFontStatus;
bool g_isMaximized = false;
RECT g_oldRect = {0};

// ================================================================
// 2. دوال مساعدة للواجهة
// ================================================================
void SetControlFont(HWND hWnd, HFONT hFont) {
    SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
}

HWND CreateStyledButton(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id, HFONT font) {
    HWND btn = CreateWindowW(L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        x, y, w, h, parent, (HMENU)id, GetModuleHandle(NULL), NULL);
    if (font) SetControlFont(btn, font);
    return btn;
}

// ================================================================
// 3. شريط العنوان المخصص
// ================================================================
void CreateCustomTitleBar(HWND parent) {
    // خلفية شريط العنوان
    HWND hTitleBg = CreateWindowW(L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        0, 0, WINDOW_WIDTH, 32, parent, (HMENU)100, GetModuleHandle(NULL), NULL);
    SetClassLongPtr(hTitleBg, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(12, 12, 20)));

    // عنوان التطبيق (يسار)
    HWND hTitle = CreateWindowW(L"STATIC", APP_NAME,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        12, 6, 300, 22, parent, (HMENU)101, GetModuleHandle(NULL), NULL);
    SetControlFont(hTitle, g_hFontTitle);

    // أزرار التحكم (يمين)
    int btnSize = 28;
    int btnY = 2;
    int btnX = WINDOW_WIDTH - 90;
    
    // زر التصغير
    g_hBtnMinimize = CreateWindowW(L"BUTTON", L"─",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        btnX, btnY, btnSize, btnSize, parent, (HMENU)200, GetModuleHandle(NULL), NULL);
    SetControlFont(g_hBtnMinimize, g_hFontTitle);

    // زر التكبير
    g_hBtnMaximize = CreateWindowW(L"BUTTON", L"☐",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        btnX + btnSize, btnY, btnSize, btnSize, parent, (HMENU)201, GetModuleHandle(NULL), NULL);
    SetControlFont(g_hBtnMaximize, g_hFontTitle);

    // زر الإغلاق
    g_hBtnClose = CreateWindowW(L"BUTTON", L"✕",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        btnX + (2 * btnSize), btnY, btnSize, btnSize, parent, (HMENU)202, GetModuleHandle(NULL), NULL);
    SetControlFont(g_hBtnClose, g_hFontTitle);
}

// ================================================================
// 4. إنشاء الواجهة الكاملة (المحسّنة)
// ================================================================
void CreateUI(HWND hwnd) {
    // إنشاء الخطوط
    g_hFontTitle = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_hFontNormal = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_hFontCode = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Cascadia Code");
    g_hFontStatus = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");

    // خلفية النافذة
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(COLOR_BG_DARK));

    // 1. شريط العنوان المخصص
    CreateCustomTitleBar(hwnd);

    // 2. اللوحة الجانبية (يسار)
    int panelX = 10, panelW = 130, panelY = 40;
    HWND hPanel = CreateWindowW(L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        panelX, panelY, panelW, WINDOW_HEIGHT - panelY - 60, hwnd, (HMENU)102, GetModuleHandle(NULL), NULL);
    SetClassLongPtr(hPanel, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(COLOR_BG_PANEL));

    // أزرار القائمة الجانبية
    const wchar_t* menuItems[] = { L"🏠 HOME", L"📋 CONSOLE", L"📂 SCRIPTS" };
    int y = panelY + 15;
    for (int i = 0; i < 3; i++) {
        HWND btn = CreateStyledButton(hwnd, menuItems[i], panelX + 10, y, panelW - 20, 34, 300 + i, g_hFontNormal);
        y += 42;
    }

    // أزرار الإجراءات الرئيسية (Attach, Inject)
    y += 20;
    const wchar_t* actionBtns[] = { L"🔗 ATTACH", L"💉 INJECT" };
    for (int i = 0; i < 2; i++) {
        HWND btn = CreateStyledButton(hwnd, actionBtns[i], panelX + 10, y, panelW - 20, 34, 400 + i, g_hFontNormal);
        y += 42;
    }

    // 3. محرر النصوص (وسط)
    int editorX = panelX + panelW + 15;
    int editorW = 420;
    g_hScriptEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", 
        L"1 -- Welcome to Chess\n2 -- Write your Lua script here...",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        editorX, panelY, editorW, 400, hwnd, (HMENU)500, GetModuleHandle(NULL), NULL);
    SendMessageW(g_hScriptEdit, EM_SETBKGNDCOLOR, 0, COLOR_BG_EDITOR);
    SendMessageW(g_hScriptEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(10, 10));
    SetControlFont(g_hScriptEdit, g_hFontCode);

    // 4. الكونسول (يمين)
    int consoleX = editorX + editorW + 15;
    int consoleW = WINDOW_WIDTH - consoleX - 25;
    g_hConsole = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", 
        L"[14:35:21] Welcome to Chess!\n[14:35:21] Waiting for Roblox...\n[14:35:23] Attached (PID: 1234)\n[14:35:23] Ready.",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
        consoleX, panelY, consoleW, 340, hwnd, (HMENU)501, GetModuleHandle(NULL), NULL);
    SendMessageW(g_hConsole, EM_SETBKGNDCOLOR, 0, COLOR_BG_CONSOLE);
    SendMessageW(g_hConsole, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(10, 10));
    SetControlFont(g_hConsole, g_hFontStatus);

    // 5. حقل إدخال الكونسول
    int inputY = panelY + 345;
    g_hConsoleInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"> ",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        consoleX, inputY, consoleW, 28, hwnd, (HMENU)502, GetModuleHandle(NULL), NULL);
    SendMessageW(g_hConsoleInput, EM_SETBKGNDCOLOR, 0, COLOR_BG_EDITOR);
    SetControlFont(g_hConsoleInput, g_hFontStatus);

    // 6. قائمة السكربتات الحديثة (أسفل المحرر)
    int listY = panelY + 410;
    int listW = editorW;
    g_hRecentList = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
        editorX, listY, listW, 130, hwnd, (HMENU)503, GetModuleHandle(NULL), NULL);
    SetControlFont(g_hRecentList, g_hFontNormal);
    const wchar_t* scripts[] = { L"aimbot.lua", L"killall.lua", L"infinite_yield.lua", L"bring_all.lua", L"dex_exploit.lua", L"fly.lua", L"noclip.lua" };
    for (const auto& s : scripts) {
        SendMessageW(g_hRecentList, LB_ADDSTRING, 0, (LPARAM)s);
    }

    // 7. أزرار التحكم (EXECUTE, CLEAR, وما إلى ذلك)
    int btnY = listY;
    CreateStyledButton(hwnd, L"▶ EXECUTE", editorX + 10, btnY + 85, 100, 32, 600, g_hFontNormal);
    CreateStyledButton(hwnd, L"🗑 CLEAR", editorX + 120, btnY + 85, 100, 32, 601, g_hFontNormal);
    CreateStyledButton(hwnd, L"📊 VIEW ALL", editorX + 230, btnY + 85, 100, 32, 602, g_hFontNormal);

    // 8. شريط الحالة (أسفل النافذة)
    int statusY = WINDOW_HEIGHT - 35;
    g_hStatus = CreateWindowW(L"STATIC", L"● STATUS: Ready  |  ATTACHED: No  |  SCRIPTS: 0  |  VERSION " APP_VERSION,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, statusY, WINDOW_WIDTH - 20, 25, hwnd, (HMENU)700, GetModuleHandle(NULL), NULL);
    SetControlFont(g_hStatus, g_hFontStatus);
}

// ================================================================
// 5. معالج الأحداث الرئيسي (WM_NCHITTEST للسماح بسحب النافذة)
// ================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateUI(hwnd);
        break;

    case WM_NCHITTEST: {
        // السماح بسحب النافذة من شريط العنوان المخصص
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ScreenToClient(hwnd, &pt);
        if (pt.y < 32) {
            return HTCAPTION;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORLISTBOX: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, COLOR_BG_DARK);
        SetTextColor(hdc, COLOR_TEXT_MAIN);
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        HWND hCtl = (HWND)lParam;

        // أزرار القائمة الجانبية
        if (id >= 300 && id < 303) {
            wchar_t buf[64];
            GetWindowTextW(hCtl, buf, 64);
            std::wstring status = L"● STATUS: ";
            status += buf;
            status += L"  |  ATTACHED: No  |  SCRIPTS: 0  |  VERSION " APP_VERSION;
            SetWindowTextW(g_hStatus, status.c_str());
        }

        // ATTACH
        if (id == 400) {
            SetWindowTextW(g_hStatus, L"● STATUS: Attaching...  |  ATTACHED: No  |  SCRIPTS: 0  |  VERSION " APP_VERSION);
            wchar_t console[8192];
            GetWindowTextW(g_hConsole, console, 8192);
            std::wstring newMsg = std::wstring(console) + L"\n[14:35:24] Attaching to Roblox...";
            SetWindowTextW(g_hConsole, newMsg.c_str());
        }

        // INJECT
        if (id == 401) {
            SetWindowTextW(g_hStatus, L"● STATUS: Injected  |  ATTACHED: Yes  |  SCRIPTS: 1  |  VERSION " APP_VERSION);
            wchar_t console[8192];
            GetWindowTextW(g_hConsole, console, 8192);
            std::wstring newMsg = std::wstring(console) + L"\n[14:35:25] Injected successfully!";
            SetWindowTextW(g_hConsole, newMsg.c_str());
        }

        // EXECUTE
        if (id == 600) {
            wchar_t script[4096];
            GetWindowTextW(g_hScriptEdit, script, 4096);
            if (wcslen(script) > 10) {
                SetWindowTextW(g_hStatus, L"● STATUS: Executing...  |  ATTACHED: Yes  |  SCRIPTS: 1  |  VERSION " APP_VERSION);
                wchar_t console[8192];
                GetWindowTextW(g_hConsole, console, 8192);
                std::wstring newMsg = std::wstring(console) + L"\n[14:35:26] Script executed!";
                SetWindowTextW(g_hConsole, newMsg.c_str());
            } else {
                SetWindowTextW(g_hStatus, L"● STATUS: Please write a valid script  |  ATTACHED: No  |  VERSION " APP_VERSION);
            }
        }

        // CLEAR
        if (id == 601) {
            SetWindowTextW(g_hScriptEdit, L"");
            SetWindowTextW(g_hStatus, L"● STATUS: Cleared  |  ATTACHED: Yes  |  SCRIPTS: 0  |  VERSION " APP_VERSION);
        }

        // VIEW ALL
        if (id == 602) {
            SetWindowTextW(g_hStatus, L"● STATUS: Viewing all scripts  |  ATTACHED: Yes  |  VERSION " APP_VERSION);
        }

        // اختيار سكربت من القائمة
        if (id == 503 && HIWORD(wParam) == LBN_SELCHANGE) {
            int sel = SendMessageW(g_hRecentList, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR) {
                wchar_t name[128];
                SendMessageW(g_hRecentList, LB_GETTEXT, sel, (LPARAM)name);
                std::wstring content = L"1 -- Loaded: ";
                content += name;
                content += L"\n2 -- Write your Lua script here...";
                SetWindowTextW(g_hScriptEdit, content.c_str());
                SetWindowTextW(g_hStatus, (L"● STATUS: Loaded " + std::wstring(name) + L"  |  ATTACHED: No  |  VERSION " APP_VERSION).c_str());
            }
        }

        // أزرار شريط العنوان
        if (id == 200) { // تصغير
            ShowWindow(hwnd, SW_MINIMIZE);
        }
        if (id == 201) { // تكبير/استعادة
            if (g_isMaximized) {
                SetWindowPlacement(hwnd, (WINDOWPLACEMENT*)&g_oldRect);
                g_isMaximized = false;
                SetWindowTextW(g_hBtnMaximize, L"☐");
            } else {
                GetWindowRect(hwnd, &g_oldRect);
                ShowWindow(hwnd, SW_MAXIMIZE);
                g_isMaximized = true;
                SetWindowTextW(g_hBtnMaximize, L"☒");
            }
        }
        if (id == 202) { // إغلاق
            PostQuitMessage(0);
        }
        break;
    }

    case WM_DESTROY:
        DeleteObject(g_hFontTitle);
        DeleteObject(g_hFontNormal);
        DeleteObject(g_hFontCode);
        DeleteObject(g_hFontStatus);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ================================================================
// 6. نقطة الدخول الرئيسية
// ================================================================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    // تهيئة عناصر التحكم
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    // تسجيل النافذة
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_BG_DARK);
    wc.lpszClassName = L"ChessExecutorClass";
    RegisterClassExW(&wc);

    // إنشاء النافذة (بدون شريط عنوان تقليدي)
    g_hMain = CreateWindowExW(
        0,
        L"ChessExecutorClass",
        APP_NAME,
        WS_OVERLAPPEDWINDOW & ~WS_CAPTION & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        NULL,
        hInst,
        NULL
    );

    if (!g_hMain) {
        MessageBoxW(NULL, L"Failed to create window!", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_hMain, nCmdShow);
    UpdateWindow(g_hMain);

    // حلقة الرسائل
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
