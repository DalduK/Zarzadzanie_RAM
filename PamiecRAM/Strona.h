#pragma once
#include <string>
struct STRON
{
	int RamaZajeta;
	bool wPam;

};

class Strona
{
public:
	std::string str = "";
	int licznik = 0;
	STRON* pageTable;
	int pageTableSize;
	void UstTabStronic(STRON* newpageTable);			//Przemkowe RAM'y
	STRON* PobTabStronic();
	void UstWielTabStronic(int num);
	int PobWielTabStronic();
	void UstWielLicznika(int num);
	int PobWielLicznika();
};