#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <filesystem>
#include <psapi.h>

namespace fs = std::filesystem;

bool IsDLLLoaded(HANDLE hProcess, const std::string& dllName) {
    HMODULE hMods[1024];
    DWORD cbNeeded;
    char szModName[MAX_PATH];

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            if (GetModuleBaseNameA(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char))) {
                if (dllName == szModName) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool InjectDLL(DWORD processID, const std::string& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Process acilamadi! Hata kodu: " << GetLastError() << std::endl;
        return false;
    }

    std::string dllName = fs::path(dllPath).filename().string();
    if (IsDLLLoaded(hProcess, dllName)) {
        std::cerr << "DLL zaten islemde yuklu: " << dllName << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    LPVOID pDllPath = VirtualAllocEx(hProcess, 0, dllPath.size() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!pDllPath) {
        std::cerr << "Bellek ayirma basarisiz! Hata kodu: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pDllPath, dllPath.c_str(), dllPath.size() + 1, NULL)) {
        std::cerr << "Bellege yazma basarisiz! Hata kodu: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"),
        pDllPath, 0, NULL);
    if (!hThread) {
        std::cerr << "Uzaktan thread olusturma basarisiz! Hata kodu: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    bool dllLoaded = IsDLLLoaded(hProcess, dllName);
    if (!dllLoaded) {
        std::cerr << "DLL enjekte edilemedi: " << dllName << std::endl;
    }
    else {
        std::cout << "DLL basariyla enjekte edildi: " << dllName << std::endl;
    }

    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return dllLoaded;
}

std::vector<std::string> ListDLLs(const std::string& path) {
    std::vector<std::string> dllFiles;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path().extension() == ".dll") {
            dllFiles.push_back(entry.path().string());
        }
    }
    return dllFiles;
}

int main() {
    DWORD pid;
    std::string dllPath;

    std::cout << "Enjekte etmek istediginiz islem PID'sini girin: ";
    std::cin >> pid;

    std::string currentPath = fs::current_path().string();
    std::vector<std::string> dllFiles = ListDLLs(currentPath);

    if (dllFiles.empty()) {
        std::cerr << "Calisilan dizinde hiç DLL dosyasi bulunamadi!" << std::endl;
        return 1;
    }

    std::cout << "Enjekte edilecek DLL dosyasini secin: " << std::endl;
    for (size_t i = 0; i < dllFiles.size(); ++i) {
        std::cout << i + 1 << ". " << dllFiles[i] << std::endl;
    }

    int dllChoice;
    std::cin >> dllChoice;

    if (dllChoice < 1 || dllChoice > static_cast<int>(dllFiles.size())) {
        std::cerr << "Gecersiz secim!" << std::endl;
        return 1;
    }

    dllPath = dllFiles[dllChoice - 1];

    if (!InjectDLL(pid, dllPath)) {
        std::cerr << "DLL Injection basarisiz!" << std::endl;
        return 1;
    }

    return 0;
}
