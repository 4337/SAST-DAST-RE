<pre>
 ██████   █████    ███████    ███████████   ██████████  
░░██████ ░░███   ███░░░░░███ ░░███░░░░░███ ░░███░░░░███ 
 ░███░███ ░███  ███     ░░███ ░███    ░███  ░███   ░░███
 ░███░░███░███ ░███      ░███ ░██████████   ░███    ░███
 ░███ ░░██████ ░███      ░███ ░███░░░░░███  ░███    ░███
 ░███  ░░█████ ░░███     ███  ░███    ░███  ░███    ███ 
 █████  ░░█████ ░░░███████░   █████   █████ ██████████  
░░░░░    ░░░░░    ░░░░░░░    ░░░░░   ░░░░░ ░░░░░░░░░░   
                                                                                                           
</pre>                                                                             
                                                                                    
                                                                                    

<br/>

Staram się co jakiś czas analizować binarki. Nie mówię, że robię to systematycznie, ale co jakiś czas.<br/>
Ponieważ ostatnio zajmowałem się trochę — na miarę możliwości — sterownikami, które działają w Windowsie, doszedłem do wniosku,<br/>
że dobrze byłoby wrócić chociaż na chwilę do user landu, a ponieważ analizuję zazwyczaj to, <br/>czego używam lub mam styczność na co dzień — w pracy lub w domu — 
wybór padł na NordVPN.<br/>
Klient NordVPN w domyślnej konfiguracji — przynajmniej w subskrypcji, z której korzystam — składa się właściwie z czterech elementów:<br/>
trzech usług systemowych, które działają jako NT SYSTEM, oraz aplikacji r3 no i sterowników.<br/>
Trochę się rozczarowałem, kiedy okazało się, że duża część tych usług została stworzona w C#, ponieważ oznaczało to,<br/> 
że muszę zamienić IDE na dekompilatory i debugery .NET. Pojawiły się jednak powody, dla których postanowiłem przyjrzeć się bliżej 
usługom systemowym NordVPN.<br/>
<br/>

<h3>.IPC</h3>
Kiedy zdekompilujemy odpowiednie moduły NordVPN, relatywnie szybko znajdziemy deklarację klasy IpcRequest.

<pre>
namespace NordSecurity.Communication.Pipes.Common
{
	// Token: 0x020000AF RID: 175
	[NullableContext(1)]
	[Nullable(0)]
	public class IpcRequest : IEquatable<IpcRequest>
	{
		// Token: 0x060003EC RID: 1004 RVA: 0x0000B2C1 File Offset: 0x000094C1
		public IpcRequest(string MessageType, string[] Headers, string Body, string[] Arguments, Guid Id)
		{
			this.MessageType = MessageType;
			this.Headers = Headers;
			this.Body = Body;
			this.Arguments = Arguments;
			this.Id = Id;
			base..ctor();
		}
,,,
</pre>
Konstruktor określa, jakie elementy muszą występować w wiadomości. Trochę niżej znajdziemy informację, że owe komunikaty IPC
są przesyłane w formacie JSON.<br/>
Jeśli natomiast przyjrzymy się klasie IpcRequest jeszcze uważniej, znajdziemy definicję metody TypeMethodCallRequest — jak poniżej.<br/>
<pre>
		public static IpcRequest TypeMethodCallRequest(TypeInformation typeName, string methodName, string[] arguments)
		{
			return new IpcRequest("call", new string[]
			{
				"type:" + typeName.TypeName,
				"method:" + methodName
			}, string.Empty, arguments);
		}
</pre>
To był chyba główny powód, dla którego postanowiłem zostać nieco dłużej z C# i MSIL-em.<br/>
Komunikat "call" wywołuje metodę obiektu wskazanego wewnątrz przesyłanego JSON-a, ze wskazanymi w nim parametrami.<br/>
Gdyby udało nam się go wywołać, prawdopodobnie mielibyśmy nieautoryzowane wykonanie kodu, a być może także eskalację uprawnień. Być może, bo serwer mógłby np. personifikować się<br/>
jako klient i wtedy byśmy nie mieli... No, ale wygląda to całkiem ciekawie.<br/>
<h3>.AUTORYZACJA</h3>
Co więc trzeba zrobić, aby wysłać komunikat "call" do usługi systemowej NordVPN?<br/>
W mojej opinii nawiązywanie "sesji" IPC z usługami z poziomu aplikacji użytkowej jest dość skomplikowane i prawdopodobnie, bazując wyłącznie na<br/>
dekompilacji kodu C#, nie byłbym w stanie tego zaimplementować — no, może gdybym patrzył w ten kod 100 lat.<br/>
Zresztą sama lista potoków Norda nie napawa optymizmem.<br/>

<pre>
C:\Users\echo\Desktop\SysinternalsSuite>pipelist64 | find "nord"
nord-security\v1\NordVpnService\Server            7               -1
nord-security\v1\0426e887f550de29ff30ceee4404925c7eb52e0f136740aa47b6cb5d69cd3c7b          1                1
PSHost.134297255297364204.4644.DefaultAppDomain.nordvpn-service          1                1
nord-security\v1\7e3d793928fb082898226fd3f818d629aa37b1df9acee175d631135b49837e7d          1                1
nord-security\v1\cf3e068ea96c19bca2767422555c95434933ff37a1b91f87a4123cc4b57d7884          1                1
nord-security\v1\708ef9fe1ca32e9de852a2863fa89e77ae83dbc9b9ba71b8bcd5707c2219361f          1                1
nord-security\v1\86f5b3f4ed18c0eba022d912d97a394d6468829bfe2ca27b73af14612dcf4f0e          1                1
nord-security\v1\3abde3f6f39a76079dff4ee3524d5a9091ff2c4636b7dbd78b2fd6561d1740e8          1                1
nord-security\v1\1eda07e8b79692ada6f8a07d256395d7ae2b9ddf680166c23ce369014820e809          1                1
nord-security\v1\3b36b1d532df51e3999541e3f8220372ab8d9203cf43119aced3ae6410c489b4          1                1
nord-security\v1\8201ec7d4fccff3fff2221661aed16d3c7876ee44283a8c738e267a28e8129f7          1                1
nord-security\v1\5f940005a8f45d327101d7e7fc08c7353f6dfe40009ad237216cc09df7e432b5          1                1
nord-security\v1\e05616f958473148cf5b8cf96a1b1bbb8eb99809cb5ef0e849cb1666a859b950          1                1
nord-security\v1\d121325304c4789b4abcf890238090401a3be3660720f703917a03ba683bef81          1                1
nord-security\v1\b6f3e390b52924c2cc7edee35613d2fd6269861c030e2b11a9021e05ac58e1a9          1                1
nord-security\v1\ThreatProtectionService\Server          7               -1
nord-security\v1\0313d44f8451b0b70a53de989bc30b34016fa1c3ae2b8560555258b8effdf2aa          1                1
nord-security\v1\6ce4869de25c9df6f277022fccb59f834b268bbc7abe7b2ed470478784926015          1                1
nord-security\v1\ae6d702089dc6e6fa7f0eff81856ddb5583d864eefd415c5c2f5ee795c7a13b6          1                1
...
...
</pre>

Publiczna komunikacja bez szyfrowania — powiedzmy, że jest to nawiązywanie sesji — wygląda tak:<br/>

<ol>
<li>Łączymy się z wybranym potokiem Norda, a on odsyła nam nazwę nowego potoku.</li>
<li>Tworzymy serwer potoku o nazwie odebranej w kroku pierwszym.</li>
<li>Wysyłamy przez potok naszego serwera ciąg znaków "ack".</li>
<li>Wysyłamy nasz klucz publiczny do potoku Norda z punktu 1.</li>
<li>W odpowiedzi otrzymujemy nazwę potoku "prywatnego" i klucz publiczny serwera.</li>
<li>Łączymy się z potokiem prywatnym.</li>
<li>Cała komunikacja w potoku prywatnym jest szyfrowana.</li>
</ol>

Oczywiście nie każdy może sobie taką sesję nawiązać od tak.<br/>
W celu zweryfikowania, czy proces jest zaufany, Nord wykorzystuje m.in. metodę TryTrust widoczną poniżej.<br/>
<pre>
		public bool TryTrust(int foreignProcessId, SecurityIdentifier foreignOwnerGroup, byte[] foreignPublicKey)
		{
			if (foreignOwnerGroup.IsWellKnown(WellKnownSidType.LocalSystemSid))
			{
				return this._keyStorage.TryAdd(foreignProcessId, this._localKeyPair.GetSharedKey(foreignPublicKey));
			}
			return this.TryUntrustOrContinue(foreignProcessId) && foreignPublicKey.Length != 0 && (this._keyStorage.TryAdd(foreignProcessId, this._localKeyPair.GetSharedKey(foreignPublicKey)) || this._keyStorage.ContainsKey(foreignProcessId));
		}
</pre>
Metoda sprawdza, czy zdalny proces działa w kontekście NT SYSTEM, i w takim wypadku jest on<br/>
"zweryfikowany", a jeśli nie, wywołuje this.TryUntrustOrContinue(foreignProcessId) && foreignPublicKey.Length != 0.<br/>
TryUntrustOrContinue jest o tyle ciekawe, że sprawdza m.in. taki warunek:<br/>
<pre>
bool isAccessDenied = false;
			ProcessSource valueOr = foreignProcessId.GetSource().GetValueOr(delegate(Exception ex)
			{
				isAccessDenied = ex.IsAccessDenied();
				return null;
			});
			if (isAccessDenied)
			{
				return true;
			}
</pre>	
Proces próbuje uzyskać dostęp do zdalnego procesu (strony komunikacji), odczytać jego certyfikat i zweryfikować, czy jest on dla niego OK.
Problem — jeśli dobrze czytam ten kod — polega na tym, że jeśli nie uzyska dostępu do procesu, z którego chce odczytać certyfikat,
to w obsłudze wyjątku ustawia isAccessDenied = ex.IsAccessDenied();, a w konsekwencji zwraca true.
Więc w teorii, jeśli uruchomię sobie proces, a następnie zmienię jego ACL-e na DENY dla NT SYSTEM i Administrators, pozostawiając ALLOW dla 
Owner, to TryUntrustOrContinue zwróci true. Druga część warunku foreignPublicKey.Length != 0 jest spełniona, jeśli uda nam się przesłać 
klucz publiczny.
Teoretycznie więc każdy proces może się komunikować przez IPC z usługami Norda.
W praktyce to nie zadziałało. Szczerze mówiąc, nie pamiętam dlaczego, ale znalazłem inny sposób, aby obejść "restrykcje zaufania" procesów.
<br/>
------

Wykorzystałem oryginalny proces NordVPN.exe i po prostu wstrzyknąłem do niego swój kod. Udało się nawiązać sesję IPC.<br/>

<pre>
[+]. Connected to \\.\pipe\nord-security\v1\ThreatProtectionService\Server
[+]. Read 98 bytes from server pipe 98
[+]. Recivied pipe name \\.\pipe\nord-security\v1\33937ef212e4f57b4d1e4f9b24d7854f0cdb20dd4ebf864be9f0ff4be561993a
[+]. \\.\pipe\nord-security\v1\33937ef212e4f57b4d1e4f9b24d7854f0cdb20dd4ebf864be9f0ff4be561993a server created
[+]. ACK sended to client via \\.\pipe\nord-security\v1\33937ef212e4f57b4d1e4f9b24d7854f0cdb20dd4ebf864be9f0ff4be561993a
[+]. Sending our public key tDJhvn0Fg2aYUORgyE0SgzrgipuziAWPcdrcDsKbGh0=
[*]. Decoded foreign key size: 32 bytes
[+]. Recivied data from \\.\pipe\nord-security\v1\NordVpnService\Server
[+]. Foreign public key GfOu1SrNxOND7A7wk5BC9WxOhMytBzkWpwmSN7IksEU=
[+]. Shared key VvmQayLocTvgeZos+3HYCvtohC83biAK9cGoPDPon/I=
[+]. New pipe name \\.\pipe\nord-security\v1\d2fcbee591b3158e3e3b57c65346b97f838171ced06d4bd6e3e9b0e676ac0f6d
[+]. Connected to the private pipe \\.\pipe\nord-security\v1\d2fcbee591b3158e3e3b57c65346b97f838171ced06d4bd6e3e9b0e676ac0f6d
</pre>
<br/>
Problem w tym, że na etapie implementacji crypto zauważyłem, że klucz współdzielony różni się po mojej stronie<br/>
od tego, z którego korzysta druga strona.
<br/>

<h3>.CRYPTO</h3>

Analizując źródła, można ustalić, że komunikacja JSON/IPC (te prywatne potoki) jest szyfrowana przy użyciu ChaCha20, ECDH i Curve25519.
Początkowo implementowałem krypto w WINAPI / Cryptography API: Next Generation, ale zwątpiłem.
Między libsodium a WinAPI podobno występują różnice w implementacjach. Windows ma też te swoje formaty kluczy (z nagłówkami), z kolei 
Nord operuje na formacie raw i takie tam.<br/>
Sądziłem, że to przez to klucze się nie zgadzają. Napisałem więc wszystko z użyciem libsodium.<br/>
I... okazało się, że klucze nadal są od siebie różne.<br/>

Okazało się, że Nord wymienia klucze dla każdej "sesji" i później chyba już nie da się ich zmienić.<br/>
Więc mój klucz publiczny był po prostu ignorowany.<br/>

---
Wymiana klucza
---
Rozwiązanie tego problemu okazało się dość proste: terminacja procesu NordVPN.exe, jeśli jest uruchomiony, uruchomienie procesu NordVPN.exe<br/>
w trybie SUSPENDED, tak aby nie zdążył wymienić się kluczami kryptograficznymi z usługami systemowymi, i przesłanie własnego klucza.<br/>

<h3>.JSON IPC</h3>

Poszczególne komunikaty można sobie wygenerować z wykorzystaniem zdekompilowanych źródeł i Gemini AI lub jakiegoś innego AI.<br/>

<h4>.KOMUNIKAT "call"</h4>

Po wysłaniu komunikatu "call" do usług Windows otrzymamy komunikaty błędów, z których możemy wywnioskować m.in., że 
Nord uniemożliwia wywołanie dowolnych metod .NET za pośrednictwem komunikatu "call". Powinniśmy przeprowadzić szczegółową analizę m.in. metod i klas
NordSecurity.Communication.Pipes.Server.ServeTypeMethodCall.Serve oraz NordSecurity.Communication.Pipes.Server.NamedPipeServer.Serve, aby zrozumieć,
jakie obiekty i metody są dozwolone w komunikacie "call".<br/>

<pre>

"{\"MessageType\":\"call\",\"Headers\":[\"type:System.Diagnostics.Process\",\"method:Start\"],\"Body\":\"\",\"Arguments\":[\"cmd.exe\"],\"Id\":\"00000000-0000-0000-0000-000000000000\"}
				 
ex:

System.NotImplementedException: Type 'System.Diagnostics.Process' is not implemented by the Server. Make sure 'System.Diagnostics.Process' implementation is included when constructing the 'NamedPipeServer'.
   at NordSecurity.Communication.Pipes.Server.ServeTypeMethodCall.Serve(IpcRequest request, CancellationToken token)
   at NordSecurity.Communication.Pipes.Server.NamedPipeServer.Serve(IpcRequest request, CancellationToken stoppingToken)
		 
ex:

[09:57:56.159] [ERR] [58] [NamedPipeServer] pipe 'PipePublicName { BaseName = nord-security\v1\NordVpnService\Server }': Failed serving request. Type: call; headers: ["type:System.Diagnostics.Process","method:Start"]; error: Type 'System.Diagnostics.Process' is not implemented by the Server. Make sure 'System.Diagnostics.Process' implementation is included when constructing the 'NamedPipeServer'..
System.NotImplementedException: Type 'System.Diagnostics.Process' is not implemented by the Server. Make sure 'System.Diagnostics.Process' implementation is included when constructing the 'NamedPipeServer'.
   at NordSecurity.Communication.Pipes.Server.ServeTypeMethodCall.Serve(IpcRequest request, CancellationToken token)
   at NordSecurity.Communication.Pipes.Server.NamedPipeServer.Serve(IpcRequest request, CancellationToken stoppingToken)
</pre>
---
<h4>.KOMUNIKAT "post"</h4>
<pre>
"{\"MessageType\":\"post\",\"Headers\":[\"type:ValueTuple<MarkedValue<ThreatProtectionServiceLogsRequest>, MarkedValue<String>>\"],\"Body\":\"{\\\"Id\\\":\\\"666ac3a2-a16a-48d6-8556-d85b0c30c666\\\",\\\"SourceProcessID\\\":0,\\\"SourcePipeName\\\":\\\"EvilNamedPipeServer\\\",\\\"Value\\\":{}}\",\"Arguments\":[\"\"],\"Id\":\"786ac3a2-a16a-48d6-8556-d85b0c30c746\"}",shared_key);
</pre>
Za pomocą komunikatu "post" możemy nakłonić usługę Windows/Nord, aby nawiązała połączenie ze wskazanym przez nas serwerem potoku nazwanego.<br/>
Teoretycznie, gdybyśmy mieli uprawnienie SeImpersonatePrivilege, usługa Norda wysłałaby do nas jakieś dane i zostałyby spełnione inne wymagania,<br/>
to moglibyśmy się personifikować jako usługa Windows, ale nie możemy, choćby z tego powodu, że klient do nas nie pisze.<br/>
No i oczywiście nie mamy SeImpersonatePrivilege.<br/>

<h3>.ATTACK SURFACE</h3>
Poza komunikatami wysyłanymi przez "priv pipe" w formacie JSON, mamy oczywiście znacznie więcej wektorów ataku.
Mamy na przykład IPC API do zarządzania poszczególnymi funkcjonalnościami NordThreadProtection<br/>
<pre>
ThreatProtectionService.Api.Ipc.FileProtectionIpc.UnquarantineFile(FileProtectionCommands.UnquarantineFile) : Task @0600000B<br/>
</pre>
Mamy usługę aktualizacji Norda, która prawdopodobnie korzysta m.in. z gRPC.<br/> 
Mamy gniazda sieciowe, które nasłuchują na loopbacku, i wiele innych rzeczy.<br/>

<h3>Wniosek</h3>

Nie wiem. <br/>
Warto uczyć się języków programowania, bo bez tego nie ma hakowania.<br/>
Jeśli programujesz w C#/.NET, to ogarnij sobie również MSIL.<br/>
Research w Nordzie wymaga sporo motywacji i cierpliwości, których ja nie mam.<br/>
No, ale drzwi do komunikacji przez named pipe zostały uchylone [lol].

<h3>Linki</h3>

<a href="">Nord named pipe client [source code]</a>  






<!--
<img src="./Nord.jpg" alt="Nord" style="position:relative;top:0;left:0;">
-->
