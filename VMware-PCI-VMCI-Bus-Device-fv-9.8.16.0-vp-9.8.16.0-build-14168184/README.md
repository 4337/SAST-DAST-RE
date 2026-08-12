//Analizowany statycznie: fv-9.8.16.0 vp-9.8.16.0 build-14168184
<br/>
//Analizowany dynamicznie: fv-9.8.16.0 vp-9.8.16.0 build-14168184 
<br/>
<h3>Historia niezainicjalizowanej pamięci</h3>
<br/>
Pierwotnie koncepcja dla tych repozytoriów była taka, aby umieszczać w nich "suche" informacje o możliwościach 
wchodzenia w interakcje z różnymi modułami jądra systemu MS Windows wzbogacone o moje jakże błyskotliwe wnioski. 
Z czasem, głównie podczas analizy vmci z VMWare workstation ten koncept się zmieniał między innymi dlatego, że znalazłem w nim potencjalne
podatności, a ponieważ diabeł jak zawsze tkwi w szczegółach 
i trudno jest opowiadać o czymkolwiek w oderwaniu od szerszego kontekstu to zmieniało się również podejście do samej analizy 
i na pewnym etapie swoje założenia i wnioski zacząłem weryfikować przy pomocy debugera. Nie zmienia to jednak faktu, że 90% informacji 
tu zawartych to statyczna analiza asemblera x64/x86, 5% to przeglądanie wersji otwarto-źródłowych np. dla linuxa, a 5% to debuger.
<br/>
PS.
<br/>
Zaznaczam, że nie jestem specjalistą od wirtualizacji i dla osób zanurzonych np. w klimaty VMWare może być trochę śmiesznie poza tym 
statyczna analiza na poziomie asma to inny poziom abstrakcji, inny nawet niż analiza C czy C++, tu poruszamy się za pomocą określeń struktura a, struktura b,
funkcja x. Podczas gdy na innym poziomie to może być VMCIContext czy get_vmci_version - zwłaszcza gdy nie posiadamy symboli.
Ba, nie jestem nawet specjalistą od jądra Windowsa, to znaczy wiem, że są mikro jądra i jądra monolityczne i jak to tam w Windows ogólnie działa,
ale moje doświadczenie praktyczne z Kernelem jest małe. Ogólnie jestem Marcin, dopiero się uczę :)
<br/>
<br/>
<ul>
<li><a name="0x81032008"><b>ioctl: 0x81032008</b></a></li>
<u>Device: 0x103</u>
<br/>
<u>Function: 0x802</u>
<br/>
<u>Access: FILE_ANY_ACCESS</u>
<br/>  
<u>Method: METHOD_BUFFERED</u>
<br/>
<u>Rozmiar bufforów: OUT >= 0x4, IN >= 0x10</u>
<br/>
<u>Additional requirements:</u> 
                         (_Irp->CurrentStackLocation->FileObject->FsContext->off_0x10 != 0)     
                         (_Irp->RequestorMode == 1) //UserMode
                         (_Irp->CurrentStackLocation->FileObject->FsContext->off_0x14 != 0)
                         (FileObject->FsContext->off_0x00 == nullptr)
                         (dword_140013000 != 0)
<br/>
<br/>
Io->IN_SystemBuffer[0] = vmci_ctx_id<br/>
Io->IN_SystemBuffer[4] = 1 || 2 || 3<br/>       
Io->IN_SystemBuffer[8] = HANDLE || INVALID_HANDLE_VALUE - wartość przekazywana do ObReferenceObjectByHandle <br/>
<br/>
Główna procedura obsługi ioctl: 0x81032008 przyjmuje 6 argumentów. Pierwszy (ecx) to wartość przekazana przez użytkownika w _IRP->AssociatedIrp->SystemBuffer[0] i jest to vcmi_ctx_id
czyli identyfikator struktury (VMCIContext)->id. Drugi argument, również 32-bitowy może przyjąć wartość 1,2 lub 3. Argument trzeci to uchwyt lub wartość INVALID_HANDLE_VALUE jest on 
wykorzystywany wyłącznie w wywołaniu ObReferenceObjectByHandle jeśli nie jest równy -1. Czwarty argument to wartość _Irp->CurrentStackLocation->FileObject->FsContext->off_0x14, nie może być ona równa 0, tę wartość możemy kontrolować przy pomocy ioctl: 0x81032024. Argument numer 5 to referencja do _Irp->CurrentStackLocation->FileObject->FsContext.
Ostatni argument to wskaźnik do struktury SID. 
<br/>
Po weryfikacji przekazywanych argumentów funkcja Create_VMCIContext_and_put_in_global_list_sub_1400052E0 alokuje pamięć na VMCIContext inicjalizuje ją, dodaje do globalnej listy struktur VMCIContext i ustawia wartość Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 = 0xB.
<br/>
Poza weryfikacją niektórych argumentów przekazywanych do głównej funkcji obsługi 0x81032008 wymaga spełnienia dodatkowych warunków.
Większość z tych warunków jest domyślnie spełniona lub łatwo możemy je spełnić. Na przykład. _Irp->RequestorMode to w naszym przypadku "UserMode" tak jak tego oczekuje sterownik,
FileObject->FsContext->off_0x00 jest równy nullptr bo to jest wskaźnik do VMCIContext który dopiero tworzymy. Wartość _Irp->CurrentStackLocation->FileObject->FsContext->off_0x14 nie może wynosić 0, w tym przypadku możemy ustawić oczekiwaną wartość za pomocą ioctl:0x81032024, które powinno zostać wywołane przed 0x81032008. 
Zmienna globalna dword_140013000 jest domyślnie ustawiona na wartość "1".<br/>
Tak naprawdę problematyczny jest jeden warunek, to znaczy: 
<br/>
<br/>
<b>_Irp->CurrentStackLocation->FileObject->FsContext->off_0x10 != 0</b>
<br/>
<br/> 
Jedyne miejsce w sterowniku, jakie udało mi się znaleźć, które ustawia wartość _Irp->CurrentStackLocation->FileObject->FsContext->off_0x10 to obsługa <a href="#IRP_MJ_CREATE">IRP_MJ_CREATE</a>.
<br/> 
Oczywiście mogłęm coś przeoczyć kiedyś podczas analizy udało mi się pomylić mov Irp->CurrentStackLocation->FileObject->FsContext->off_0x8, 0xb z cmp Irp->CurrentStackLocation->FileObject->FsContext->off_0x8, 0xb.
<br/>
<br/>
<li><a name="0x81032004"><b>ioctl: 0x81032004</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function: 0x801</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>  
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: OUT >= 4, IN: Not used</u><br/>
<u>Additional requirements:</u> (FileObject->FsContext != nullptr)
<br/>
<br/>
<pre>

if(OutPutLength < 4) return ...

if( FileObject->FsContext->off_0x14 - 1 <= 8FFFEh ) IO->Out_buffer[0] = FileObject->FsContext->off_0x14;
else IO->Out_buffer[0] = 0x0B0000;

IofCompleteRequest(...)
...
return;
</pre>
<br/>
<br/>
<li><a name="0x81032018"><b>ioctl: 0x81032018</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function: 0x806</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>  
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: N/A</u><br/>
<u>Additional requirements:</u>  (FileObject->FsContext != nullptr) && (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB)
<br/>
<br/>
N/A	
<br/>
<br/>
<li><a name="0x8103201C"><b>ioctl: 0x8103201C</b></a></li>	
<u>Device: 0x103</u><br/>
<u>Function: 0x807</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>  
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: N/A</u><br/>
<u>Additional requirements:</u>  (FileObject->FsContext != nullptr) && (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB)
<br/>
<br/>
N/A	
<br/>
<br/>		 
<li><a name="0x81032020"><b>ioctl: 0x81032020</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function: 0x808</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>  
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: N/A</u><br/>
<u>Additional requirements:</u>  (FileObject->FsContext != nullptr) && (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB)
<br/>
<br/>
N/A	
<br/>
<br/>
<li><a name="0x81032024"><b>ioctl: 0x81032024</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function: 0x809</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: IN: >= 4, OUT >= 4</u><br/>
<u>Additional requirments:</u>  (FileObject->FsContext != nullptr)
<br/>
<br/>
IO->SystemBuffer_IN[0] = VALUE<br/>
<br/>
<br/>
<pre>
FileObject->FsContext->off_0x14 = IO->SystemBuffer_IN[0];

if(FileObject->FsContext->off_0x14 - 1 <= 0x8FFFE) {
   IO->SystemBuffer_OUT[0] = FileObject->FsContext->off_0x14;
} else {
      IO->SystemBuffer_OUT[0] = 0x0B0000;
}
</pre>
<br/>
<br/>
<li><a name="0x81032028"><b>ioctl: 0x81032028</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function: 0x80a</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>  
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: N/A</u><br/>
<u>Additional requirements:</u>  (FileObject->FsContext != nullptr) && (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB)
<br/>
<br/>
N/A	
<br/>
<br/>
<li><a name="0x8103202C"><b>ioctl: 0x8103202C</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function: 0x80b</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>  
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: N/A</u><br/>
<u>Additional requirements:</u>  (FileObject->FsContext != nullptr) && (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB)
<br/>
<br/>
N/A	
<br/>
<br/>
<li><a name="0x81032030"><b>ioctl: 0x81032030</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function: 0x80c</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: IN: 0x10, OUT >= 4</u><br/>
<u>Additional requirments:</u> (_Irp->CurrentStackLocation->FileObject->off_0x10 != 0) && (_Irp->CurrentStackLocation->FileObject->off_0x18 != 0) &&
                        (_Irp->RequestorMode == UserMode /*1*/)  && (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB)
<br/>
<br/>
VMCIQueuePair: Failed to unmap queue headers for queue pair (handle) - zwalania listę i deskryptory MDL, korzysta z DMA.
<br/>
<br/>
<li><a name="0x81032034"><b>ioctl: 0x81032034</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function: 0x80d</u><br/>
<u>Access:   FILE_ANY_ACCESS</u><br/>
<u>Method:  METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: Rozmiar bufforów: OUT >= 4, IN - 0x18 > 0x10FE8</u><br/>
<u>Additional requirments:</u> (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB) && (_Irp->CurrentStackLocation->FileObject->off_0x10 != 0) 
<br/>
<br/>
Komunikuje się z kontrolerem DMA, zwraca dość dużo danych.
<br/>
<br/>
<li><a name="0x81032038"><b>ioctl: 0x81032038</b></a></li>
<u>Device:  0x103</u><br/>
<u>Function: 0x80e</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: IN: 0, OUT: 0</u><br/>
<u>Additional requirments:</u> (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB) && (_Irp->CurrentStackLocation->FileObject->off_0x18 != 0)
<br/>
<br/>
N/A
<br/>
<br/>
<li><a name="0x81032044"><b>ioctl: 0x81032044</b></a></li>
<u>Device:  0x103</u><br/>
<u>Function: 0x811</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów:  IN == 8, OUT >= 4</u><br/>
<u>Additional requirments:</u> (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB) && (Irp->CurrentStackLocation->FileObject->FsContext->off_0x0 != 0)
<br/>
<br/>
N/A
<br/>
<br/>
<li><a name="0x81032048"><b>ioctl: 0x81032048</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function: 0x812</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów:  IN == 8, OUT  >= 4</u><br/>
<u>Additional requirments: </u> (Irp->CurrentStackLocation->FileObject->FsContext != nullptr) && (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB)
<br/>
<br/>
Między innymi zwalnia VMCIContext.
<br/>
<br/>
<li><a name="0x8103204C"><b>ioctl: 0x8103204C</b></a></li>
<u>Device:  0x103</u><br/>
<u>Function: 0x813</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: IN == 0x18, OUT >= IN</u><br/>
<u>Additional requirments: </u>(Irp->CurrentStackLocation->FileObject->FsContext != nullptr) && (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB)
<br/>
<br/>
IO->IN_SystemBuffer[0] = User-Land virtual address<br/>
IO->IN_SystemBuffer[8] = Kod operacji (1,2 6,7,8)<br/>
IO->IN_SystemBuffer[0xc] = Rozmiar alokacji >= 8<br/>
<br/>
Kiedy uda nam się spełnić wszystkie warunki wymagane przez kod obsługi ioctl 0x8103204C, czyli kiedy między innymi wielkości buforów wejścia/wyjścia są odpowiednie <br/>
oraz tajemnicza wartość 0xB zostanie ustawiona — oczywiście ja od pewnego momentu
wiem czym jest "tajemnicza" wartość 0xb, wiem nawet co jest przypisywane do Irp->CurrentStackLocation->FileObject->FsContext, <br/>
jest to prawdopodobnie Windowsowa implementacja struktury vmci_host_dev którą możecie znaleźć mn. <a href="https://android.googlesource.com/platform/hardware/bsp/kernel/freescale/+/master/drivers/misc/vmw_vmci/vmci_host.c" taregt="_blank">tu</a>,
<br/>
ale przez jakiś czas była to "tajemnicza wartość 0xB" - to wywoływana jest procedura która pobiera identyfikator struktury VMCIContext powiązanej z Irp->CurrentStackLocation->FileObject->FsContext->off_0x00->off_0x10 (FsContext = vmci_host_dev->context->cid), a następnie wywoływana jest procedura IOCTL_0x8103204C_sub_140005090.
<br/>
Funkcja IOCTL_0x8103204C_sub_140005090 przyjmuje 4 argumenty, z czego 2 kontrolujemy bezpośrednio za pomocą _IRP->AssociatedIrp->SystemBuffer. Argument przekazywany jako pierwszy 
to vmci_host_dev->context->cid uzyskany z Irp->CurrentStackLocation->FileObject->FsContext, drugi nie w pełni kontrolowany przez nas argument to wskaźnik, do którego zostanie przypisany bufor alokowany za pomocą ExAllocatePoolWithTag z kontrolowaną przez nas wielkością.
Pierwszy z kontrolowanych przez nas bezpośrednio argumentów to 32-bitowa wartość określająca 
"kod operacji" którą IOCTL_0x8103204C_sub_140005090 wykona, drugi argument to 32-bitowa wartość określająca wielkość alokacji, jest to wartość przekazywana w parametrze "NumberOfBytes" do funkcji ExAllocatePoolWithTag - tak mi też się tu zapaliła lampka z napisem "Denial of Service".
Poza tym w buforze I/O (_IRP->AssociatedIrp->SystemBuffer) jest jeszcze przekazywana 64-bitowa wartość, która określa adres pamięci rezydujący w przestrzeni użytkownika (r3), który będzie wykorzystany na późniejszym etapie.
Wykonanie funkcji IOCTL_0x8103204C_sub_140005090 zaczyna się od próby pobrania z globalnej listy instancji struktury VMCIContext na podstawie argumentu przekazanego w rejestrze ECX (FsContext = vmci_host_dev->context->cid). Jeżeli nie ma takiej instancji obsługa ioctl zostaje zakończona. 
<br/>
Jeśli natomiast instancja zostanie znaleziona to funkcja zaczyna parsować drugi z przekazanych argumentów, czyli "kod operacji" (_IRP->AssociatedIrp->SystemBuffer[8]).
<br/>
<br/>
<b>Kody operacji:</b><br/>
<ul>
<li>.Opcode "1" - Alokuje miejsce dla elementu VMCIContext->off_0x60 ...</li>
<li>
.Opcode. "2" - Zapiuje w IO->IN_SystemBuffer[0xc] wartośc 0, zeruje również drugi argument czyli wskaźnik do którego w innym wypadku przypisywany jest buffor alokowany za pomocą ExAllocatePoolWithTag z kontrolowaną przez nas wielkością. Następnie zmieniejsza licznik referencji struktury VMCIContext, zwalania ją i kończy działanie 
</li>
<li>
.Opcode. "6" - Alokuje miejsce dla elementu VMCIContext->off_0x50  ...
</li>
<li>
.Opcode. "7" - N/A
</li>
<li>
🔥.Opcode. "8"🔥 - Wywołuje funkcje IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0, która przyjmuje 3 argumenty.
Pierwszy argument to wskaźnik do struktury VMCIContext, drugi argument to wielkość alokacji _IRP->AssociatedIrp->SystemBuffer[0xc], trzeci argument to wskaźnik przekazywany do funkcji nadrzędnej jako parametr przy pomocy rejestru r9. 
Funkcja IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0 zaczyna od weryfikacji czy wielkość alokacji (_IRP->AssociatedIrp->SystemBuffer[0xc] < 8) jest mniejsza niż 8.
Jeśli tak to kończy działanie, jeśli nie to wywołuje ExAllocatePoolWithTag(PoolType=0x200 /*NonPagedPoolNx*/, NumberOfBytes=_IRP->AssociatedIrp->SystemBuffer[0xc],0x4D4D5443).
To, co jest dość istotne w tym miejscu to fakt, że pamięć po alokacji nie jest inicjalizowana.
<br/>
Następnie funkcja w pętli wyszukuje obiekt VMCIContext->off_0x18 na liście obiektów VMCIContext->off_0x20, jeśli obiekt nie zostanie znaleziony na liście to do zaalokowanej przez nas pamięci dynamicznej są kopiowane elementy z listy VMCIContext->off_0x20. To kopiowanie jest poprzedzone weryfikacją warunku, który sprawdza czy 
VMCIContext->off_0x20->off_0x10 + 8 <= IO->SystemBuffer[0xc] - tak więc funkcja sama zakłada, że bufor może być większy. 
Z naszego punktu widzenia jednak niczego to nie zmienia, ponieważ zarówno w przypadku kiedy bufor byłby mniejszy jak również wtedy kiedy szukany obiekt znajduje się na liście 
zaalokowana przez nas pamięć zostaje zwrócona przez wskaźnik do funkcji nadrzędnej.
Tak więc niezależnie od wielkości danych pojawia się tu problem niezainicjalizowanej pamięci jądra.
<br/>
Wartość zwracana przez funkcje IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0 jest weryfikowana dopiero po powrocie z funkcji IOCTL_0x8103204C_sub_140005090.
Ta wartość to 0 w przypadku gdy wywołanie powiodło się lub inna wartość kiedy nie uda się zaalokować pamięci, wtedy też kończy się obsługa ioctl: 0x8103204C.
Jeśli wszystko się powiodło to ioctl: 0x8103204C przechodzi do wywołania Call_ProbeForWrite_and_CopyMem_sub_14000964C.
Ta funkcja przyjmuje 3 parametry. 
Pierwszy parametry przekazywany przez RCX to _IRP->AssociatedIrp->SystemBuffer[0] i jest to adres pamięci ring-3, drugi parametr to
wskaźnik do pamięci zaalokowanej w IOCTLIRP_SystemBuffer_8_eq_8_sub_1400044C0 (to ta której wielkość kontrolujemy i która "często bywa niezainicjalizowana".
Trzeci to _IRP->AssociatedIrp->SystemBuffer[0xc] czyli wielkość alokacji — co może pójść nie tak :D.
Funkcja weryfikuje za pomocą ProbeForWrite czy pamięć wielkości alokacji, której adres przekazywany jest jako pierwszy argument rezyduje w przestrzeni użytkownika
ma odpowiednie prawa dostępu i jest odpowiednio wyrównana.
<br/>
Jeśli tak jest to koiuje pamięć jądra pod wskazany przez użytkownika adres r3.<br/>
Jeśli czytałeś uważnie to mamy tu conajmniej 2 błędy bezpieczeństwa:
<br/>
<br/>
1. Alokacja pamięci dynamicznej o rozmiarze bezpośrednio kontrolowanym przez użytkownika - potencjalny Local Denial of Service.<br/>
2. Zwrócenie do użytkownika pamięci o rozmiarze kontrolowanym przez r-3 która może być niezinicjalizowana - Ujawnienie pamięci jądra.<br/> 
<br/>
<br/>
Super, ale jak już wam wspomniałem większość ioctl tak jak w tym wypadku wymaga spełnienia pewnych warunków. W przypadku 0x8103204C wartość
Irp->CurrentStackLocation->FileObject->FsContext nie może być równa 0 (nullptr) oraz Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 musi mieć wartość 0xB.
Oczywiście ja już wiem, że w tym drugim przypadku wartość 0xB jest ustawiona tylko kiedy uzyskamy instancje struktury VMCIContext, każdy ioctl, który sprawdza tą wartość 
jeśli nie jest ona zgodna z 0xB zapisuje w logach komunikat "VMCI: Only valid for contexts." i kończy się jego obsługa.
Natomiast samą instancje VMCIContext możemy uzyskać za pośrednictwem <a href="#0x81032008">ioctl: 0x81032008.</a>
<br/>
<br/>
<b><a name="POC">[POC]</a></b>
<br/>
<br/>
Żeby potwierdzić przypuszczenia dotyczące słabości występujących w obsłudze ioctl 0x8103204C trzeba ustawić wartość różną od zera w  <br/>
_Irp->CurrentStackLocation->FileObject->FsContext->off_0x10. <br/>
Utworzyłem w tym celu prosty skrypt 
<a herf="https://github.com/4337/SAST-DAST-RE/blob/main/VMware-PCI-VMCI-Bus-Device-fv-9.8.16.0-vp-9.8.16.0-build-14168184/vmci_fv-9.8.16.0_vp-9.8.16.0-build-14168184.js">[JavaScript WinDbg]</a>,
który modyfikuje określoną wartość w pamięci podczas wykonania 
procedury obsługi IRP_MJ_CREATE. Skrypt ustawia również kilka punktów przerwań, które pozwalają monitorować przebieg wykonania obsługi ioctl 0x8103204C.
<br/>
<br/>
<b>[Wielka strategia]</b>
<br/>
<br/>
Dysponując kontrolą nad rozmiarem alokacji pamięci moglibyśmy wykorzystać opisywany błąd do ujawnienia naprawdę dużych fragmentów pamięci jądra.
Ja jednak zdecydowałem wykorzystać strukturę VMCIContext. Windowsowa wersja VMCIContext pod przesunięciem 0x88 zawiera wskaźnik do funkcji — niestety nie pamiętam, w którym miejscu jest on inicjalizowany, więc w naszym przypadku jest on równy 0 - poza tym składowe to wielu przypadkach wskaźniki do pamięci dynamicznej jądra. 
Kolejny aspekt, który skłonił mnie do wykorzystania VMCIContext to kontrolowana przez nas składowa ctx_id (offset 0x10), dzięki niej możemy łatwo zidentyfikować VMCIContext w zwróconej do r-3 pamięci jądra.
Implementacja znajdującego się w tym repozytorium <a href="https://github.com/4337/SAST-DAST-RE/blob/main/VMware-PCI-VMCI-Bus-Device-fv-9.8.16.0-vp-9.8.16.0-build-14168184/VMW.cpp">dowodu koncepcji</a> alokuje 2 struktury VMCIContext, zwalania jedną z alokowanych struktur VMCIContext przez zamknięcie uchwytu do urządzenia.
Następnie alokuje 13 bloków pamięci dynamicznej o rozmiarze 0x90 (rozmiar VMCIContext) za pomocą 0x8103204C i przeszukuje zwróconą pamięć w poszukiwaniu wartości (663133766) VMCIContext->ctx_id zwolnionej instancji. Jeśli wartość zostaje znaleziona to zwraca wartość VMCIContext->off_0x78.
Pierwotnie miała to być wartość VMCIContext->off_0x88 czyli wskaźnik do funkcji 
na którego podstawie można wyliczyć adres bazowy vmci.sys w pamięci i wtedy KASLR przestaje stanowić problem, ale jak już wspomniałem off_0x88 jest równe 0 i nie chce mi się szukać 
miejsca, w którym jest inicajlizowany adresem funkcji.
<br/>
<br/>
<b>[DEMO]</b>
<br/>
<br/>
<a href="https://www.youtube.com/watch?v=zI8ftv-y350" target="_blank">https://www.youtube.com/watch?v=zI8ftv-y350</a>
</li>
</ul>
<br/>
<br/>
<li><a name="0x81032050"><b>ioctl: 0x81032050</b></a></li>
<u>Device:  0x103</u><br/>
<u>Function: 0x814</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów: N/A</u><br/>
<u>Additional requirments: </u> (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB) 
                        ((VMCIContext->off_0x70 & 1) == true)
<br/>
<br/>
IO->SystemBuffer[0] = User-land address<br/>
IO->SystemBuffer[8] = 1 || 2, przy czym 2 jest nieobsługiwany :), procedura tworzy tylko zapis w event logu<br/>
IO->SystemBuffer[0xc] = Rozmiar alokacji pamięci dyn.<br/>
<br/>
Robi właściwie to samo co ioctl:0x8103204C (opcode:8) z tym, że w drugą stronę czyli odczytuje pamięc z r-3 i wypełnia nią struktury vmci.
Tu też kontrolujemy rozmiar alokacji i adres r-3.
<br/>
<br/>
<li><a name="0x810320BC"><b>ioctl: 0x810320BC</b></a></li>
<u>Device:  0x103</u><br/>
<u>Function: 0x82f</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów:  OUT >= 4, IN == 0</u><br/>
<u>Additional requirments: </u> (Irp->CurrentStackLocation->FileObject->FsContext->off_0x8 == 0xB) 
                        ((VMCIContext->off_0x70 & 1) == true)
<br/>
<br/>
Komunikuje się z DMA 
<br/>
<br/>
<li><a name="0x81032054"><b>ioctl: 0x81032054</b></a></li>
<u>Device: 0x103</u><br/>
<u>Function:  0x815</u><br/>
<u>Access: FILE_ANY_ACCESS</u><br/>
<u>Method: METHOD_BUFFERED</u><br/>
<u>Rozmiar bufforów:  OUT >= 4, IN</u><br/>
<u>Additional requirments:</u> None
<br/>
<br/>
Komunikuje się z DMA
<br/>
<br/>
<li><a name="IRP_MJ_CREATE"><b>IRP_MJ_CREATE</b></a></li>
<u>Device: N/A</u><br/>
<u>Function:  N/A</u><br/>
<u>Access: N/A</u><br/>
<u>Method: N/A</u><br/>
<u>Rozmiar bufforów:  N/A</u><br/>
<u>Additional requirments N/A</u><br/>
Handler: NTSTATUS DriverDispatch([in, out] _DEVICE_OBJECT *DeviceObject,[in, out] _IRP *Irp)
<br/>
<br/>
Oczywiście nie ma się co rozwodzić nad pakietem IRP_MJ_CREATE, możecie sobie o nim poczytać w dokumentacji np. tu https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-mj-create. 
W przypadku vmci.sys obsługa IRP_MJ_CREATE najpierw weryfikuje czy powiodło się wyowłanie IoCreateSymbolicLink w proceurze AddDevice, następnie weryfikuje czy 
Irp->CurrentStackLocation->Parameters->Create->DesiredAccess & 0x3FEDFE60. 
<br/>
Jeśli wszystko się powiodło to sprawdzany jest warunek który odpowiada za ustwienie _Irp->CurrentStackLocation->FileObject->FsContext->off_0x10 wartości 0 lub 1.
<br/>
Jeżeli wartość Irp->CurrentStackLocation->Parameters->Create->DesiredAccess jest odpowiednia to weryfikowany jest kolejny warunek który wywołuje funkcje  
RtlCompareUnicodeString("\\VMX", Irp->CurrentStackLocation->FileObject->off_0x58, TRUE) == 0, czyli porównuje czy Irp->CurrentStackLocation->FileObject->FileName jest równy "\VMX".
<s>Nie mam zielonego pojęcia kiedy ten warunek może być spełniony w systemie Windows, być może jest to związane z "internalsami" samego VMWare Workstation, być może z zagnieżdżoną wirtualizacją lub hiper-vizorem, być może jest to "zaślepka", lub korzystają z tego developerzy, albo inne sterowniki, a być może wyjaśnienie jest prostsze niż się wydaje tylko jeszcze muszę parę lat pogrzebać w jądrze Windows.</s>
<br/>
😁 Trochę już mam ... pojęcia <a href="https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/controlling-device-namespace-access">click</a> 😁
<br/>
W każdym razie gdyby jakimś cudem udało się sprawić, aby to porównanie było prawdziwe to kolejnym krokiem jest weryfikacja uprawnień i dopiero kiedy ta weryfikacja 
zakończy się sukcesem to w _Irp->CurrentStackLocation->FileObject->FsContext->off_0x10 zostanie ustawiona wartość 1.
<br/>
<br/>
<a href="#POC">POC</a>

<h3>Wnioski</h3>
Ta historia dość dobrze pokazuje zależności pomiędzy poszczególnymi fragmentami kodu, to znaczy, aby z powodzeniem wywołać określoną funkcję wejścia/wyjścia musimy 
najpierw wywołać inne funkcje w określonej kolejności, wszystkie te wywołania muszą mieć odpowiednio ustawione wartości parametrów. Między innymi dlatego automatyzacja wyszukiwania 
podatności jest trudna i zawsze warto przeprowadzić głęboką analizę rozwiązania, które chcemy atakować. Błędy niezainicjalizowanej pamięci same w sobie 
mogą stwarzać trudności w identyfikacji dla narzędzi automatycznych. Jeśli taka pamięć nie jest wypełniana określonym wzorcem tak jak ma to miejsce w przypadku 
korzystania z ASAN/MSAN lub narzędzi GFlags (ogólnie podsystemu debugowania Windows, ale nie tylko, bo o ile dobrze kojarzę to przynajmniej jeszcze do niedawna
firefox i wersja jmalloc wykorzystywała "bit patterns" nawet w przypadku gdy firefox nie był debugowany) to fuzzer nawet jeśli monitoruje zmiany zwracanych wartości nie będzie w stanie stwierdzić czy zwrócona pamięć jest niezinicjalizowana.

<h3>LINKI</h3>
<a href="https://www.youtube.com/watch?v=zI8ftv-y350" target="_blank">[demo YT]</a><br/>
<a href="https://github.com/4337/SAST-DAST-RE/blob/main/VMware-PCI-VMCI-Bus-Device-fv-9.8.16.0-vp-9.8.16.0-build-14168184/VMW.cpp" target="_blank">[poc C++]</a><br/>
<a href="https://github.com/4337/SAST-DAST-RE/blob/main/VMware-PCI-VMCI-Bus-Device-fv-9.8.16.0-vp-9.8.16.0-build-14168184/vmci_fv-9.8.16.0_vp-9.8.16.0-build-14168184.js" target="_blank">[WinDbg JavaScript]</a><br/>
<a href="https://github.com/4337/SAST-DAST-RE/blob/main/VMware-PCI-VMCI-Bus-Device-fv-9.8.16.0-vp-9.8.16.0-build-14168184/vmci.sys.i64" target="_blank">[IDA DB]</a><br/>
<a href="https://github.com/4337/SAST-DAST-RE/tree/main/VMware-PCI-VMCI-Bus-Device-fv-9.8.16.0-vp-9.8.16.0-build-14168184/IMG" target="_blank">[obrazky]</a><br/>
