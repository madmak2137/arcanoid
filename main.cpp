#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

// Struktura gracza do przechowywania danych
struct Gracz {
    std::string nazwa;
    int punkty;
};

// Klasa gry
class Gra {
private:
    sf::RenderWindow okno;
    sf::Font czcionka;
    sf::Text tekstStatus;
    sf::Text tekstPomoc;
    sf::Text animowanyTekst;
    sf::Text tekstWyboru;
    sf::RectangleShape paletka;
    sf::CircleShape pilka;
    sf::ConvexShape przeszkoda;
    sf::RectangleShape tlo;
    std::vector<sf::RectangleShape> cegielki;
    std::vector<Gracz> gracze;
    sf::Vector2f predkoscPilki;
    bool wstrzymana = false;
    bool wyswietlPomoc = false;
    int punkty = 0;
    int poziom = 1;
    bool zapiszStan = false;
    float mnoznikTrudnosci = 1.0f; // Mno¿nik prêdkoœci pi³ki

public:
    Gra() : okno(sf::VideoMode(800, 600), "Arkanoid") {
        if (!czcionka.loadFromFile("arial.ttf")) {
            throw std::runtime_error("Nie uda³o siê za³adowaæ czcionki!");
        }
        wybierzTrudnosc();
        przygotujGre();
    }

    void przygotujGre() {
        paletka.setSize({ 100, 20 });
        paletka.setFillColor(sf::Color::Blue);
        paletka.setPosition(350, 550);

        pilka.setRadius(10.f);
        pilka.setFillColor(sf::Color::Red);
        pilka.setPosition(400, 300);

        tlo.setSize({ 800, 600 });
        tlo.setFillColor(sf::Color(50, 50, 50));

        tekstStatus.setFont(czcionka);
        tekstStatus.setCharacterSize(20);
        tekstStatus.setPosition(10, 10);
        tekstStatus.setFillColor(sf::Color::White);

        animowanyTekst.setFont(czcionka);
        animowanyTekst.setCharacterSize(20);
        animowanyTekst.setFillColor(sf::Color::Green);
        animowanyTekst.setPosition(400, 10);

        tekstPomoc.setFont(czcionka);
        tekstPomoc.setCharacterSize(20);
        tekstPomoc.setFillColor(sf::Color::Yellow);
        tekstPomoc.setString("Ekran pomocy\nNacisnij F1 aby wrocic\nNacisnij ESC aby wyjsc");
        tekstPomoc.setPosition(200, 250);

        przeszkoda.setPointCount(5);
        przeszkoda.setPoint(0, { 0.f, 0.f });
        przeszkoda.setPoint(1, { 50.f, 10.f });
        przeszkoda.setPoint(2, { 30.f, 50.f });
        przeszkoda.setPoint(3, { -30.f, 50.f });
        przeszkoda.setPoint(4, { -50.f, 10.f });
        przeszkoda.setFillColor(sf::Color::Red);
        przeszkoda.setPosition(400, 150);

        stworzCegielki();
        predkoscPilki = { 0.05f * mnoznikTrudnosci, -0.05f * mnoznikTrudnosci };
    }

    void wybierzTrudnosc() {
        tekstWyboru.setFont(czcionka);
        tekstWyboru.setCharacterSize(20);
        tekstWyboru.setFillColor(sf::Color::White);
        tekstWyboru.setString("Wybierz poziom trudnosci:\n1. Latwy\n2. Sredni\n3. Ciezki");
        tekstWyboru.setPosition(200, 250);

        okno.clear();
        okno.draw(tekstWyboru);
        okno.display();

        while (true) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                mnoznikTrudnosci = 1.0f;
                break;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                mnoznikTrudnosci = 1.5f;
                break;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                mnoznikTrudnosci = 2.0f;
                break;
            }
        }
    }

    void stworzCegielki() {
        cegielki.clear();
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 10; ++j) {
                sf::RectangleShape cegielka({ 60, 20 });
                cegielka.setFillColor(sf::Color::Green);
                cegielka.setPosition(j * 70 + 30, i * 30 + 50);
                cegielki.push_back(cegielka);
            }
        }
    }

    void uruchom() {
        zapytajOStart();
        while (okno.isOpen()) {
            obsluzZdarzenia();
            if (!wstrzymana && !wyswietlPomoc) {
                aktualizuj();
            }
            rysuj();
        }
    }

    void obsluzZdarzenia() {
        sf::Event zdarzenie;
        while (okno.pollEvent(zdarzenie)) {
            if (zdarzenie.type == sf::Event::Closed) {
                okno.close();
            }
            else if (zdarzenie.type == sf::Event::KeyPressed) {
                if (zdarzenie.key.code == sf::Keyboard::F1) {
                    wyswietlPomoc = !wyswietlPomoc;
                }
                else if (zdarzenie.key.code == sf::Keyboard::Escape) {
                    if (zapytajOCzyWyjsc()) {
                        zapiszStanDoPliku();
                        okno.close();
                    }
                }
                else if (zdarzenie.key.code == sf::Keyboard::Space) {
                    wstrzymana = !wstrzymana;
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && paletka.getPosition().x > 0) {
            paletka.move(-0.08f, 0.f);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && paletka.getPosition().x < 700) {
            paletka.move(0.08f, 0.f);
        }
    }

    void aktualizuj() {
        std::string tekst = "Punkty: " + std::to_string(punkty) + " | Poziom: " + std::to_string(poziom);
        animowanyTekst.setString(tekst);

        pilka.move(predkoscPilki);

        if (pilka.getPosition().x < 0 || pilka.getPosition().x + pilka.getRadius() * 2 > 800) {
            predkoscPilki.x = -predkoscPilki.x;
        }
        if (pilka.getPosition().y < 0) {
            predkoscPilki.y = -predkoscPilki.y;
        }
        else if (pilka.getPosition().y > 600) {
            zresetujPilke();
        }

        if (pilka.getGlobalBounds().intersects(paletka.getGlobalBounds())) {
            predkoscPilki.y = -predkoscPilki.y;
        }
        for (auto it = cegielki.begin(); it != cegielki.end(); ) {
            if (obsluzKolizjePilkiZCegielka(pilka, *it, predkoscPilki)) {
                it = cegielki.erase(it); // Usuñ cegie³kê i przejdŸ do nastêpnego elementu
                punkty += 10;           // Dodaj punkty
            }
            else {
                ++it; // PrzejdŸ do nastêpnego elementu
            }
        }
    }

    void rysuj() {
        okno.clear();
        okno.draw(tlo);
        okno.draw(paletka);
        okno.draw(pilka);
        okno.draw(przeszkoda);
        okno.draw(animowanyTekst);

        for (const auto& cegielka : cegielki) {
            okno.draw(cegielka);
        }

        if (wyswietlPomoc) {
            okno.draw(tekstPomoc);
        }

        okno.display();
    }

    void zresetujPilke() {
        pilka.setPosition(400, 300);
        predkoscPilki = { 0.05f * mnoznikTrudnosci, -0.05f * mnoznikTrudnosci };
    }

    bool zapytajOCzyWyjsc() {
        sf::Text pytanie("Czy na pewno chcesz wyjsc? T/N", czcionka, 20);
        pytanie.setPosition(200, 300);
        pytanie.setFillColor(sf::Color::White);

        okno.draw(pytanie);
        okno.display();

        while (true) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
                return true;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                return false;
            }
        }
    }

    void zapiszStanDoPliku() {
        std::ofstream plik("stan_gry.txt");
        if (!plik) {
            std::cout << "Nie uda³o siê zapisaæ stanu gry!\n";
            return;
        }

        plik << punkty << "\n";
        plik << poziom << "\n";
        plik << pilka.getPosition().x << " " << pilka.getPosition().y << "\n";
        plik << predkoscPilki.x << " " << predkoscPilki.y << "\n";
        for (const auto& cegielka : cegielki) {
            plik << cegielka.getPosition().x << " " << cegielka.getPosition().y << "\n";
        }

        std::cout << "Stan gry zapisany pomyœlnie!\n";
    }

    bool wczytajStanZPliku() {
        std::ifstream plik("stan_gry.txt");
        if (!plik) {
            std::cout << "Nie znaleziono zapisanego stanu gry!\n";
            return false;
        }

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

    void zapytajOStart() {
        sf::Text pytanie("Czy chcesz wczytac zapisany stan gry? T/N", czcionka, 20);
        pytanie.setPosition(200, 300);
        pytanie.setFillColor(sf::Color::White);

        okno.draw(pytanie);
        okno.display();

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

            // Wyznacz minimalny przesuniêcie (nadmiar) na ka¿dej osi
            float nadmiarLewo = pilkaPrawo - cegielkaLewo;
            float nadmiarPrawo = cegielkaPrawo - pilkaLewo;
            float nadmiarGora = pilkaDol - cegielkaGora;
            float nadmiarDol = cegielkaDol - pilkaGora;

            // ZnajdŸ najmniejsze przesuniêcie, aby ustaliæ stronê kolizji
            float minimalnyNadmiarX = (std::abs(nadmiarLewo) < std::abs(nadmiarPrawo)) ? -nadmiarLewo : nadmiarPrawo;
            float minimalnyNadmiarY = (std::abs(nadmiarGora) < std::abs(nadmiarDol)) ? -nadmiarGora : nadmiarDol;

            // Kolizja w osi X
            if (std::abs(minimalnyNadmiarX) < std::abs(minimalnyNadmiarY)) {
                predkoscPilki.x = -predkoscPilki.x;
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
                predkoscPilki.y = -predkoscPilki.y;
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
    Gra gra;
    gra.uruchom();
    return 0;
}
