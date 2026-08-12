#pragma once

#include <Windows.h>

#include "Utils.h"

const int MAX_ERROR = 4;

class Ipc {

	HANDLE handler;
	const size_t HEADER_SIZE = 4;
	const DWORD VERSION = 0x00000001;

	/*
	 * Nie udzielamy dostępu do handler nikomu poza funkcją process::execve_as_client
	 * która potrzebuje go w trybie odczytu aby uruchomić process z tokenem klienta.
	*/

	friend DWORD execve_as_client(const Ipc&);  

	Ipc(const Ipc&) = delete;
	Ipc& operator=(const Ipc&) = delete;

public:

	Ipc() : handler(INVALID_HANDLE_VALUE) {}

	DWORD client(const std::string&, DWORD, DWORD timeout = 20000);
	DWORD server(const std::string&, 
		         DWORD open_mode = PIPE_ACCESS_DUPLEX | WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY,
		         DWORD pipe_mode = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
		         DWORD max_instances = PIPE_UNLIMITED_INSTANCES); 

	DWORD impersonation();

	bool is_alive() {
		DWORD PipeState = 0;
		return GetNamedPipeHandleState(handler, &PipeState, nullptr, nullptr, nullptr, nullptr, 0) != FALSE;
	}

	size_t bytes_to_read() {
		DWORD to_read = 0;
		PeekNamedPipe(handler, nullptr, 0, nullptr, &to_read, nullptr);
		return static_cast<size_t>(to_read);
	}

	/*
	* Pobierz klucz publiczny z komunikatu IPC w formacie JSON
	*/
	template<typename T>
	std::string get_public_key(const T&);

	template<typename T>
	binary_string create_public_msg(const T&);

	template <typename T>
	std::string create_pipename(const T&);

	template <typename T>
	T read(size_t);

	template <typename T>
	size_t write(const T&);

	binary_string crypto_stream_header(const binary_string&);

	/*
	* Ok, dane zaszyfrowane mają chyba inny format
	* 4b=rozmiar,4b=wersja(0x00,0x00,0x00,0x01), nonce, cryptodata ?!? 
	* gdzieś to widziałem, ale zapomniałem
	*/
	size_t write_crypto_stream(const binary_string&);

	binary_string read_crypto_stream(size_t);

	/*
	* format
	* 4b=size|4b=VERSION|none|msg
	*/
	template <typename T>
	binary_string create_private_msg(const T&); 

	void close() {
		if (handler != nullptr && handler != INVALID_HANDLE_VALUE) {
			CloseHandle(handler);
			handler = INVALID_HANDLE_VALUE;
		}
	}

	~Ipc() {
		close();
	}

};