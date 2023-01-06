#include <Windows.h>
#define DL 25 //ilosc slow w kategorii
#define LP 10 //liczba prob (po ilu blednych literach przegrywasz)

char alfabet[26];
char litera;
string hasla[DL];
string wisielec[50];

bool czy_znak_jest_litera(char);
void dostepne_litery();
void dostepne_litery_aktualizacja();
void wyswietlenie_liter();
char duza_litera(char);
void wczytanie_hasel(string);
void wczytanie_wisielca();

int sprawdzenie(char zgadywana_litera, string poprawne_haslo, string& zgadywane_haslo)
{
    int pasujace = 0;
    int dlugosc = poprawne_haslo.length();
    for (int i = 0; i < dlugosc; i++)
    {

        if (zgadywana_litera == zgadywane_haslo[i])
            return 0;

        if (zgadywana_litera == poprawne_haslo[i])
        {
            zgadywane_haslo[i] = zgadywana_litera;
            pasujace++;
        }
    }
    return pasujace;
}

int gra() {
    system("cls");

    //losowanie kategorii
    srand(time(NULL));
    int plik_losowy = rand() % 9 + 1;
    string kategoria, nazwa_kategorii;
    switch (plik_losowy)
    {
    case 1:
        nazwa_kategorii = "imie.txt";
        kategoria = "IMIE";
        break;
    case 2:
        nazwa_kategorii = "zawod.txt";
        kategoria = "ZAWOD";
        break;
    case 3:
        nazwa_kategorii = "zwierze.txt";
        kategoria = "ZWIERZE";
        break;
    case 4:
        nazwa_kategorii = "panstwo.txt";
        kategoria = "PANSTWO";
        break;
    case 5:
        nazwa_kategorii = "miasto.txt";
        kategoria = "MIASTO";
        break;
    case 6:
        nazwa_kategorii = "jedzenie.txt";
        kategoria = "JEDZENIE";
        break;
    case 7:
        nazwa_kategorii = "roslina.txt";
        kategoria = "ROSLINA";
        break;
    case 8:
        nazwa_kategorii = "sport.txt";
        kategoria = "SPORT";
        break;
    case 9:
        nazwa_kategorii = "rzecz.txt";
        kategoria = "RZECZ";
        break;
    default:
        return 1;
    }
    //wczytywanie pliku, wisielca i losowanie hasla
    wczytanie_hasel(nazwa_kategorii);

    wczytanie_wisielca();

    int wylosowany_numer_hasla = rand() % DL;

    string wylosowane_haslo = hasla[wylosowany_numer_hasla];

    //tworzenie ukrytego hasla i tabeli dostepnych liter

    string ukryte_haslo(wylosowane_haslo.length(), '_');

    dostepne_litery();

    //kolejne proby

    int ile_blednych_prob = 0;

    while (ile_blednych_prob != LP)
    {
        //wyswietlanie kategorii, dlugosci hasla, zgadywanego hasla i dostepnych liter
        cout << "Kategoria: " << kategoria << "\n";

        cout << "Dlugosc hasla: " << wylosowane_haslo.length() << "\n";

        wyswietlenie_liter();

        cout << "\n";

        for (int i; i < ukryte_haslo.length(); i++) {
            cout << ukryte_haslo[i] << ' ';
        }

        //zgadywanie litery i jej sprawdzenie

        cout << "\nWpisz zgadywaną literę: ";
        cin >> znak;
        bool czy_znak = czy_znak_jest_litera(znak);

        while (czy_znak == false) 
        {
            cout << "\nWprowadzony znak nie jest litera, wpisz literę: ";
            cin >> znak;
            czy_znak = czy_znak_jest_litera(znak);
        }

        duza_litera(znak);

        if (sprawdzenie(znak, wylosowane_haslo, ukryte_haslo) == 0)
        {
            cout << "\nWprowadziles bledna litere!";
            ile_blednych_prob++;
        }
        else
        {
            cout <<"\nWprowadziles poprawna litere!" << endl;
        }

        //aktualizacja liter, wisielca i sprawdzenie hasla

        dostepne_litery_aktualizacja();

        if (ile_blednych_prob > 0)
        {
            for (int i = 0; i < 5; i++)
            {
                cout << wisielec[(ile_blednych_prob - 1) * 5 + i] << "\n";
            }
        }

        if (ukryte_haslo == wylosowane_haslo) 
        {
            cout << "Zgadles haslo. Brawo!\n" << "Haslo: " << ukryte_haslo << "\n";
            break;
        }

    }

    if (ile_blednych_prob == LP)
    {
        cout << "Niestety nie odgadles hasla :(\n" << "Haslo: " << wylosowane_haslo << "\n";
    }

    cout << "Czy chcesz zagrac jeszcze raz? [t/n]\n";
    char odpowiedz;
    if (odpowiedz == 't' || odpowiedz == 'T')
    {
        gra();
    }
    cout << "Dziekujemy za gre!";
    return 0;
}

bool czy_znak_jest_litera(char znak)
{
    if ( (znak < 65) || (znak > 91 && znak < 96) || (znak > 122) ) {
        return false;
    }
    else return true;
}

void dostepne_litery() 
{
    for (int i = 0; i < 26; i++) 
    {
        alfabet[i] = i + 65;
    }
}

void dostepne_litery_aktualizacja() 
{
    for (int i = 0; i < 26; i++)
    {
        if (alfabet[i] == znak) 
        {
            alfabet[i] = ' ';
        }
    }
}

void wyswietlenie_liter() 
{
    cout << "\nPozostale litery\n";
    for (int i = 1; i <= 26; i++) 
    {
        if (alfabet[i] != ' ')
        {
            cout << alfabet[i] << ", ";
        }
    }
}

char duza_litera(char znak) 
{
    if (znak >= 96) 
    {
        znak -= 32;
        return znak;
    }
}

void wczytanie_hasel(string nazwa_pliku) 
{
    fstream plik;
    plik.open(nazwa_pliku);
    for (int i = 0; i < DL; i++) 
    {
        plik >> hasla[i];
    }
    plik.close();
}

void wczytanie_wisielca()
{
    fstream dane;
    dane.open("tworzenie_wisielca.txt", ios::in);
    for (int i = 0; i < 50; i++)
    {
        getline(dane, wisielec[i]);
    }
    dane.close();
}
