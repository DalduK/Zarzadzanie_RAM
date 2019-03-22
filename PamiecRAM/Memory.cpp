#include "Memory.h"

using namespace std;
Memory s;
map<int, Strona*> proc;
map<int, string> adresystr;
vector<int> PIDList;

const void Memory::PrzeStroBuffDoPam(int PID, int nr, Strona *g) {

	STRON * TabStron = g->PobTabStronic();
	ZapewnijWolnaRame(); //sprawiamy, ze na pewno pojawi sie wolna ramka pamieci

	int NowoZajRama = PobierzWolRamePamieci();
	int NowoWolnaRamaBuff = TabStron[nr].RamaZajeta; //zmiana stronicy z SF do RAM
	WolneRamkiBuffera.push_front(NowoWolnaRamaBuff);
	PrzeZawBuffDoPam(NowoWolnaRamaBuff, NowoZajRama); //ustawiamy indeks aktualnej strony w RAM

	TabStron[nr].RamaZajeta = NowoZajRama; //nowa ramka w RAM
	TabStron[nr].wPam = true; //strona jest w RAM
	if (PID != 0) {
		DodajDoFifo(FIFO_entry(PID, nr)); //dodanie procesu do FIFO
	}
}
//
const void Memory::PrzeStroPamDoBuff(int PID, int nr, Strona *g) {
	STRON * TabStron = g->PobTabStronic();

	if (WolneRamkiBuffera.empty()) //jezeli brak wolnych ramek w pliku wymiany, to mamy problem
	{
		cout << "Out_off_mem"; // nie powinno nigdy sie stac
		return;
	}
	
	int NowaWolnaRama = TabStron[nr].RamaZajeta; //jezeli trafimy na strone w pamieci RAM to wysylamy ja do pliku wymiany
	WolneRamki.push_back(NowaWolnaRama); //wolna ramke ram dodajemy do listy wolnych ramek

	int NowoZajetaRamaBuff = PobierzWolnaRameBuff();

	PrzeZawPamDoBuff(NowaWolnaRama, NowoZajetaRamaBuff); //przenosimy strone

	TabStron[nr].RamaZajeta = NowoZajetaRamaBuff; //nowa ramka w SwapFile
	TabStron[nr].wPam = false; //strona nie jest w RAM
}
//
const void Memory::PrzeZawPamDoBuff(int RamaPam, int RamaPWy) {
	int IndeksBuff = RamaPWy * WIE_RAM;
	int IndeksPam = RamaPam * WIE_RAM;
	int IndeksKBuff = IndeksBuff + WIE_RAM;
	int IndeksKPam = IndeksPam + WIE_RAM;

	while (IndeksPam < IndeksKPam && IndeksBuff < IndeksKBuff)
	{
		Buffer[IndeksBuff] = RAM[IndeksPam];
		RAM[IndeksPam] = ' ';
		IndeksPam++;
		IndeksBuff++;
	}
}
//
const void Memory::PrzeZawBuffDoPam(int RamaPWy, int RamaPam) {
	int IndeksBuff = RamaPWy * WIE_RAM;
	int IndeksPam = RamaPam * WIE_RAM;
	int IndeksKBuff = IndeksBuff + WIE_RAM;
	int IndeksKPam = IndeksPam + WIE_RAM;

	while (IndeksPam < IndeksKPam && IndeksBuff < IndeksKBuff)
	{
		RAM[IndeksPam] = Buffer[IndeksBuff];
		Buffer[IndeksBuff] = ' ';
		IndeksPam++;
		IndeksBuff++;
	}
}
//
const void Memory::ZapiszDoPlikuWym(int nr, string Zaw) {
	int IndeksBuff = nr * WIE_RAM;
	int IndeksStringa = 0;

	while (IndeksStringa < Zaw.size())
	{
		Buffer[IndeksBuff] = Zaw[IndeksStringa];
		IndeksBuff++;
		IndeksStringa++;
	}
}
//
const int Memory::ObliczOffset(int addr1) {
	return addr1 & 0x0f;
}
//
const int Memory::ObliczNumerStrony(int addr1) {
	return (addr1 & 0xf0) >> 4;
}
//
const int Memory::ObliczAdresFizyczny(int PID, int addr1, Strona *g) {
	STRON * TablicaStronic = g->PobTabStronic();
	int NumerStrony = ObliczNumerStrony(addr1);
	int offset = ObliczOffset(addr1);
	if (TablicaStronic[NumerStrony].wPam) {
		int NumerRamy = TablicaStronic[NumerStrony].RamaZajeta;
		return NumerRamy * WIE_RAM + offset;
	}
	else {
		return 99999;
	}
}
//
const int Memory::ObliczTabliceStronic(int siBytes) {
	double wielkoscRamy = static_cast<double>(WIE_RAM);
	double CalkowitaWielkosc = static_cast<double>(siBytes);
	double wynik = ceil(CalkowitaWielkosc / wielkoscRamy);
	return static_cast<int>(wynik);
}
//
const char Memory::OdczytajZPamieci(int PID, int addr1, Strona *g) {
	if (BrakMieWPowAdr(PID, addr1, g)) {
		cout << "Out of Range" << endl;
		return ' ';
	}
	ZapewnijStroneWPamieci(PID, addr1, g);
	int AddrP = ObliczAdresFizyczny(PID, addr1, g);
	return RAM[AddrP];
}
//
const void Memory::ZapiszWPamieci(int PID, int addr1, char element, Strona *g) {
	if (BrakMieWPowAdr(PID, addr1, g)) {
		cout << "Out of Range" << endl;
		return;
	}
	ZapewnijStroneWPamieci(PID, addr1, g);
	int p_Addr = ObliczAdresFizyczny(PID, addr1, g);
	RAM[p_Addr] = element;
}
//
const void Memory::ZapewnijStroneWPamieci(int PID, int AdresLogiczny, Strona *g) {
	STRON * TabelaStron = g->PobTabStronic();

	int NumerPam = ObliczNumerStrony(AdresLogiczny);

	if (TabelaStron[NumerPam].wPam == false) {
		PrzeStroBuffDoPam(PID, NumerPam, g);
	}
}
//
const int Memory::PobierzWolnaRameBuff() {
	int WolnaRama = WolneRamkiBuffera.front();
	WolneRamkiBuffera.pop_front();
	return WolnaRama;
}
//
const int Memory::PobierzWolRamePamieci() {
	int WolnaRama = WolneRamki.front();
	WolneRamki.pop_front();
	return WolnaRama;
}
//
const void Memory::ZapewnijWolnaRame() {
	while (WolneRamki.empty())
	{
		//nie ma wolnych ramek wiec usuwamy metoda FIFO najstarszy Process
		FIFO_entry victim = FIFO.front();
		FIFO.pop_front();
		PrzeStroPamDoBuff(victim.PID, victim.pageNumber, proc[victim.PID]);
	}
}
//
const void Memory::DodajDoFifo(FIFO_entry entry) {
	FIFO.push_back(entry);
}
//
const bool Memory::BrakMieWPowAdr(int PID, int AdresLogiczny, Strona *g) {
	int NumerStrony = ObliczNumerStrony(AdresLogiczny);
	if (NumerStrony >= g->PobWielTabStronic())
		return true;
	else
		return false;
}
//
const void Memory::PrzydzialPamieci(int PID, string proces, int size, Strona *g) {
	//ilosc stronic potrzebynch do alokacji
	int WieTabStron = ObliczTabliceStronic(size);
	STRON * TablicaStron = new STRON[WieTabStron];
	int PoczatekStr = 0;

	for (int NrStr = 0; NrStr < WieTabStron; NrStr++) {
		//jezeli plik wymiany jest pelny
		if (WolneRamkiBuffera.empty())
		{
			cout << "Out_of_mem" << endl;//blad, nie da sie zaalokowac
			return;
		}
		int NowoZajetaRamaBuff = PobierzWolnaRameBuff();

		TablicaStron[NrStr].RamaZajeta = NowoZajetaRamaBuff;
		TablicaStron[NrStr].wPam = false;

		string pageContent = proces.substr(PoczatekStr, WIE_RAM);
		proces.erase(PoczatekStr, WIE_RAM);
		ZapiszDoPlikuWym(NowoZajetaRamaBuff, pageContent);

	}
	g->UstTabStronic(TablicaStron);
	g->UstWielTabStronic(WieTabStron);
}

const string Memory::odczytajString(int PID, int Addr, Strona *g) {
	int WielkoscTablicyStron = g->PobWielTabStronic();
	string result = "";
	char byte;
	int LimitAddr = WielkoscTablicyStron * WIE_RAM;
	//powtarzej dopoki adres logiczny nie wskazuje na " " i nie przekroczyl logicznej pamieci
	while (Addr < LimitAddr)
	{
		byte = OdczytajZPamieci(PID, Addr, g);
		if (byte == ' ')
			break;
		result += byte;
		Addr++;
	}
	return result;
}
//
const void Memory::zapiszString(int PID, int Addr, string Zaw, Strona *g) {
	for (int IndeksZaw = 0; IndeksZaw < Zaw.size(); IndeksZaw++)
	{
		ZapiszWPamieci(PID, Addr + IndeksZaw, Zaw[IndeksZaw], g);
	}
}
//
const void Memory::WypiszZasobPamieci(int DoWypis) {
	if (DoWypis <= 0 || DoWypis > ILOSC_RAM)
		DoWypis = ILOSC_RAM;

	for (int i = 0; i < DoWypis; i++)
	{
		WydrukujRame(i);
	}
}
//
const void Memory::WpiszZasobPamDoBuff(int nr, string zasobPam)
{
	int IndeksPlikuWym = nr * WIE_RAM;
	int IndeksSTR = 0;

	while (IndeksSTR < zasobPam.size())
	{
		Buffer[IndeksPlikuWym] = RAM[IndeksPlikuWym];
		IndeksPlikuWym++;
		IndeksSTR++;
	}
}
const void Memory::WydrukujProcesy(int PID, bool wRamie, Strona *g) {

	if (PID == 0)
	{
		cout << "Nie ma takiego procesu.\n";
		return;
	}

	STRON * StronProc = g->PobTabStronic();
	int WieStronProc = g->PobWielTabStronic();

	cout << " --- Pamiec zarezerwowana przez proces: " << PID << " ---\n";
	for (int i = 0; i < WieStronProc; i++)
	{
		if (StronProc[i].wPam)
		{
			WydrukujRame(StronProc[i].RamaZajeta, i); //Jezeli strona znajduje sie w pamieci
		}
	}
}
//
const void Memory::WydrukujFIFO() {
	cout << "Stan algorytmu FIFO. Element po lewej zostanie usuniety, w przypadku braku miejsca w pamieci." << endl
		<< "Format: [{PID_procesu},{numer strony}]" << endl;
	for (auto entry : FIFO)
	{

		cout << "[" << entry.PID << "," << entry.pageNumber << "] ";
	}
	cout << endl;
}
//
const bool Memory::CzyAdrWPowAdresss(int AddrLog, Strona *g) {
	int NrStrony = ObliczNumerStrony(AddrLog);
	if (NrStrony < g->PobWielTabStronic())
		return true;
	else
		return false;
}
//
const bool Memory::CzyZasiegAdrWPowAdres(int AddrLog, int zasieg, Strona *g) {
	int AddrKon = zasieg + AddrLog;
	while (AddrLog < AddrKon)
	{
		if (CzyAdrWPowAdresss(AddrLog, g))
			return false;
		AddrLog++;
	}
	return true;
}

//
const void Memory::WydrukujRame(int RamaNr, int StronaNr)
{
	int addr = RamaNr * WIE_RAM;
	string space = "        ";
	cout << "Ramka nr: " << RamaNr;
	if (StronaNr >= 0)
	{
		cout << ", zawiera strone nr: " << StronaNr;
	}
	cout << endl;

	for (int i = 0; i < WIE_RAM; i++)
	{
		if (addr + i < 10)
			printf("  %d  |", addr + i);
		else
			printf(" %3d |", addr + i);
	}
	cout << endl;
	// wyzwietla zawartosc RAMu
	addr = RamaNr * WIE_RAM;

	for (int i = 0; i < WIE_RAM; i++)
	{
		printf("  %c  |", RAM[addr + i]);

	}
	cout << endl;
	// ----------------------------------------------------------------------
	// wyzwietla spod ramki

}

const void Memory::WyczyscRamPlikuWym(int NrRamy) {
	int IndeksRamy = NrRamy * WIE_RAM;
	int IndeksKRamy = NrRamy + WIE_RAM;
	while (IndeksRamy < IndeksKRamy)
	{
		Buffer[IndeksKRamy] = ' ';
		IndeksRamy++;
	}
}
//
const void Memory::zwolnieniePamieci(int PID, Strona *g) {
	STRON * TablicaStron = g->PobTabStronic();
	int WTablicyStron = g->PobWielTabStronic();
	int NrStrony = 0;

	while (NrStrony < WTablicyStron) //usuwanie informacji z pamieci i stronic
	{
		if (TablicaStron[NrStrony].wPam) //wysylamy ramki z pamieci do pliku wymiany
		{
			PrzeStroPamDoBuff(PID, NrStrony, g);
		}

		FIFO.remove_if([PID](const FIFO_entry &victim) {return victim.PID == PID; }); //usuwamy z kolejki FIFO pcb

		int frameToFree = TablicaStron[NrStrony].RamaZajeta;
		WyczyscRamPlikuWym(frameToFree);
		WolneRamkiBuffera.push_back(frameToFree);

		NrStrony++;
	}
	delete[] TablicaStron;


	g->UstTabStronic(nullptr);
	g->UstWielTabStronic(0);
}
//
const void Memory::WydrukujTabliceStronic(int PID, Strona *g) {
	if (PID == 0)
	{
		//rlutil::setColor(rlutil::LIGHTRED);
		cout << "Nie ma takiego procesu.\n";
		//rlutil::setColor(rlutil::LIGHTGREEN);
		return;
	}

	STRON * TablicaStron = g->PobTabStronic();
	int WTablicyStron = g->PobWielTabStronic();

	cout << " --- Tablica stron procesu: " << PID << " ---\n";
	//cout << " Nr strony " << (char)CharTable::VL << " Nr  ramki " << (char)CharTable::VL << " Czy w pamieci?\n";

	for (int page = 0; page < WTablicyStron; page++)
	{
		cout << "     " << page;
		for (int i = 0; i < (6 - to_string(page).size()); i++)
		{
			cout << " ";
		}
		//cout << (char)CharTable::VL;
		if (TablicaStron[page].wPam == true)
		{
			cout << "     " << TablicaStron[page].RamaZajeta << "     " << (char)CharTable::VL;
			cout << "      Tak      \n";
		}
		else
		{
			cout << "           " << (char)CharTable::VL;
			cout << "      Nie      \n";
		}
	}

}
void Memory::PIDproces(int PID, string plik, Strona *g) {
	int ADDR = 0;
	PrzydzialPamieci(PID, plik, plik.length(), g);
	/*ADDR = WolneRamki.front();
	WolneRamki.pop_front();*/
	for (int IndeksPlik = 0; IndeksPlik < plik.length(); IndeksPlik++)
	{
		ZapiszWPamieci(PID, ADDR + IndeksPlik, plik[IndeksPlik], g);
	}
}
char Memory::wypiszZnak(int PID, int Addr) {
	return 'a';
}

char Memory::wypiszZnak(int Addr) {
	if (Addr >= 0 && Addr <= 255) {
		return RAM[Addr];
	}
	else {
		cout << "Zly Adres";
	}
}
void Memory::wypiszString(int PID, Strona *m) {
	STRON * TablicaStron = m->PobTabStronic();
	int WieTablicyStron = m->PobWielTabStronic();
	string temp;
	char byte;
	for (int page = 0; page < WieTablicyStron; page++)
	{
		if (TablicaStron[page].wPam == true)
		{
			for (int i = TablicaStron[page].RamaZajeta*16; i < TablicaStron[page].RamaZajeta * 16 + 16; i++) {
				byte = RAM[i];
				temp += byte;
			}
		}
	}
	cout << "String zawarty w pamieci, procesu :" << PID << " to : " << temp;
}
void Memory::wypiszStringAA(int Addr, int Addr2) {
	string temp;
	char byte;
	int h = Addr;
	while (Addr < Addr2) {
		byte = RAM[Addr];
		temp += byte;
		Addr++;
	}
	cout << "String zawarty w pamieci, od Adresu" << h << " do adresu : " << Addr2 << " to : " << temp;
}

void Memory::wpiszString(string zawartosc, int Addr) {
	for (int i = Addr; i < Addr + zawartosc.length(); i++) {
		RAM[i] = ' ';
		if (i > 255) {
			Addr = 16;
		}
	}
	int IndeksPlik = 0;
	
	for (int i = Addr; i < Addr + zawartosc.length(); i++) {
		RAM[i] = zawartosc[IndeksPlik];
		IndeksPlik++;
		if (i > 255) {
			Addr = 16;
		}
	}	
}

void Memory::clear() {
	
	for (int i = 16; i < 256; i++) {
		RAM[i] = ' ';
	}
}






Memory::Memory()
{
	RAM = new char[WIE_PAM];
	Buffer = new char[WIE_P_WYMIANY];
	for (int i = 0; i < WIE_PAM; i++)
	{
		RAM[i] = ' ';
	}
	for (int i = 0; i < ILOSC_RAM; i++)
		WolneRamki.push_back(i);
	for (int i = 0; i < WIE_P_WYMIANY; i++)
	{
		Buffer[i] = ' ';
	}
	for (int i = 0; i < ILOSC_RAM_P_WYMIANY; i++)
	{
		WolneRamkiBuffera.push_back(i);
	}
	Strona *g = new Strona;
	s.PIDproces(0, "JP 0", g);
}
Memory::~Memory()
{
	delete[] RAM;
	delete[] Buffer;
}
int main()
{
	int x;
	std::string ps = "MV 3 4 CM RM 83 MR 0 83 OF fib XR 0 DR 3 JZ 64 CLF CP p3 1 JP 7 CLF SM 79 3 SP END ";
	std::string ps1 = "MV 2 4 MV 0 0 MV 1 1 EL 2 2 JZ 40 JP 52 MV 0 1 RT 0 ET MC 3 0 AD 0 1 MC 1 3 MW 0 124 SM 124 2 CM RM 129 DR 2 JZ 121 JE 0 SP";
	std::string ps2 = "MV 0 0 JP 16 MV 0 1 JP 32 MV 0 2 JP 48 MV 0 3 JP 64 MV 0 4 JP 80 MV 0 5 JP 96 MV 0 6 JP 112 MV 0 7 JP 128 MV 0 8 JP 144 MV 0 9 JP 160 SP";
	;
	map<int, string> procesy;
	map<int, int> gogo;
	string h = ("");
	cout << " Pomoc " << endl;
	std::cout << "1. Stworz proces." << std::endl;
	std::cout << "2. Pokaz pamiec." << std::endl;
	std::cout << "3. Pokaz Fifo." << std::endl;
	std::cout << "4. Tablica Stronic." << std::endl;
	std::cout << "5. Usun proces." << std::endl;
	std::cout << "6. Wydrukuj procesy." << std::endl;
	std::cout << "7. Translacja Adresow." << std::endl;
	std::cout << "8. Wpisz String." << std::endl;
	std::cout << "9. Wypisz Znak pod danym adresem fizycznym" << std::endl;
	std::cout << "A. Wypisz Znak pod danym adresem logicznym danego procesu" << std::endl;
	std::cout << "B. Wpisz do ramu caly program" << std::endl;
	std::cout << "C. GO." << std::endl;
	std::cout << "D. Wypisz zawartosc programu w pamieci." << std::endl;
	std::cout << "E. Wypisz zawartosc w pamieci od Adresu do adresu" << std::endl;
	std::cout << "F. Wpisywanie stringu pod adresem (niepolecane)" << std::endl;
	std::cout << "G. Wyczysc ram" << std::endl;
	regex wzorzec("[1-7ABCDEFG]{1}");
	regex liczby("[1-9][0-9]*");
	regex liczby2("[0-9]*");

	while (h != "k") {
		while (!(regex_match(h.begin(), h.end(), wzorzec))) {
			std::cin >> h;


			if (h == "1") {
				string Pid1;
				int Pid;
				std::string name;

				std::cout << "Podaj PID: ";
				std::cin >> Pid1;
				if ((regex_match(Pid1.begin(), Pid1.end(), liczby))) {
					Pid = atoi(Pid1.c_str());

					while (Pid == 0) {
						cout << "niepoprawne PID , podaj ponownie" << endl;
						std::cin >> Pid;
					}
					while (find(PIDList.begin(), PIDList.end(), Pid) != PIDList.end()) {
						cout << "jest Proces o podanym ID, podaj inne ID" << endl;
						std::cin >> Pid;
					}
					std::cout << "Podaj nazwe programu: ";
					std::cin >> name;
					if (name == "ps") {
						Strona *g1 = new Strona;
						s.PrzydzialPamieci(Pid, ps, ps.length(), g1);
						PIDList.push_back(Pid);
						procesy.insert(pair<int, string>(Pid, ps));
						proc.insert(pair<int, Strona*>(Pid, g1));
					}
					else if (name == "ps1") {
						Strona *g2 = new Strona;
						s.PrzydzialPamieci(Pid, ps1, ps1.length(), g2);
						PIDList.push_back(Pid);
						procesy.insert(pair<int, string>(Pid, ps1));
						proc.insert(pair<int, Strona*>(Pid, g2));
					}
					else if (name == "ps2") {
						Strona *g3 = new Strona;
						s.PrzydzialPamieci(Pid, ps2, ps2.length(), g3);
						PIDList.push_back(Pid);
						procesy.insert(pair<int, string>(Pid, ps2));
						proc.insert(pair<int, Strona*>(Pid, g3));
					}
					else {
						cout << "nie ma takiego pliku" << endl;
					}
					std::cout << "Utworzono proces " << name << std::endl;
				}
				else {
					std::cout << "Niepoprawna wartosc" << std::endl;

				}

			}
			if (h == "2") {
				s.WypiszZasobPamieci();
			}
			if (h == "3") {
				s.WydrukujFIFO();
			}
			if (h == "4") {
				string Pid1;
				int Pid;
				std::cout << "Podaj PID: ";
				std::cin >> Pid1;
				if ((regex_match(Pid1.begin(), Pid1.end(), liczby))) {
					Pid = atoi(Pid1.c_str());
					if (Pid == 0) {
						cout << "niepoprawne PID , podaj ponownie" << endl;
						std::cin >> Pid;
					}
					else if (!(find(PIDList.begin(), PIDList.end(), Pid) != PIDList.end())) {
						cout << "Nie ma danego procesu, prosze podac proces ktory istnieje" << endl;
					}
					else {
						s.WydrukujTabliceStronic(Pid, proc[Pid]);
					}
					Pid = 0;
				}
				else {
					std::cout << "Niepoprawna wartosc" << std::endl;
				}
			}
			if (h == "5") {
				string Pid1;
				int Pid;
				std::cout << "Podaj PID usuwanego procesu";
				std::cin >> Pid1;
				if ((regex_match(Pid1.begin(), Pid1.end(), liczby))) {
					Pid = atoi(Pid1.c_str());

					if (find(PIDList.begin(), PIDList.end(), Pid) != PIDList.end()) {
						s.zwolnieniePamieci(Pid, proc[Pid]);
						PIDList.erase(find(PIDList.begin(), PIDList.end(), Pid));
					}
				}
			}
			if (h == "6") {
				for (int i = 0; i < PIDList.size(); i++) {
					int pidtemp = PIDList[i];
					s.WydrukujProcesy(pidtemp, true, proc[pidtemp]);
				}
			}
			if (h == "7") {
				string Pid1;
				int Pid;
				std::cout << "Podaj PID: ";
				std::cin >> Pid1;
				if ((regex_match(Pid1.begin(), Pid1.end(), liczby))) {
					Pid = atoi(Pid1.c_str());
					if (!(find(PIDList.begin(), PIDList.end(), Pid) != PIDList.end())) {
						cout << "brak podanego id";
					}
					else {
						int addr1;
						string address;
						std::cin >> address;
						if ((regex_match(address.begin(), address.end(), liczby))) {
							addr1 = atoi(address.c_str());
							int h = s.ObliczAdresFizyczny(Pid, addr1, proc[Pid]);
							if (h == addr1) {
								cout << "Podany adres nie istnieje" << endl;
							}
							else if (h > 255) {
								cout << "Podany adres nie istnieje" << endl;
							}
							else {
								cout << h << endl;
							}
						}
					}
				}
				else {
					cout << "zle wpisano adres";
				}
			}
			if (h == "8") {
				cout << "Podaj PID i wpisz po spacji Stringa ktorego chcesz wpisac" << endl;
				int Pid;
				cin >> Pid;
				string wpisz;
				getline(cin, wpisz);
				wpisz = wpisz.substr(1, wpisz.length());
				Strona *g1 = new Strona;
				s.PrzydzialPamieci(Pid,wpisz,wpisz.length(),g1);
				for (int i = 0; i < wpisz.size(); i++) {
					s.odczytajString(Pid, i, g1);
				}
				PIDList.push_back(Pid);
				procesy.insert(pair<int, string>(Pid, ps));
				proc.insert(pair<int, Strona*>(Pid, g1));
			}
			if (h == "9") {
				int addr1;
				string address;
				cout << "podaj adres fizyczny ktory chcesz wypisac : ";
				cin >> address;
				cout << endl;
				if ((regex_match(address.begin(), address.end(), liczby2))) {
					addr1 = atoi(address.c_str());
					cout << " _ " << endl;
					cout << "|" << s.wypiszZnak(addr1) << "|" << endl;
					cout << " _ " << endl;
				}
			}
			if (h == "A") {
				string Pid1;
				int Pid;
				std::cout << "Podaj PID: ";
				std::cin >> Pid1;
				cout << endl;
				if ((regex_match(Pid1.begin(), Pid1.end(), liczby))) {
					Pid = atoi(Pid1.c_str());
					if (!(find(PIDList.begin(), PIDList.end(), Pid) != PIDList.end())) {
						cout << "brak podanego id";
					}
					else {
						int addr1;
						string address;
						std::cin >> address;
						cout << endl;
						if ((regex_match(address.begin(), address.end(), liczby))) {
							addr1 = atoi(address.c_str());
							if(!(s.ObliczAdresFizyczny(Pid, addr1, proc[Pid])>256))
							cout << " _ " << endl;
							cout << "|" << s.wypiszZnak(s.ObliczAdresFizyczny(Pid, addr1, proc[Pid])) << "|" << endl;
							cout << " _ " << endl;
						}
					}
				}
			}
			if (h == "B") {
				string Pid1;
				int Pid;
				cin >> Pid1;
				if ((regex_match(Pid1.begin(), Pid1.end(), liczby))) {
					Pid = atoi(Pid1.c_str());
					Strona *m = proc[Pid];
					string temp;
					if (procesy[Pid].compare("ps")) {
						for (int i = 0; i < ps.size(); i++) {
							temp = s.odczytajString(Pid, i, m);
							m->str += temp;
							temp = "";
						}
					}
					if (procesy[Pid].compare("ps1")) {
						for (int i = 0; i < ps1.size(); i++) {
							temp = s.odczytajString(Pid, i, m);
							m->str += temp;
							temp = "";
						}
					}
					if (procesy[Pid].compare("ps2")) {
						for (int i = 0; i < ps2.size(); i++) {
							temp = s.odczytajString(Pid, i, m);
							m->str += temp;
							temp = "";
						}
					}
				}
			}
			if (h == "C") {
				string Pid1;
				string temp;
				int Pid;
				cin >> Pid1;
				if ((regex_match(Pid1.begin(), Pid1.end(), liczby))) {
					Pid = atoi(Pid1.c_str());
					if (procesy[Pid].compare("ps")) {
						Strona *m = proc[Pid];
						s.odczytajString(Pid, m->licznik * 16, m);							
						(m->licznik)++;
					}
					else if (procesy.at(Pid).compare("ps1")) {
						Strona *m = proc[Pid];
						s.odczytajString(Pid, m->licznik * 16, m);
						(m->licznik)++;
					}
					else if (procesy[Pid].compare("ps2")) {
						Strona *m = proc[Pid];
						s.odczytajString(Pid, m->licznik * 16, m);
						(m->licznik)++;
					}

				}
				else {
					std::cout << "Niepoprawna wartosc" << std::endl;
				}
			}
			if (h == "D") {
				string Pid1;
				int Pid;
				cin >> Pid1;
				if ((regex_match(Pid1.begin(), Pid1.end(), liczby))) {
					Pid = atoi(Pid1.c_str());
					if (!(find(PIDList.begin(), PIDList.end(), Pid) != PIDList.end())) {
						cout << "Brak podanego procesu" << endl;
					}
					else {
						
						Strona *m = proc[Pid];
						s.wypiszString(Pid, m);
					}
				}
			}
			if (h == "E") {
				int Addr1;
				cin >> Addr1;
				int Addr2;
				cin >> Addr2;
				s.wypiszStringAA(Addr1, Addr2);
			}
			if (h == "F") {
				int Addr;
				cin >> Addr;
				string wpisz;
				getline(cin, wpisz);
				wpisz = wpisz.substr(1,wpisz.length());
				if (wpisz.length() > 255 - 16) {
					cout << "string zbyt wielki" << endl;
					break;
				}
				else if (Addr > 16) {
					s.wpiszString(wpisz, Addr);
				}
				else {
					break;
				}
			}
			if (h == "G") {
				for (int a = 0; a < PIDList.size(); a++) {
					int PID = PIDList[a];
					s.zwolnieniePamieci(PID, proc[PID]);
					PIDList.erase(find(PIDList.begin(), PIDList.end(), PID));
				}
				s.clear();
			}
			h = " ";
		}
	}
}

