// [interface] Текст, кнопки, HUD и все экраны (меню, настройки, пауза, конец уровня, game over).
#pragma once
#include "../gameplay/Game.hpp"
#include <SFML/Graphics.hpp>
#include <string>

class Ui {
public:
    bool loadFont();   // false, если шрифт не найден

    // вызывать раз в кадр до отрисовки интерфейса: запоминает мышь и клик
    void begin(sf::RenderWindow& window, bool clicked);

    void text(const std::wstring& s, float x, float y, unsigned size,
              sf::Color c = sf::Color::White, bool center = true);
    bool button(const std::wstring& s, float cx, float cy, float w = 340.f);   // true при клике

    void drawHud(const Game& g);                // жизни, монеты, волна, полоска босса
    void drawScreens(Game& g, bool esc);        // меню/настройки/пауза/итоги

    bool full = false;                 // полноэкранный режим включён
    bool fullscreenToggled = false;    // кнопку нажали — main пересоздаёт окно

private:
    sf::Font font;
    sf::RenderWindow* win = nullptr;
    V mouse;
    bool clicked = false;
};
