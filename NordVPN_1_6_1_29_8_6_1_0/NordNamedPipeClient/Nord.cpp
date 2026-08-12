#include <Windows.h>
#include <shlwapi.h>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include <tlhelp32.h>

#include "Helpers.h"
#include "Nord.h"

DWORD Nord::start_suspended() {

    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    ZeroMemory(&process_info, sizeof(process_info));

    BOOL success = CreateProcess(
        nullptr,                    
        full_path.data(),        
        nullptr,                   
        nullptr,                    
        FALSE,                      
        CREATE_SUSPENDED,           
        nullptr,                    
        nullptr,                    
        &startup_info,              
        &process_info               
    );

    if (!success) {
        return -1;
    }

    pid = process_info.dwProcessId;

    handler=process_info.hProcess;

    return pid;
}

BOOL Nord::stop(DWORD pid) const {
    if (pid == -1) {
        return FALSE;
    }

    HANDLE process_handle = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (process_handle == nullptr) {
        return false; 
    }

    BOOL result = TerminateProcess(process_handle, 0);

    CloseHandle(process_handle);

    return result;
}

DWORD Nord::is_running() const {
    if (full_path.empty()) {
        throw std::runtime_error("NordVPN is not installed");
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return -1; 
    }

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);

    DWORD pid = -1;
    const std::string target_process = "NordVPN.exe";

    if (Process32First(snapshot, &process_entry)) {
        do {
            if (_stricmp(target_process.c_str(), process_entry.szExeFile) == 0) {

                HANDLE h_proc = OpenProcess(SYNCHRONIZE, FALSE, process_entry.th32ProcessID);
                if (h_proc != NULL) {
                    DWORD wait_result = WaitForSingleObject(h_proc, 0);
                    CloseHandle(h_proc);

                    if (wait_result == WAIT_TIMEOUT) {
                        pid = process_entry.th32ProcessID;
                        break;
                    }
                }
            }
        } while (Process32Next(snapshot, &process_entry));
    }

    CloseHandle(snapshot);
    return pid;
}


std::string Nord::version() const noexcept(false) {

    if (full_path.empty()) {
        throw std::runtime_error("NordVPN is not installed");
    }

    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSize(full_path.c_str(), &handle);
    if (size == 0) {
        throw std::runtime_error("Failed to get version info size for NordVPN.exe");
    }

    binary_string buffer(size, 0);
    if (!GetFileVersionInfo(full_path.c_str(), handle, size, buffer.data())) {
        throw std::runtime_error("Failed to extract version info from NordVPN.exe");
    }

    VS_FIXEDFILEINFO* file_info = nullptr;
    UINT file_info_size = 0;
    if (!VerQueryValue(buffer.data(), "\\", reinterpret_cast<LPVOID*>(&file_info), &file_info_size) || file_info_size == 0) {
        throw std::runtime_error("Failed to query version value from NordVPN.exe");
    }

    std::string major = std::to_string(HIWORD(file_info->dwFileVersionMS));
    std::string minor = std::to_string(LOWORD(file_info->dwFileVersionMS));
    std::string build = std::to_string(HIWORD(file_info->dwFileVersionLS));
    std::string revision = std::to_string(LOWORD(file_info->dwFileVersionLS));

    return major + "." + minor + "." + build + "." + revision;
   
}

bool Nord::is_installed() {

    const std::string sub_path = "\\NordVPN\\NordVPN.exe";
    const std::string env_variables[] = { "ProgramFiles", "ProgramFiles(x86)" };

    for (const std::string& env_var : env_variables) {
        char* buffer = nullptr;
        size_t size = 0;

        if (getenv_s(&size, nullptr, 0, env_var.c_str()) == 0 && size > 0) {
            full_path.resize(size);

            if (getenv_s(&size, &full_path[0], size, env_var.c_str()) == 0) {
                
                if (!full_path.empty() && full_path.back() == '\0') {
                    full_path.pop_back();
                }

                full_path += sub_path;

                if (PathFileExistsA(full_path.c_str())) {
                    return true;
                }
            }
        }
    }

    return false;
}