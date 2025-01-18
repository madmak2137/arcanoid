#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

// Struktura gracza do przechowywania danych
// Przechowuje nazwê gracza i jego punkty
struct Gracz {
    std::string nazwa; // Nazwa gracza
    int punkty;        // Liczba punktów zdobytych przez gracza
};

// Klasa gry
// Reprezentuje ca³¹ logikê gry oraz elementy graficzne
class Gra {
private:
    sf::RenderWindow okno;  // G³ówne okno gry
    sf::Font czcionka;      // Czcionka u¿ywana w grze
    sf::Text tekstStatus;   // Tekst do wyœwietlania statusu gry
    sf::Text tekstPomoc;    // Tekst wyœwietlany na ekranie pomocy
    sf::Text animowanyTekst; // Animowany tekst punktacji i poziomu
    sf::RectangleShape paletka; // Prostok¹t reprezentuj¹cy paletkê
    sf::CircleShape pilka;      // Ko³o reprezentuj¹ce pi³kê
    sf::ConvexShape przeszkoda; // Wielok¹t jako przeszkoda
    sf::RectangleShape tlo;     // Prostok¹t jako t³o gry
    std::vector<sf::RectangleShape> cegielki; // Wektor przechowuj¹cy cegie³ki
    std::vector<Gracz> gracze;   // Wektor graczy (do przechowywania wyników)
    sf::Vector2f predkoscPilki;  // Wektor prêdkoœci pi³ki
    bool wstrzymana = false;     // Czy gra jest wstrzymana
    bool wyswietlPomoc = false;  // Czy wyœwietlany jest ekran pomocy
    int punkty = 0;              // Aktualna liczba punktów gracza
    int poziom = 1;              // Aktualny poziom gry
    bool zapiszStan = false;     // Flaga wskazuj¹ca, czy zapisaæ stan gry

public:
    // Konstruktor klasy Gra
    // Inicjalizuje okno gry, czcionkê i przygotowuje elementy gry
    Gra() : okno(sf::VideoMode(800, 600), "Arkanoid"), predkoscPilki(0.05f, -0.05f) {
        if (!czcionka.loadFromFile("arial.ttf")) { // £adowanie czcionki
            throw std::runtime_error("Nie uda³o siê za³adowaæ czcionki!");
        }

        przygotujGre(); // Przygotowanie elementów gry
    }

    // Funkcja przygotowuj¹ca grê
    void przygotujGre() {
        // Ustawienia paletki (rozmiar, kolor, pozycja)
        paletka.setSize({ 100, 20 });
        paletka.setFillColor(sf::Color::Blue);
        paletka.setPosition(350, 550);

        // Ustawienia pi³ki (rozmiar, kolor, pozycja)
        pilka.setRadius(10.f);
        pilka.setFillColor(sf::Color::Red);
        pilka.setPosition(400, 300);

        // Ustawienia t³a gry (rozmiar, kolor)
        tlo.setSize({ 800, 600 });
        tlo.setFillColor(sf::Color(50, 50, 50));

        // Ustawienia tekstu statusu
        tekstStatus.setFont(czcionka);
        tekstStatus.setCharacterSize(20);
        tekstStatus.setPosition(10, 10);
        tekstStatus.setFillColor(sf::Color::White);

        // Ustawienia animowanego tekstu punktacji
        animowanyTekst.setFont(czcionka);
        animowanyTekst.setCharacterSize(20);
        animowanyTekst.setFillColor(sf::Color::Green);
        animowanyTekst.setPosition(400, 10);

        // Ustawienia tekstu pomocy
        tekstPomoc.setFont(czcionka);
        tekstPomoc.setCharacterSize(20);
        tekstPomoc.setFillColor(sf::Color::Yellow);
        tekstPomoc.setString("Ekran pomocy\nNacisnij F1 aby wrocic\nNacisnij ESC aby wyjsc");
        tekstPomoc.setPosition(200, 250);

        // Tworzenie przeszkody (wielok¹t nieregularny)
        przeszkoda.setPointCount(5);
        przeszkoda.setPoint(0, { 0.f, 0.f });
        przeszkoda.setPoint(1, { 50.f, 10.f });
        przeszkoda.setPoint(2, { 30.f, 50.f });
        przeszkoda.setPoint(3, { -30.f, 50.f });
        przeszkoda.setPoint(4, { -50.f, 10.f });
        przeszkoda.setFillColor(sf::Color::Magenta);
        przeszkoda.setPosition(400, 150);

        // Tworzenie cegie³ek w grze
        stworzCegielki();
    }

    // Funkcja tworz¹ca cegie³ki
    void stworzCegielki() {
        cegielki.clear(); // Czyœci listê cegie³ek przed ich ponownym utworzeniem
        for (int i = 0; i < 5; ++i) { // 5 rzêdów cegie³ek
            for (int j = 0; j < 10; ++j) { // 10 cegie³ek w ka¿dym rzêdzie
                sf::RectangleShape cegielka({ 60, 20 }); // Rozmiar cegie³ki
                cegielka.setFillColor(sf::Color::Green); // Kolor cegie³ki
                cegielka.setPosition(j * 70 + 30, i * 30 + 50); // Pozycja cegie³ki
                cegielki.push_back(cegielka); // Dodanie cegie³ki do wektora
            }
        }
    }

    // Funkcja uruchamiaj¹ca grê
    void uruchom() {
        zapytajOStart(); // Pytanie o wczytanie stanu gry
        while (okno.isOpen()) { // Pêtla gry
            obsluzZdarzenia(); // Obs³uga zdarzeñ u¿ytkownika
            if (!wstrzymana && !wyswietlPomoc) { // Aktualizacja tylko, jeœli gra nie jest wstrzymana
                aktualizuj();
            }
            rysuj(); // Rysowanie elementów gry
        }
    }

    // Funkcja obs³uguj¹ca zdarzenia klawiatury i okna
    void obsluzZdarzenia() {
        sf::Event zdarzenie;
        while (okno.pollEvent(zdarzenie)) { // Sprawdzanie wszystkich zdarzeñ
            if (zdarzenie.type == sf::Event::Closed) { // Zamkniêcie okna
                okno.close();
            }
            else if (zdarzenie.type == sf::Event::KeyPressed) {
                if (zdarzenie.key.code == sf::Keyboard::F1) { // Wyœwietlenie pomocy
                    wyswietlPomoc = !wyswietlPomoc;
                }
                else if (zdarzenie.key.code == sf::Keyboard::Escape) { // Wyjœcie z gry
                    if (zapytajOCzyWyjsc()) {
                        zapiszStanDoPliku(); // Zapis stanu przed wyjœciem
                        okno.close();
                    }
                }
                else if (zdarzenie.key.code == sf::Keyboard::Space) { // Wstrzymanie gry
                    wstrzymana = !wstrzymana;
                }
            }
        }

        // Sterowanie paletk¹ (strza³ki lewo/prawo)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && paletka.getPosition().x > 0) {
            paletka.move(-0.08f, 0.f);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && paletka.getPosition().x < 700) {
            paletka.move(0.08f, 0.f);
        }
    }

    // Funkcja aktualizuj¹ca stan gry
    void aktualizuj() {
        // Aktualizacja animowanego tekstu (punkty i poziom)
        std::ostringstream ss;
        ss << "Punkty: " << punkty << " | Poziom: " << poziom;
        animowanyTekst.setString(ss.str());

        // Ruch pi³ki
        pilka.move(predkoscPilki);

        // Kolizje pi³ki z krawêdziami okna
        if (pilka.getPosition().x < 0 || pilka.getPosition().x + pilka.getRadius() * 2 > 800) {
            predkoscPilki.x = -predkoscPilki.x; // Odbicie od lewej lub prawej krawêdzi
        }
        if (pilka.getPosition().y < 0) {
            predkoscPilki.y = -predkoscPilki.y; // Odbicie od górnej krawêdzi
        }
        else if (pilka.getPosition().y > 600) {
            zresetujPilke(); // Reset pi³ki po opuszczeniu dolnej krawêdzi
        }

        // Kolizja pi³ki z paletk¹
        if (pilka.getGlobalBounds().intersects(paletka.getGlobalBounds())) {
            predkoscPilki.y = -predkoscPilki.y; // Odbicie od paletki
        }

        // Kolizje pi³ki z cegie³kami
        for (auto it = cegielki.begin(); it != cegielki.end(); ) {
            if (pilka.getGlobalBounds().intersects(it->getGlobalBounds())) {
                obsluzKolizjePilkiZCegielka(pilka, *it, predkoscPilki); // Obs³uga kolizji
                it = cegielki.erase(it); // Usuniêcie cegie³ki
                punkty += 10; // Dodanie punktów za zniszczenie cegie³ki
            }
            else {
                ++it;
            }
        }
    }

    // Funkcja rysuj¹ca elementy gry
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

        if (wyswietlPomoc) { // Wyœwietlenie ekranu pomocy, jeœli aktywny
            okno.draw(tekstPomoc);
        }

        okno.display(); // Wyœwietlenie na ekranie
    }

    // Funkcja resetuj¹ca pi³kê po utracie ¿ycia
    void zresetujPilke() {
        pilka.setPosition(400, 300); // Reset pozycji pi³ki
        predkoscPilki = { 0.05f, -0.05f }; // Reset prêdkoœci pi³ki
    }

    // Funkcja pytaj¹ca gracza, czy na pewno chce wyjœæ z gry
    bool zapytajOCzyWyjsc() {
        sf::Text pytanie("Czy na pewno chcesz wyjsc? T/N", czcionka, 20);
        pytanie.setPosition(200, 300);
        pytanie.setFillColor(sf::Color::White);

        okno.draw(pytanie);
        okno.display();

        while (true) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
                return true; // Potwierdzenie wyjœcia
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                return false; // Anulowanie wyjœcia
            }
        }
    }

    // Funkcja zapisuj¹ca stan gry do pliku
    void zapiszStanDoPliku() {
        std::ofstream plik("stan_gry.txt");
        if (!plik) {
            std::cerr << "Nie uda³o siê zapisaæ stanu gry!\n";
            return;
        }

        // Zapisujemy dane o punktach, poziomie, pozycjach cegie³ek i pi³ki
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
            std::cerr << "Nie znaleziono zapisanego stanu gry!\n";
            return false;
        }

        // Wczytujemy dane o punktach, poziomie, pozycjach cegie³ek i pi³ki
        plik >> punkty;
        plik >> poziom;
        float pilkaX, pilkaY, predkoscX, predkoscY;
        plik >> pilkaX >> pilkaY;
        pilka.setPosition(pilkaX, pilkaY);
        plik >> predkoscX >> predkoscY;
        predkoscPilki = { predkoscX, predkoscY };

        cegielki.clear();
        float cegielkaX, cegielkaY;
        while (plik >> cegielkaX >> cegielkaY) {
            sf::RectangleShape cegielka({ 60, 20 });
            cegielka.setFillColor(sf::Color::Green);
            cegielka.setPosition(cegielkaX, cegielkaY);
            cegielki.push_back(cegielka);
        }

        std::cout << "Stan gry wczytany pomyœlnie!\n";
        return true;
    }

    // Funkcja pytaj¹ca o wczytanie stanu gry
    void zapytajOStart() {
        sf::Text pytanie("Czy chcesz wczytac zapisany stan gry? T/N", czcionka, 20);
        pytanie.setPosition(200, 300);
        pytanie.setFillColor(sf::Color::White);

        okno.draw(pytanie);
        okno.display();

        while (true) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
                if (!wczytajStanZPliku()) {
                    std::cerr << "Rozpoczynam nowa gre!\n";
                }
                break;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                break;
            }
        }
    }

    void obsluzKolizjePilkiZCegielka(sf::CircleShape& pilka, sf::RectangleShape& cegielka, sf::Vector2f& predkoscPilki) {
        sf::FloatRect pilkaBounds = pilka.getGlobalBounds();
        sf::FloatRect cegielkaBounds = cegielka.getGlobalBounds();

        float lewyNadmiar = cegielkaBounds.left - (pilkaBounds.left + pilkaBounds.width);
        float prawyNadmiar = (cegielkaBounds.left + cegielkaBounds.width) - pilkaBounds.left;
        float gornyNadmiar = cegielkaBounds.top - (pilkaBounds.top + pilkaBounds.height);
        float dolnyNadmiar = (cegielkaBounds.top + cegielkaBounds.height) - pilkaBounds.top;

        float minimalnyNadmiarX = std::abs(lewyNadmiar) < std::abs(prawyNadmiar) ? lewyNadmiar : prawyNadmiar;
        float minimalnyNadmiarY = std::abs(gornyNadmiar) < std::abs(dolnyNadmiar) ? gornyNadmiar : dolnyNadmiar;

        if (std::abs(minimalnyNadmiarX) < std::abs(minimalnyNadmiarY)) {
            predkoscPilki.x = -predkoscPilki.x; // Odbicie od boku
        }
        else {
            predkoscPilki.y = -predkoscPilki.y; // Odbicie od góry lub do³u
        }
    }
};

int main() {
    Gra gra;
    gra.uruchom();
    return 0;
}
