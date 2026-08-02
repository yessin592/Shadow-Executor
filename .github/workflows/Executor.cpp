// ================================================================
// SHADOW EXECUTOR V99 - CORE ENGINE
// ================================================================
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

class PatternScanner {
public:
    static std::vector<int> PatternToBytes(const char* pattern) {
        std::vector<int> bytes;
        const char* start = pattern;
        while (*start) {
            if (*start == ' ') { start++; continue; }
            if (*start == '?') { bytes.push_back(-1); start++; if (*start == ' ') start++; continue; }
            bytes.push_back(strtoul(start, (char**)&start, 16));
        }
        return bytes;
    }

    static uintptr_t FindPattern(HMODULE module, const char* pattern) {
        if (!module) return 0;
        MODULEINFO info = { 0 };
        if (!GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info))) return 0;
        uintptr_t start = (uintptr_t)info.lpBaseOfDll;
        uintptr_t end = start + info.SizeOfImage;
        auto patternBytes = PatternToBytes(pattern);
        if (patternBytes.empty()) return 0;

        for (uintptr_t i = start; i < end - patternBytes.size(); i++) {
            bool found = true;
            for (size_t j = 0; j < patternBytes.size(); j++) {
                if (patternBytes[j] == -1) continue;
                if (*(unsigned char*)(i + j) != patternBytes[j]) { found = false; break; }
            }
            if (found) return i;
        }
        return 0;
    }

    static uintptr_t FindPatternAndResolve(HMODULE module, const char* pattern, int offset = 0) {
        uintptr_t addr = FindPattern(module, pattern);
        if (!addr) return 0;
        if (*(unsigned char*)addr == 0xE8 || *(unsigned char*)addr == 0xE9) {
            int32_t relOffset = *(int32_t*)(addr + 1);
            return addr + relOffset + 5;
        }
        if (*(unsigned char*)addr == 0x48 && *(unsigned char*)(addr + 1) == 0x8D) {
            int32_t relOffset = *(int32_t*)(addr + 3);
            return addr + relOffset + 7;
        }
        return addr + offset;
    }
};

struct SignatureSet {
    const char* name;
    const char* patterns[5];
    int count;
};

static SignatureSet g_Signatures[] = {
    {
        "lua_loadstring",
        {
            "E8 ?? ?? ?? ?? 80 3D ?? ?? ?? ?? 00 74 0F",
            "E8 ?? ?? ?? ?? 83 C4 04 85 C0 74 0A",
            "E8 ?? ?? ?? ?? 80 3D ?? ?? ?? ?? 00 75",
            "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0F E8",
            "E8 ?? ?? ?? ?? 84 C0 74 0F 8B"
        },
        5
    },
    {
        "lua_pcall",
        {
            "8B 44 24 04 85 C0 74 0A 6A 00",
            "8B 44 24 04 83 EC 0C 85 C0 74 0A",
            "55 8B EC 83 EC 0C 8B 45 08 85 C0",
            "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0A E8",
            "8B 44 24 04 85 C0 74 07 6A 00"
        },
        5
    },
    {
        "getstate",
        {
            "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 05 48 8B 40 ??",
            "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0C 48 8B 40 10",
            "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0A 48 8B 40 08",
            "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 07 48 8B 40 20",
            "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 05 48 8B 41 10"
        },
        5
    }
};

typedef int(__cdecl* luaL_loadstring_t)(void* L, const char* s);
typedef int(__cdecl* lua_pcall_t)(void* L, int nargs, int nresults, int errfunc);
typedef void*(__cdecl* rbx_getstate_t)();

luaL_loadstring_t g_luaL_loadstring = nullptr;
lua_pcall_t g_lua_pcall = nullptr;
void* g_lua_State = nullptr;
HANDLE g_hPipe = INVALID_HANDLE_VALUE;
std::string g_CurrentUsername = "Unknown";

bool FindSignaturesAutomatically(HMODULE hRoblox) {
    for (auto& sigSet : g_Signatures) {
        uintptr_t addr = 0;
        bool found = false;
        for (int i = 0; i < sigSet.count; i++) {
            addr = PatternScanner::FindPatternAndResolve(hRoblox, sigSet.patterns[i]);
            if (addr != 0) { found = true; break; }
        }
        if (!found) {
            for (int i = 0; i < sigSet.count; i++) {
                addr = PatternScanner::FindPattern(hRoblox, sigSet.patterns[i]);
                if (addr != 0) { found = true; break; }
            }
        }
        if (!found) { continue; }
        if (strcmp(sigSet.name, "lua_loadstring") == 0) g_luaL_loadstring = (luaL_loadstring_t)addr;
        else if (strcmp(sigSet.name, "lua_pcall") == 0) g_lua_pcall = (luaL_pcall_t)addr;
        else if (strcmp(sigSet.name, "getstate") == 0) {
            rbx_getstate_t getstate = (rbx_getstate_t)addr;
            g_lua_State = getstate();
        }
    }
    return (g_luaL_loadstring && g_lua_pcall && g_lua_State);
}

bool InitializeRobloxMemory() {
    HMODULE hRoblox = GetModuleHandleA("RobloxPlayerBeta.exe");
    if (!hRoblox) { hRoblox = GetModuleHandle(NULL); if (!hRoblox) return false; }
    if (!FindSignaturesAutomatically(hRoblox)) {
        MessageBoxA(NULL, "Failed to find Lua signatures!", "Error", MB_OK | MB_ICONERROR);
        return false;
    }
    return true;
}

bool ExecuteScript(const char* script) {
    if (!g_luaL_loadstring || !g_lua_pcall || !g_lua_State) {
        if (!InitializeRobloxMemory()) return false;
    }
    if (g_luaL_loadstring(g_lua_State, script) != 0) return false;
    if (g_lua_pcall(g_lua_State, 0, 0, 0) != 0) return false;
    return true;
}

void SendUsernameToInjector(const std::string& username) {
    HANDLE hPipe = CreateFileA("\\\\.\\pipe\\ShadowExecutorPipe", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe != INVALID_HANDLE_VALUE) {
        std::string msg = "USERNAME:" + username;
        DWORD bytesWritten;
        WriteFile(hPipe, msg.c_str(), msg.length(), &bytesWritten, NULL);
        CloseHandle(hPipe);
    }
}

std::string GetRobloxUsername() {
    if (!g_lua_State) return "Unknown";
    return "Bound_User";
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    Sleep(2000);
    if (!InitializeRobloxMemory()) {
        MessageBoxA(NULL, "Failed to initialize memory!", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    g_CurrentUsername = GetRobloxUsername();
    SendUsernameToInjector(g_CurrentUsername);

    std::string msg = "Shadow Executor V99 Injected!\nBound to account: " + g_CurrentUsername;
    MessageBoxA(NULL, msg.c_str(), "Shadow Executor", MB_OK);

    const char* defaultScript = R"(
        loadstring(game:HttpGet('https://raw.githubusercontent.com/Klinac/scripts/main/blockspin.lua', true))()
        print('Shadow Executor V99: Script Executed!')
    )";
    ExecuteScript(defaultScript);

    g_hPipe = CreateNamedPipeA(
        "\\\\.\\pipe\\ShadowExecutorPipe",
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 4096, 4096, 0, NULL
    );

    if (g_hPipe != INVALID_HANDLE_VALUE) {
        ConnectNamedPipe(g_hPipe, NULL);
        char buffer[8192];
        DWORD bytesRead;
        while (true) {
            if (ReadFile(g_hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                ExecuteScript(buffer);
            }
            Sleep(100);
        }
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
    }
    return TRUE;
}

extern "C" __declspec(dllexport) bool StartExecutor() {
    // This function is called from main.exe to start the injection
    return true;
}