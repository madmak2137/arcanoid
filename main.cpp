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
    sf::RenderWindow okno;              // G³ówne okno gry
    sf::Font czcionka;                  // Czcionka u¿ywana w grze
    sf::Text tekstStatus;               // Tekst informuj¹cy o stanie gry
    sf::Text tekstPomoc;                // Tekst ekranu pomocy
    sf::Text animowanyTekst;            // Animowany tekst z punktami i poziomem
    sf::Text tekstWyboru;               // Tekst wyboru poziomu trudnoœci
    sf::RectangleShape paletka;         // Paletka kontrolowana przez gracza
    sf::CircleShape pilka;              // Pi³ka
    sf::ConvexShape przeszkoda;         // Przyk³adowa przeszkoda (wielok¹t)
    sf::RectangleShape tlo;             // T³o gry
    std::vector<sf::RectangleShape> cegielki; // Lista cegie³ek do zniszczenia
    std::vector<Gracz> gracze;          // Lista graczy (na razie nie u¿ywana)
    sf::Vector2f predkoscPilki;         // Prêdkoœæ pi³ki
    bool wstrzymana = false;            // Flaga pauzy
    bool wyswietlPomoc = false;         // Flaga wyœwietlania ekranu pomocy
    int punkty = 0;                     // Aktualna liczba punktów
    int poziom = 1;                     // Aktualny poziom gry
    bool zapiszStan = false;            // Flaga zapisu stanu gry
    float mnoznikTrudnosci = 1.0f;      // Mno¿nik trudnoœci (prêdkoœci pi³ki)

public:
    Gra() : okno(sf::VideoMode(800, 600), "Arkanoid") { // Konstruktor
        if (!czcionka.loadFromFile("arial.ttf")) { // £adowanie czcionki
            throw std::runtime_error("Nie uda³o siê za³adowaæ czcionki!");
        }
        wybierzTrudnosc(); // Wywo³anie funkcji wyboru poziomu trudnoœci
        przygotujGre();    // Przygotowanie elementów gry
    }

    // Funkcja przygotowuj¹ca grê
    void przygotujGre() {
        // Ustawienia paletki
        paletka.setSize({ 100, 20 });
        paletka.setFillColor(sf::Color::Blue);
        paletka.setPosition(350, 550);

        // Ustawienia pi³ki
        pilka.setRadius(10.f);
        pilka.setFillColor(sf::Color::Red);
        pilka.setPosition(400, 300);

        // Ustawienia t³a
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

        // Ustawienia przeszkody (wielok¹ta)
        przeszkoda.setPointCount(5);
        przeszkoda.setPoint(0, { 0.f, 0.f });
        przeszkoda.setPoint(1, { 50.f, 10.f });
        przeszkoda.setPoint(2, { 30.f, 50.f });
        przeszkoda.setPoint(3, { -30.f, 50.f });
        przeszkoda.setPoint(4, { -50.f, 10.f });
        przeszkoda.setFillColor(sf::Color::Red);
        przeszkoda.setPosition(400, 150);

        // Tworzenie cegie³ek
        stworzCegielki();

        // Inicjalizacja prêdkoœci pi³ki
        predkoscPilki = { 0.05f * mnoznikTrudnosci, -0.05f * mnoznikTrudnosci };
    }

    // Funkcja wyboru trudnoœci gry
    void wybierzTrudnosc() {
        tekstWyboru.setFont(czcionka);
        tekstWyboru.setCharacterSize(20);
        tekstWyboru.setFillColor(sf::Color::White);
        tekstWyboru.setString("Wybierz poziom trudnosci:\n1. Latwy\n2. Sredni\n3. Ciezki");
        tekstWyboru.setPosition(200, 250);

        // Wyœwietlenie ekranu wyboru trudnoœci
        okno.clear();
        okno.draw(tekstWyboru);
        okno.display();

        // Oczekiwanie na wybór u¿ytkownika
        while (true) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                mnoznikTrudnosci = 1.0f; // £atwy poziom
                break;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                mnoznikTrudnosci = 1.5f; // Œredni poziom
                break;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                mnoznikTrudnosci = 2.0f; // Trudny poziom
                break;
            }
        }
    }

    // Funkcja tworz¹ca cegie³ki
    void stworzCegielki() {
        cegielki.clear(); // Usuniêcie starych cegie³ek
        for (int i = 0; i < 5; ++i) { // Piêæ rzêdów cegie³ek
            for (int j = 0; j < 10; ++j) { // Dziesiêæ kolumn cegie³ek
                sf::RectangleShape cegielka({ 60, 20 });
                cegielka.setFillColor(sf::Color::Green);
                cegielka.setPosition(j * 70 + 30, i * 30 + 50); // Ustawienie pozycji
                cegielki.push_back(cegielka);
            }
        }
    }

    // G³ówna pêtla gry
    void uruchom() {
        zapytajOStart(); // Sprawdzenie, czy u¿ytkownik chce wczytaæ zapis gry
        while (okno.isOpen()) {
            obsluzZdarzenia(); // Obs³uga zdarzeñ (klawiatura, mysz itp.)
            if (!wstrzymana && !wyswietlPomoc) {
                aktualizuj(); // Aktualizacja stanu gry
            }
            rysuj(); // Rysowanie elementów na ekranie
        }
    }

    // Obs³uga zdarzeñ
    void obsluzZdarzenia() {
        sf::Event zdarzenie;
        while (okno.pollEvent(zdarzenie)) {
            if (zdarzenie.type == sf::Event::Closed) {
                okno.close(); // Zamkniêcie okna gry
            }
            else if (zdarzenie.type == sf::Event::KeyPressed) {
                if (zdarzenie.key.code == sf::Keyboard::F1) {
                    wyswietlPomoc = !wyswietlPomoc; // W³¹czenie/wy³¹czenie pomocy
                }
                else if (zdarzenie.key.code == sf::Keyboard::Escape) {
                    if (zapytajOCzyWyjsc()) { // Sprawdzenie, czy wyjœæ
                        zapiszStanDoPliku(); // Zapis stanu gry
                        okno.close();
                    }
                }
                else if (zdarzenie.key.code == sf::Keyboard::Space) {
                    wstrzymana = !wstrzymana; // Pauza gry
                }
            }
        }

        // Obs³uga ruchu paletki
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && paletka.getPosition().x > 0) {
            paletka.move(-0.08f, 0.f); // Ruch w lewo
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && paletka.getPosition().x < 700) {
            paletka.move(0.08f, 0.f); // Ruch w prawo
        }
    }

    // Funkcja aktualizuj¹ca stan gry
    void aktualizuj() {
        // Aktualizacja tekstu stanu gry
        std::string tekst = "Punkty: " + std::to_string(punkty) + " | Poziom: " + std::to_string(poziom);
        animowanyTekst.setString(tekst);

        // Ruch pi³ki
        pilka.move(predkoscPilki);

        // Sprawdzenie kolizji pi³ki z krawêdziami ekranu
        if (pilka.getPosition().x < 0 || pilka.getPosition().x + pilka.getRadius() * 2 > 800) {
            predkoscPilki.x = -predkoscPilki.x; // Odbicie w poziomie
        }
        if (pilka.getPosition().y < 0) {
            predkoscPilki.y = -predkoscPilki.y; // Odbicie w pionie
        }
        else if (pilka.getPosition().y > 600) {
            zresetujPilke(); // Reset pi³ki po spadniêciu poni¿ej planszy
        }

        // Sprawdzenie kolizji pi³ki z paletk¹
        if (pilka.getGlobalBounds().intersects(paletka.getGlobalBounds())) {
            predkoscPilki.y = -predkoscPilki.y; // Odbicie pi³ki od paletki
        }

        // Sprawdzenie kolizji pi³ki z cegie³kami
        for (auto it = cegielki.begin(); it != cegielki.end(); ) {
            if (obsluzKolizjePilkiZCegielka(pilka, *it, predkoscPilki)) {
                it = cegielki.erase(it); // Usuniêcie cegie³ki
                punkty += 10;           // Dodanie punktów
            }
            else {
                ++it; // Przejœcie do kolejnej cegie³ki
            }
        }
    }

    // Funkcja rysuj¹ca elementy na ekranie
    void rysuj() {
        okno.clear(); // Czyszczenie ekranu
        okno.draw(tlo); // Rysowanie t³a
        okno.draw(paletka); // Rysowanie paletki
        okno.draw(pilka); // Rysowanie pi³ki
        okno.draw(przeszkoda); // Rysowanie przeszkody
        okno.draw(animowanyTekst); // Rysowanie animowanego tekstu

        // Rysowanie cegie³ek
        for (const auto& cegielka : cegielki) {
            okno.draw(cegielka);
        }

        // Rysowanie ekranu pomocy (jeœli aktywny)
        if (wyswietlPomoc) {
            okno.draw(tekstPomoc);
        }

        okno.display(); // Wyœwietlanie zawartoœci okna
    }

    // Resetowanie pozycji pi³ki po spadniêciu
    void zresetujPilke() {
        pilka.setPosition(400, 300); // Powrót pi³ki do pozycji startowej
        predkoscPilki = { 0.05f * mnoznikTrudnosci, -0.05f * mnoznikTrudnosci }; // Przywrócenie prêdkoœci
    }

    // Funkcja obs³uguj¹ca pytanie o wyjœcie z gry
    bool zapytajOCzyWyjsc() {
        sf::Text pytanie("Czy na pewno chcesz wyjsc? T/N", czcionka, 20);
        pytanie.setPosition(200, 300);
        pytanie.setFillColor(sf::Color::White);

        okno.draw(pytanie); // Wyœwietlenie pytania
        okno.display();

        // Oczekiwanie na odpowiedŸ u¿ytkownika
        while (true) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
                return true; // Tak
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                return false; // Nie
            }
        }
    }

    // Funkcja zapisuj¹ca stan gry do pliku
    void zapiszStanDoPliku() {
        std::ofstream plik("stan_gry.txt");
        if (!plik) {
            std::cout << "Nie uda³o siê zapisaæ stanu gry!\n";
            return;
        }

        // Zapis punktów, poziomu, pozycji pi³ki, prêdkoœci i cegie³ek
        plik << punkty << "\n";
        plik << poziom << "\n";
        plik << pilka.getPosition().x << " " << pilka.getPosition().y << "\n";
        plik << predkoscPilki.x << " " << predkoscPilki.y << "\n";
        for (const auto& cegielka : cegielki) {
            plik << cegielka.getPosition().x << " " << cegielka.getPosition().y << "\n";
        }

        std::cout << "Stan gry zapisany pomyœlnie!\n";
    }

    // Funkcja wczytuj¹ca stan gry z pliku
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

        cegielki.clear(); // Usuniêcie obecnych cegie³ek
        float cegielkaX, cegielkaY;
        while (plik >> cegielkaX >> cegielkaY) {
            sf::RectangleShape cegielka({ 60, 20 });
            cegielka.setFillColor(sf::Color::Green);
            cegielka.setPosition(cegielkaX, cegielkaY);
            cegielki.push_back(cegielka); // Dodanie wczytanej cegie³ki
        }

        std::cout << "Stan gry wczytany pomyœlnie!\n";
        return true;
    }

    // Funkcja pytaj¹ca u¿ytkownika o start gry
    void zapytajOStart() {
        sf::Text pytanie("Czy chcesz wczytac zapisany stan gry? T/N", czcionka, 20);
        pytanie.setPosition(200, 300);
        pytanie.setFillColor(sf::Color::White);

        okno.draw(pytanie); // Wyœwietlenie pytania
        okno.display();

        // Oczekiwanie na odpowiedŸ u¿ytkownika
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

    // Funkcja obs³uguj¹ca kolizjê pi³ki z cegie³k¹
    bool obsluzKolizjePilkiZCegielka(sf::CircleShape& pilka, sf::RectangleShape& cegielka, sf::Vector2f& predkoscPilki) {
        sf::FloatRect pilkaBounds = pilka.getGlobalBounds();
        sf::FloatRect cegielkaBounds = cegielka.getGlobalBounds();

        // SprawdŸ, czy wyst¹pi³a kolizja
        if (pilkaBounds.intersects(cegielkaBounds)) {
            // Wyznacz po³o¿enia boków pi³ki i cegie³ki
            float pilkaLewo = pilkaBounds.left;
            float pilkaPrawo = pilkaBounds.left + pilkaBounds.width;
            float pilkaGora = pilkaBounds.top;
            float pilkaDol = pilkaBounds.top + pilkaBounds.height;

            float cegielkaLewo = cegielkaBounds.left;
            float cegielkaPrawo = cegielkaBounds.left + cegielkaBounds.width;
            float cegielkaGora = cegielkaBounds.top;
            float cegielkaDol = cegielkaBounds.top + cegielkaBounds.height;

            // Wyznacz minimalne przesuniêcie (nadmiar) na ka¿dej osi
            float nadmiarLewo = pilkaPrawo - cegielkaLewo;
            float nadmiarPrawo = cegielkaPrawo - pilkaLewo;
            float nadmiarGora = pilkaDol - cegielkaGora;
            float nadmiarDol = cegielkaDol - pilkaGora;

            // ZnajdŸ najmniejsze przesuniêcie, aby ustaliæ stronê kolizji
            float minimalnyNadmiarX = (std::abs(nadmiarLewo) < std::abs(nadmiarPrawo)) ? -nadmiarLewo : nadmiarPrawo;
            float minimalnyNadmiarY = (std::abs(nadmiarGora) < std::abs(nadmiarDol)) ? -nadmiarGora : nadmiarDol;

            // Kolizja w osi X
            if (std::abs(minimalnyNadmiarX) < std::abs(minimalnyNadmiarY)) {
                predkoscPilki.x = -predkoscPilki.x; // Zmiana kierunku w poziomie
                // Korekta pozycji pi³ki, aby unikn¹æ nak³adania siê
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
                // Korekta pozycji pi³ki, aby unikn¹æ nak³adania siê
                if (minimalnyNadmiarY < 0) {
                    pilka.setPosition(pilka.getPosition().x, cegielkaGora - pilkaBounds.height);
                }
                else {
                    pilka.setPosition(pilka.getPosition().x, cegielkaDol);
                }
            }
            return true; // Kolizja wyst¹pi³a
        }
        return false; // Brak kolizji
    }

};

int main() {
    Gra gra; // Tworzenie obiektu gry
    gra.uruchom(); // Uruchomienie gry
    return 0;
}
