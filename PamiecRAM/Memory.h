#pragma once
#ifndef Memory_H
#define Memory_H
#include "Strona.h"
#include <iostream>
#include <list>
#include <queue>
#include <string>
#include <fstream>
#include <time.h>
#include <random>
#include <vector>
#include <map>
#include <regex>
using namespace std;
enum class CharTable : unsigned char
{
	VL = 179,
};

class Process;



struct FIFO_entry {
	int PID;
	int pageNumber;

	FIFO_entry(int PID, int pageNumber) {
		this->PID = PID;
		this->pageNumber = pageNumber;
	}
};

class Memory
{
private:
	//---RAM---//
	char * RAM;
	const int WIE_PAM = 256;
	const int WIE_RAM = 16;
	const int ILOSC_RAM = WIE_PAM / WIE_RAM;
	//---Obliczenia zwiazane z ramem i plikiem wym---//
	const int ObliczOffset(int addr1);
	const int ObliczNumerStrony(int addr1);
	const int ObliczTabliceStronic(int siBytes);
	//---Zapewnienie wolnej ramy Procesowi---//
	const void ZapewnijWolnaRame();//=Zapewnienie ¿e rama jest wolna
	//---Odczyt Pamieci---//
	const char OdczytajZPamieci(int PID, int addr1, Strona *g);
	//---Zapisanie danych w PAMRam---//
	const void ZapiszWPamieci(int PID, int addr1, char element, Strona *g);
	//---Zapewnienie miejsca dla Strony w pamieci ram---//
	const void ZapewnijStroneWPamieci(int PID, int logicalAddress, Strona *g);
	//---Pobranie Ramy z pamieci dla procesu---//
	const int PobierzWolRamePamieci();
	//---PLIK WYMIANY---//

	char * Buffer;
	const int ILOSC_RAM_P_WYMIANY = 512;
	const int WIE_P_WYMIANY = ILOSC_RAM_P_WYMIANY * WIE_RAM;
	//---Zapis do pliku wymiany bez pam ram---//
	const void ZapiszDoPlikuWym(int nr, string Content);
	//---Pobieranie wolnej ramki z p. Wymiany---//
	const int PobierzWolnaRameBuff();
	//---Wyczyszczenie pustej ramki pliku wymiany---//
	const void WyczyscRamPlikuWym(int frameNumber);
	list<int> WolneRamki;
	list<int> WolneRamkiBuffera;
	list<FIFO_entry> FIFO;
	//---Przenoszenie z pliku wymiany do ramu i w druga strone---//
	const void PrzeStroBuffDoPam(int PID, int pageNr, Strona *g);
	const void PrzeStroPamDoBuff(int PID, int pageNr, Strona *g);
	const void PrzeZawPamDoBuff(int MemoryFrame, int SwapFileFrame);
	const void PrzeZawBuffDoPam(int SwapFileFrame, int MemoryFrame);
	//---Wpis do Fifo---//
	const void DodajDoFifo(FIFO_entry entry);
	const bool BrakMieWPowAdr(int PID, int logicalAddress, Strona *g);

public:
	
	const int ObliczAdresFizyczny(int PID, int addr1, Strona *g);
	void PIDproces(int PID, string plik, Strona *g);
	//-----Pamiec-----//
	const void PrzydzialPamieci(int PID, string proces, int size, Strona *g);
	const void zwolnieniePamieci(int PID, Strona *g);
	const void WypiszZasobPamieci(int nrToPrint = 0);
	const void WpiszZasobPamDoBuff(int nr, string zasobPam);
	//----Sprawdzanie Powierzchni Adresowej----//
	const bool CzyAdrWPowAdresss(int AddrLog, Strona *g);
	const bool CzyZasiegAdrWPowAdres(int AddrLog, int range, Strona *g);
	//----Drukowanie----//
	const void WydrukujProcesy(int PID, bool wRamie, Strona *g);
	const void WydrukujFIFO();
	const void WydrukujTabliceStronic(int PID, Strona *g);
	const void WydrukujRame(int RamaNr, int StronaNr = -1);
	//----Odczytywanie i zapisywanie stringow----//
	//(potrzebne do odczytu procesow//
	const void zapiszString(int PID, int Addr1, string conten, Strona *g);
	const string odczytajString(int PID, int Addr1, Strona *g);
	char wypiszZnak(int PID, int Addr);
	char wypiszZnak(int Addr);
	void wypiszString(int PID,Strona *m);
	void wypiszStringAA(int Addr, int Addr2);
	void wpiszString(string zawartosc, int Addr);
	void clear();
	//konstruktor//
	Memory();
	~Memory();
};
#endif