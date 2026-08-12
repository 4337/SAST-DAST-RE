#pragma once

#include <string>
#include <Windows.h>

class Nord {
	  
	DWORD pid;
	HANDLE handler;

	const DWORD time_out = 5000;

	Nord(const Nord&) = delete;
	Nord& operator=(const Nord&) = delete;

	std::string full_path;

public:

	Nord() noexcept : pid(-1), full_path() {
	}

	Nord(Nord&& other) noexcept
		: pid(other.pid) {
		other.pid = -1;
	}

	Nord& operator=(Nord&& other) noexcept {
		if (this != &other) {
			if (handler != INVALID_HANDLE_VALUE) {
				CloseHandle(handler);
			}
			pid = other.pid;
			handler = other.handler;
			other.pid = -1;
			other.handler = INVALID_HANDLE_VALUE;
		}
		return *this;
	}

	~Nord() noexcept {
		if (handler != INVALID_HANDLE_VALUE && handler != nullptr) {
			TerminateProcess(handler, 0);
			CloseHandle(handler);
			handler = INVALID_HANDLE_VALUE;
		}
	}

	/*
	* Checks if NordVPN is installed
	*/
	bool  is_installed();

	/*
	* Returns the NordVPN 
	*/
	std::string version() const noexcept(false);

	/*
	* Checks if NordVPN is running
	*/
	DWORD is_running() const;

	/*
	* Terminate NordVPN
	*/
	BOOL stop(DWORD) const;

	/*
	* Create NordVPN process suspended 
	*/
	DWORD start_suspended();

	DWORD get_pid() const noexcept {
		return pid;
	}

};
