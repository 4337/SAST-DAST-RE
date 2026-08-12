#pragma once

#include <Windows.h>
#include <stdexcept>

#include "Helpers.h"
#include "LoadLibraryR.h"

class Inject {

	binary_string buff;

	int read_all(const std::string& path) {
		
		HANDLE h = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (h == INVALID_HANDLE_VALUE || h == NULL) {
			return -1;
		}
		
		LARGE_INTEGER file_size;
		if (!GetFileSizeEx(h, &file_size)) {
			CloseHandle(h);
			return -1;
		}

		if (file_size.QuadPart <= 0) {
			CloseHandle(h);
			return -1;
		}

		buff.resize(static_cast<size_t>(file_size.QuadPart));
		DWORD bytes_to_read = static_cast<DWORD>(file_size.QuadPart);

		DWORD readed = 0;
		if (!ReadFile(h, buff.data(), bytes_to_read, &readed, nullptr)) {
			return -1;
		}

		return 0;

	}

public:

	Inject(const std::string& path) {
		if (read_all(path) != 0) {
			throw std::runtime_error("Invalid file path [Inject::Inject]");
		}
	}
	
	void se_debug() const noexcept(false) {

		HANDLE h_token = INVALID_HANDLE_VALUE;
		TOKEN_PRIVILEGES priv = { 0 };

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &h_token)) {
		    throw std::runtime_error("OpenProcessToken failed");
		}

	    priv.PrivilegeCount = 1;
		priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid)) {
			throw std::runtime_error("LookupPrivilegeValue failed");
		}
			
		AdjustTokenPrivileges(h_token, FALSE, &priv, 0, NULL, NULL);
	    CloseHandle(h_token);
			
	}

	bool inject(DWORD pid) const {
		 HANDLE h_proc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | 
			                         PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
		 if (NULL == h_proc) {
			 return false;
		 }

		 HANDLE h_mod = LoadRemoteLibraryR(h_proc, (LPVOID)buff.data(), (DWORD)buff.length(), NULL);
		 if (NULL == h_mod) {
			 return false;
		 }
		 return true;
	}

};