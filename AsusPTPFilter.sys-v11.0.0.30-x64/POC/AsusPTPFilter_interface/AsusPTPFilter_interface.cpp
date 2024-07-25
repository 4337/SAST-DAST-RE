// AsusPTPFilter_interface.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//
#include <Windows.h>
#include <wchar.h>
#include <iostream>

//\Device\AsusTP

class AsusPTPFileter_interface {

    HANDLE dev_;

public:
    explicit AsusPTPFileter_interface(const WCHAR* dev_name = L"\\\\.\\AsusTP");
    ~AsusPTPFileter_interface();

    inline bool is_open() {
        return (dev_ != INVALID_HANDLE_VALUE) ? true : false;
    }

    DWORD read(void* out, DWORD size);

    DWORD ioctl_0x002215A8();

    DWORD ioctl_0x00221404();

    DWORD ioctl_0x00221408();

    //...

};


DWORD AsusPTPFileter_interface::ioctl_0x00221408() {

    BYTE buff[1024] = { 0x434 };
    DWORD dwRet = 0;
    DeviceIoControl(dev_, 0x00221408, &buff[0], 0x00, &buff[0], 0x434, &dwRet, NULL);
    return dwRet;

}

DWORD AsusPTPFileter_interface::ioctl_0x00221404() {

    BYTE buff[1024] = { 0x42 };
    DWORD dwRet = 0;
    DeviceIoControl(dev_, 0x00221404, &buff[0], 0x00, &buff[0],1024, &dwRet, NULL);
    return dwRet;

}

DWORD AsusPTPFileter_interface::ioctl_0x002215A8() {

        BYTE buff[0x438] = { 0x41 };
        DWORD dwRet = 0;
        DeviceIoControl(dev_, 0x002215A8, &buff[0], 0x438, NULL, 0x0, &dwRet, NULL);
        return dwRet;

}


DWORD AsusPTPFileter_interface::read(void* out, DWORD size) {
     
    DWORD dwRead = 0;
    ReadFile(dev_, out, size, &dwRead, NULL);
    return dwRead;

}

AsusPTPFileter_interface::AsusPTPFileter_interface(const WCHAR* dev_name) {
    dev_ = CreateFile(dev_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

AsusPTPFileter_interface::~AsusPTPFileter_interface() {
    if (dev_ != INVALID_HANDLE_VALUE) {
        CloseHandle(dev_);
    }
}


int main()
{
    AsusPTPFileter_interface Asus_tp;

    std::wcout << L"Is Open ? " << Asus_tp.is_open() << std::endl;

   // Asus_tp.ioctl_0x002215A8();

    //Asus_tp.ioctl_0x00221404();

   // Asus_tp.ioctl_0x00221408();

    //std::wcout << L"Last error: " << GetLastError() << std::endl;

    BYTE r_buff[128] = { 0x00 };
    DWORD ret = Asus_tp.read(r_buff, 128); //crashh

    std::wcout << L"read = " << ret << std::endl;

    return 0;

}

// Uruchomienie programu: Ctrl + F5 lub menu Debugowanie > Uruchom bez debugowania
// Debugowanie programu: F5 lub menu Debugowanie > Rozpocznij debugowanie

// Porady dotyczące rozpoczynania pracy:
//   1. Użyj okna Eksploratora rozwiązań, aby dodać pliki i zarządzać nimi
//   2. Użyj okna programu Team Explorer, aby nawiązać połączenie z kontrolą źródła
//   3. Użyj okna Dane wyjściowe, aby sprawdzić dane wyjściowe kompilacji i inne komunikaty
//   4. Użyj okna Lista błędów, aby zobaczyć błędy
//   5. Wybierz pozycję Projekt > Dodaj nowy element, aby utworzyć nowe pliki kodu, lub wybierz pozycję Projekt > Dodaj istniejący element, aby dodać istniejące pliku kodu do projektu
//   6. Aby w przyszłości ponownie otworzyć ten projekt, przejdź do pozycji Plik > Otwórz > Projekt i wybierz plik sln
