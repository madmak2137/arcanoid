#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

// Struktura gracza do przechowywania danych
struct Gracz {
    std::string nazwa; // Nazwa gracza
    int punkty;        // Zdobyte punkty
};

// Klasa gry
class Gra {
private:
    sf::RenderWindow okno;              // G��wne okno gry
    sf::Font czcionka;                  // Czcionka u�ywana w grze
    sf::Text tekstStatus;               // Tekst informuj�cy o stanie gry
    sf::Text tekstPomoc;                // Tekst ekranu pomocy
    sf::Text animowanyTekst;            // Animowany tekst z punktami i poziomem
    sf::Text tekstWyboru;               // Tekst wyboru poziomu trudno�ci
    sf::RectangleShape paletka;         // Paletka kontrolowana przez gracza
    sf::CircleShape pilka;              // Pi�ka
    sf::ConvexShape przeszkoda;         // Przyk�adowa przeszkoda (wielok�t)
    sf::RectangleShape tlo;             // T�o gry
    std::vector<sf::RectangleShape> cegielki; // Lista cegie�ek do zniszczenia
    std::vector<Gracz> gracze;          // Lista graczy (na razie nie u�ywana)
    sf::Vector2f predkoscPilki;         // Pr�dko�� pi�ki
    bool wstrzymana = false;            // Flaga pauzy
    bool wyswietlPomoc = false;         // Flaga wy�wietlania ekranu pomocy
    int punkty = 0;                     // Aktualna liczba punkt�w
    int poziom = 1;                     // Aktualny poziom gry
    bool zapiszStan = false;            // Flaga zapisu stanu gry
    float mnoznikTrudnosci = 1.0f;      // Mno�nik trudno�ci (pr�dko�ci pi�ki)

public:
    Gra() : okno(sf::VideoMode(800, 600), "Arkanoid") { // Konstruktor
        if (!czcionka.loadFromFile("arial.ttf")) { // �adowanie czcionki
            throw std::runtime_error("Nie uda�o si� za�adowa� czcionki!");
        }
        wybierzTrudnosc(); // Wywo�anie funkcji wyboru poziomu trudno�ci
        przygotujGre();    // Przygotowanie element�w gry
    }

    // Funkcja przygotowuj�ca gr�
    void przygotujGre() {
        // Ustawienia paletki
        paletka.setSize({ 100, 20 });
        paletka.setFillColor(sf::Color::Blue);
        paletka.setPosition(350, 550);

        // Ustawienia pi�ki
        pilka.setRadius(10.f);
        pilka.setFillColor(sf::Color::Red);
        pilka.setPosition(400, 300);

        // Ustawienia t�a
        tlo.setSize({ 800, 600 });
        tlo.setFillColor(sf::Color(50, 50, 50));

        // Ustawienia tekstu stanu gry
        tekstStatus.setFont(czcionka);
        tekstStatus.setCharacterSize(20);
        tekstStatus.setPosition(10, 10);
        tekstStatus.setFillColor(sf::Color::White);

        // Ustawienia animowanego tekstu
        animowanyTekst.setFont(czcionka);
        animowanyTekst.setCharacterSize(20);
        animowanyTekst.setFillColor(sf::Color::Green);
        animowanyTekst.setPosition(400, 10);

        // Ustawienia ekranu pomocy
        tekstPomoc.setFont(czcionka);
        tekstPomoc.setCharacterSize(20);
        tekstPomoc.setFillColor(sf::Color::Yellow);
        tekstPomoc.setString("Ekran pomocy\nNacisnij F1 aby wrocic\nNacisnij ESC aby wyjsc");
        tekstPomoc.setPosition(200, 250);

        // Ustawienia przeszkody (wielok�ta)
        przeszkoda.setPointCount(5);
        przeszkoda.setPoint(0, { 0.f, 0.f });
        przeszkoda.setPoint(1, { 50.f, 10.f });
        przeszkoda.setPoint(2, { 30.f, 50.f });
        przeszkoda.setPoint(3, { -30.f, 50.f });
        przeszkoda.setPoint(4, { -50.f, 10.f });
        przeszkoda.setFillColor(sf::Color::Red);
        przeszkoda.setPosition(400, 150);

        // Tworzenie cegie�ek
        stworzCegielki();

        // Inicjalizacja pr�dko�ci pi�ki
        predkoscPilki = { 0.05f * mnoznikTrudnosci, -0.05f * mnoznikTrudnosci };
    }

    // Funkcja wyboru trudno�ci gry
    void wybierzTrudnosc() {
        tekstWyboru.setFont(czcionka);
        tekstWyboru.setCharacterSize(20);
        tekstWyboru.setFillColor(sf::Color::White);
        tekstWyboru.setString("Wybierz poziom trudnosci:\n1. Latwy\n2. Sredni\n3. Ciezki");
        tekstWyboru.setPosition(200, 250);

        // Wy�wietlenie ekranu wyboru trudno�ci
        okno.clear();
        okno.draw(tekstWyboru);
        okno.display();

        // Oczekiwanie na wyb�r u�ytkownika
        while (true) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                mnoznikTrudnosci = 1.0f; // �atwy poziom
                break;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                mnoznikTrudnosci = 1.5f; // �redni poziom
                break;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                mnoznikTrudnosci = 2.0f; // Trudny poziom
                break;
            }
        }
    }

    // Funkcja tworz�ca cegie�ki
    void stworzCegielki() {
        cegielki.clear(); // Usuni�cie starych cegie�ek
        for (int i = 0; i < 5; ++i) { // Pi�� rz�d�w cegie�ek
            for (int j = 0; j < 10; ++j) { // Dziesi�� kolumn cegie�ek
                sf::RectangleShape cegielka({ 60, 20 });
                cegielka.setFillColor(sf::Color::Green);
                cegielka.setPosition(j * 70 + 30, i * 30 + 50); // Ustawienie pozycji
                cegielki.push_back(cegielka);
            }
        }
    }

    // G��wna p�tla gry
    void uruchom() {
        zapytajOStart(); // Sprawdzenie, czy u�ytkownik chce wczyta� zapis gry
        while (okno.isOpen()) {
            obsluzZdarzenia(); // Obs�uga zdarze� (klawiatura, mysz itp.)
            if (!wstrzymana && !wyswietlPomoc) {
                aktualizuj(); // Aktualizacja stanu gry
            }
            rysuj(); // Rysowanie element�w na ekranie
        }
    }

    // Obs�uga zdarze�
    void obsluzZdarzenia() {
        sf::Event zdarzenie;
        while (okno.pollEvent(zdarzenie)) {
            if (zdarzenie.type == sf::Event::Closed) {
                okno.close(); // Zamkni�cie okna gry
            }
            else if (zdarzenie.type == sf::Event::KeyPressed) {
                if (zdarzenie.key.code == sf::Keyboard::F1) {
                    wyswietlPomoc = !wyswietlPomoc; // W��czenie/wy��czenie pomocy
                }
                else if (zdarzenie.key.code == sf::Keyboard::Escape) {
                    if (zapytajOCzyWyjsc()) { // Sprawdzenie, czy wyj��
                        zapiszStanDoPliku(); // Zapis stanu gry
                        okno.close();
                    }
                }
                else if (zdarzenie.key.code == sf::Keyboard::Space) {
                    wstrzymana = !wstrzymana; // Pauza gry
                }
            }
        }

        // Obs�uga ruchu paletki
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && paletka.getPosition().x > 0) {
            paletka.move(-0.08f, 0.f); // Ruch w lewo
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && paletka.getPosition().x < 700) {
            paletka.move(0.08f, 0.f); // Ruch w prawo
        }
    }

    // Funkcja aktualizuj�ca stan gry
    void aktualizuj() {
        // Aktualizacja tekstu stanu gry
        std::string tekst = "Punkty: " + std::to_string(punkty) + " | Poziom: " + std::to_string(poziom);
        animowanyTekst.setString(tekst);

        // Ruch pi�ki
        pilka.move(predkoscPilki);

        // Sprawdzenie kolizji pi�ki z kraw�dziami ekranu
        if (pilka.getPosition().x < 0 || pilka.getPosition().x + pilka.getRadius() * 2 > 800) {
            predkoscPilki.x = -predkoscPilki.x; // Odbicie w poziomie
        }
        if (pilka.getPosition().y < 0) {
            predkoscPilki.y = -predkoscPilki.y; // Odbicie w pionie
        }
        else if (pilka.getPosition().y > 600) {
            zresetujPilke(); // Reset pi�ki po spadni�ciu poni�ej planszy
        }

        // Sprawdzenie kolizji pi�ki z paletk�
        if (pilka.getGlobalBounds().intersects(paletka.getGlobalBounds())) {
            predkoscPilki.y = -predkoscPilki.y; // Odbicie pi�ki od paletki
        }

        // Sprawdzenie kolizji pi�ki z cegie�kami
        for (auto it = cegielki.begin(); it != cegielki.end(); ) {
            if (obsluzKolizjePilkiZCegielka(pilka, *it, predkoscPilki)) {
                it = cegielki.erase(it); // Usuni�cie cegie�ki
                punkty += 10;           // Dodanie punkt�w
            }
            else {
                ++it; // Przej�cie do kolejnej cegie�ki
            }
        }
    }

    // Funkcja rysuj�ca elementy na ekranie
    void rysuj() {
        okno.clear(); // Czyszczenie ekranu
        okno.draw(tlo); // Rysowanie t�a
        okno.draw(paletka); // Rysowanie paletki
        okno.draw(pilka); // Rysowanie pi�ki
        okno.draw(przeszkoda); // Rysowanie przeszkody
        okno.draw(animowanyTekst); // Rysowanie animowanego tekstu

        // Rysowanie cegie�ek
        for (const auto& cegielka : cegielki) {
            okno.draw(cegielka);
        }

        // Rysowanie ekranu pomocy (je�li aktywny)
        if (wyswietlPomoc) {
            okno.draw(tekstPomoc);
        }

        okno.display(); // Wy�wietlanie zawarto�ci okna
    }

    // Resetowanie pozycji pi�ki po spadni�ciu
    void zresetujPilke() {
        pilka.setPosition(400, 300); // Powr�t pi�ki do pozycji startowej
        predkoscPilki = { 0.05f * mnoznikTrudnosci, -0.05f * mnoznikTrudnosci }; // Przywr�cenie pr�dko�ci
    }

    // Funkcja obs�uguj�ca pytanie o wyj�cie z gry
    bool zapytajOCzyWyjsc() {
        sf::Text pytanie("Czy na pewno chcesz wyjsc? T/N", czcionka, 20);
        pytanie.setPosition(200, 300);
        pytanie.setFillColor(sf::Color::White);

        okno.draw(pytanie); // Wy�wietlenie pytania
        okno.display();

        // Oczekiwanie na odpowied� u�ytkownika
        while (true) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
                return true; // Tak
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                return false; // Nie
            }
        }
    }

    // Funkcja zapisuj�ca stan gry do pliku
    void zapiszStanDoPliku() {
        std::ofstream plik("stan_gry.txt");
        if (!plik) {
            std::cout << "Nie uda�o si� zapisa� stanu gry!\n";
            return;
        }

        // Zapis punkt�w, poziomu, pozycji pi�ki, pr�dko�ci i cegie�ek
        plik << punkty << "\n";
        plik << poziom << "\n";
        plik << pilka.getPosition().x << " " << pilka.getPosition().y << "\n";
        plik << predkoscPilki.x << " " << predkoscPilki.y << "\n";
        for (const auto& cegielka : cegielki) {
            plik << cegielka.getPosition().x << " " << cegielka.getPosition().y << "\n";
        }

        std::cout << "Stan gry zapisany pomy�lnie!\n";
    }

    // Funkcja wczytuj�ca stan gry z pliku
    bool wczytajStanZPliku() {
        std::ifstream plik("stan_gry.txt");
        if (!plik) {
            std::cout << "Nie znaleziono zapisanego stanu gry!\n";
            return false;
        }

        // Wczytanie danych z pliku
        plik >> punkty;
        plik >> poziom;
        float pilkaX, pilkaY, predkoscX, predkoscY;
        plik >> pilkaX >> pilkaY;
        pilka.setPosition(pilkaX, pilkaY);
        plik >> predkoscX >> predkoscY;
        predkoscPilki = { predkoscX, predkoscY };

        cegielki.clear(); // Usuni�cie obecnych cegie�ek
        float cegielkaX, cegielkaY;
        while (plik >> cegielkaX >> cegielkaY) {
            sf::RectangleShape cegielka({ 60, 20 });
            cegielka.setFillColor(sf::Color::Green);
            cegielka.setPosition(cegielkaX, cegielkaY);
            cegielki.push_back(cegielka); // Dodanie wczytanej cegie�ki
        }

        std::cout << "Stan gry wczytany pomy�lnie!\n";
        return true;
    }

    // Funkcja pytaj�ca u�ytkownika o start gry
    void zapytajOStart() {
        sf::Text pytanie("Czy chcesz wczytac zapisany stan gry? T/N", czcionka, 20);
        pytanie.setPosition(200, 300);
        pytanie.setFillColor(sf::Color::White);

        okno.draw(pytanie); // Wy�wietlenie pytania
        okno.display();

        // Oczekiwanie na odpowied� u�ytkownika
        while (true) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
                if (!wczytajStanZPliku()) {
                    std::cout << "Rozpoczynam nowa gre!\n";
                }
                break;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                break;
            }
        }
    }

    // Funkcja obs�uguj�ca kolizj� pi�ki z cegie�k�
    bool obsluzKolizjePilkiZCegielka(sf::CircleShape& pilka, sf::RectangleShape& cegielka, sf::Vector2f& predkoscPilki) {
        sf::FloatRect pilkaBounds = pilka.getGlobalBounds();
        sf::FloatRect cegielkaBounds = cegielka.getGlobalBounds();

        // Sprawd�, czy wyst�pi�a kolizja
        if (pilkaBounds.intersects(cegielkaBounds)) {
            // Wyznacz po�o�enia bok�w pi�ki i cegie�ki
            float pilkaLewo = pilkaBounds.left;
            float pilkaPrawo = pilkaBounds.left + pilkaBounds.width;
            float pilkaGora = pilkaBounds.top;
            float pilkaDol = pilkaBounds.top + pilkaBounds.height;

            float cegielkaLewo = cegielkaBounds.left;
            float cegielkaPrawo = cegielkaBounds.left + cegielkaBounds.width;
            float cegielkaGora = cegielkaBounds.top;
            float cegielkaDol = cegielkaBounds.top + cegielkaBounds.height;

            // Wyznacz minimalne przesuni�cie (nadmiar) na ka�dej osi
            float nadmiarLewo = pilkaPrawo - cegielkaLewo;
            float nadmiarPrawo = cegielkaPrawo - pilkaLewo;
            float nadmiarGora = pilkaDol - cegielkaGora;
            float nadmiarDol = cegielkaDol - pilkaGora;

            // Znajd� najmniejsze przesuni�cie, aby ustali� stron� kolizji
            float minimalnyNadmiarX = (std::abs(nadmiarLewo) < std::abs(nadmiarPrawo)) ? -nadmiarLewo : nadmiarPrawo;
            float minimalnyNadmiarY = (std::abs(nadmiarGora) < std::abs(nadmiarDol)) ? -nadmiarGora : nadmiarDol;

            // Kolizja w osi X
            if (std::abs(minimalnyNadmiarX) < std::abs(minimalnyNadmiarY)) {
                predkoscPilki.x = -predkoscPilki.x; // Zmiana kierunku w poziomie
                // Korekta pozycji pi�ki, aby unikn�� nak�adania si�
                if (minimalnyNadmiarX < 0) {
                    pilka.setPosition(cegielkaLewo - pilkaBounds.width, pilka.getPosition().y);
                }
                else {
                    pilka.setPosition(cegielkaPrawo, pilka.getPosition().y);
                }
            }
            // Kolizja w osi Y
            else {
                predkoscPilki.y = -predkoscPilki.y; // Zmiana kierunku w pionie
                // Korekta pozycji pi�ki, aby unikn�� nak�adania si�
                if (minimalnyNadmiarY < 0) {
                    pilka.setPosition(pilka.getPosition().x, cegielkaGora - pilkaBounds.height);
                }
                else {
                    pilka.setPosition(pilka.getPosition().x, cegielkaDol);
                }
            }
            return true; // Kolizja wyst�pi�a
        }
        return false; // Brak kolizji
    }

};

int main() {
    Gra gra; // Tworzenie obiektu gry
    gra.uruchom(); // Uruchomienie gry
    return 0;
}
