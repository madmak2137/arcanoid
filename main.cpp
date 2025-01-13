#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <fstream>
#include <vector>
#include <string>

#include <iostream>

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
    sf::RectangleShape paletka;
    sf::CircleShape pilka;
    std::vector<sf::RectangleShape> cegielki;
    std::vector<Gracz> gracze;
    sf::Vector2f predkoscPilki;
    bool wstrzymana = false;
    bool wyswietlPomoc = false;
    int punkty = 0;
    int poziom = 1;

public:
    Gra() : okno(sf::VideoMode(800, 600), "Arkanoid"), predkoscPilki(0.05f, -0.05f) {
        if (!czcionka.loadFromFile("arial.ttf")) {
            throw std::runtime_error("Nie uda³o siê za³adowaæ czcionki!");
        }

        przygotujGre();
    }

    void przygotujGre() {
        // Ustawienia paletki
        paletka.setSize({ 100, 20 });
        paletka.setFillColor(sf::Color::Blue);
        paletka.setPosition(350, 550);

        // Ustawienia pi³ki
        pilka.setRadius(10.f);
        pilka.setFillColor(sf::Color::Red);
        pilka.setPosition(400, 300);

        // Ustawienia tekstu
        tekstStatus.setFont(czcionka);
        tekstStatus.setCharacterSize(20);
        tekstStatus.setPosition(10, 10);
        tekstStatus.setFillColor(sf::Color::White);

        tekstPomoc.setFont(czcionka);
        tekstPomoc.setCharacterSize(20);
        tekstPomoc.setFillColor(sf::Color::Yellow);
        tekstPomoc.setString("Ekran pomocy\nNacisnij F1 aby wrocic\nNacisnij ESC aby wyjsc");
        tekstPomoc.setPosition(200, 250);

        // Tworzenie cegie³ek
        stworzCegielki();
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
                        zapiszStanGry();
                        okno.close();
                    }
                }
            }
        }

        // Sterowanie paletk¹
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && paletka.getPosition().x > 0) {
            paletka.move(-0.08f, 0.f);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && paletka.getPosition().x < 700) {
            paletka.move(0.08f, 0.f);
        }
    }

    void aktualizuj() {
        // Ruch pi³ki
        pilka.move(predkoscPilki);

        // Kolizje pi³ki z krawêdziami okna
        if (pilka.getPosition().x < 0 || pilka.getPosition().x + pilka.getRadius() * 2 > 800) {
            predkoscPilki.x = -predkoscPilki.x;
        }
        if (pilka.getPosition().y < 0) {
            predkoscPilki.y = -predkoscPilki.y;
        }
        else if (pilka.getPosition().y > 600) {
            zresetujPilke();
        }

        // Kolizja z paletk¹
        if (pilka.getGlobalBounds().intersects(paletka.getGlobalBounds())) {
            predkoscPilki.y = -predkoscPilki.y;
        }

        // Kolizje z cegie³kami
        for (auto it = cegielki.begin(); it != cegielki.end(); ) {
            if (pilka.getGlobalBounds().intersects(it->getGlobalBounds())) {
                obsluzKolizjePilkiZCegielka(pilka, *it, predkoscPilki);
                it = cegielki.erase(it);
                punkty += 10;
            }
            else {
                ++it;
            }
        }


    }

    void rysuj() {
        okno.clear();

        if (wyswietlPomoc) {
            okno.draw(tekstPomoc);
        }
        else {
            okno.draw(paletka);
            okno.draw(pilka);

            for (const auto& cegielka : cegielki) {
                okno.draw(cegielka);
            }

            okno.draw(tekstStatus);
        }

        okno.display();
    }

    void zresetujPilke() {
        pilka.setPosition(400, 300);
        predkoscPilki = { 0.05f, -0.05f };
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

    void zapiszStanGry() {
        std::ofstream plik("stan_gry.txt");
        plik << punkty << " " << poziom << "\n";
        plik.close();
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
