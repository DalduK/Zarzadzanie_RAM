#include "Strona.h"



void Strona::UstTabStronic(STRON* newpageTable)
{
	this->pageTable = newpageTable;
}

STRON* Strona::PobTabStronic()
{
	return this->pageTable;
}
void Strona::UstWielTabStronic(int num)
{
	this->pageTableSize = num;
}
int Strona::PobWielTabStronic()
{
	return this->pageTableSize;
}

void Strona::UstWielLicznika(int num)
{
	this->licznik = num;
}

int Strona::PobWielLicznika()
{
	return this->licznik;
}