// [interface]
#include "Ui.hpp"
#include <algorithm>
#include <iostream>

bool Ui::loadFont() {
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf") && !font.openFromFile("assets/fonts/font.ttf")) {
        std::cout << "Не найден шрифт (C:/Windows/Fonts/arial.ttf)\n";
        return false;
    }
    return true;
}

void Ui::begin(sf::RenderWindow& window, bool c) {
    win = &window;
    clicked = c;
    mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));   // мышь в логических координатах 1280x720
}

void Ui::text(const std::wstring& s, float x, float y, unsigned size, sf::Color c, bool center) {
    sf::Text t(font, sf::String(s), size);
    t.setFillColor(c);
    auto b = t.getLocalBounds();
    t.setOrigin({center ? b.position.x + b.size.x / 2 : 0.f, b.position.y + b.size.y / 2});
    t.setPosition({x, y});
    win->draw(t);
}

bool Ui::button(const std::wstring& s, float cx, float cy, float w) {
    sf::FloatRect r({cx - w / 2, cy - 30}, {w, 60});
    bool hov = r.contains(mouse);   // навели курсор — кнопка светлеет
    sf::RectangleShape b(r.size);
    b.setPosition(r.position);
    b.setFillColor(hov ? sf::Color(60, 110, 220, 230) : sf::Color(15, 25, 60, 210));
    b.setOutlineThickness(2);
    b.setOutlineColor(sf::Color(120, 180, 255));
    win->draw(b);
    text(s, cx, cy, 28);
    return hov && clicked;
}

void Ui::drawHud(const Game& g) {
    if (!g.inRun()) return;
    text(L"Жизни: " + std::to_wstring(g.lives), 20, 20, 24, sf::Color::White, false);
    text(L"Монеты: " + std::to_wstring(g.coins), 20, 50, 24, sf::Color(255, 210, 40), false);
    text(L"Оружие: " + std::to_wstring(g.wpn) + L"/8", 20, 80, 24, sf::Color(100, 230, 255), false);
    text(L"Уровень " + std::to_wstring(g.level) + L"  Волна " + std::to_wstring(std::min(g.wave, 4)) + L"/4", W / 2, 24, 24);
    text(L"Счёт: " + std::to_wstring(g.score), W - 200, 24, 24);
    if (g.rage > 0) text(L"ЯРОСТЬ!", W / 2, 60, 28, sf::Color(255, 80, 80));

    // полоска здоровья босса (когда он уже выехал на позицию)
    for (auto& e : g.en) if (e.boss && e.p.y >= 130) {
        sf::RectangleShape bk({600, 16}); bk.setPosition({W / 2 - 300, 90}); bk.setFillColor(sf::Color(60, 0, 0)); win->draw(bk);
        sf::RectangleShape fl({600 * std::max(0.f, e.hp) / e.maxhp, 16}); fl.setPosition({W / 2 - 300, 90});
        fl.setFillColor(sf::Color(230, 40, 60)); win->draw(fl);
    }
}

void Ui::drawScreens(Game& g, bool esc) {
    sf::RectangleShape dim({W, H});
    dim.setFillColor(sf::Color(0, 0, 0, 150));   // затемнение под окнами паузы/итогов

    if (g.st == State::Menu) {
        text(L"VOID SHIFT", W / 2, 130, 96, sf::Color(140, 200, 255));
        if (button(L"Играть", W / 2, 290)) { g.resetRun(); g.st = State::Play; }
        if (button(L"Магазин", W / 2, 370)) { /* этап 2: ветка shop */ }
        if (button(L"Настройки", W / 2, 450)) g.st = State::Settings;
        if (button(L"Выйти", W / 2, 530)) win->close();
    } else if (g.st == State::Settings) {
        text(L"Настройки", W / 2, 130, 64);
        if (button(full ? L"Полный экран: ВКЛ" : L"Полный экран: ВЫКЛ", W / 2, 290, 420)) {
            full = !full;
            fullscreenToggled = true;   // окно пересоздаст main
        }
        text(L"Громкость музыки и звуков — в этапе 2", W / 2, 370, 22, sf::Color(170, 170, 170));
        text(L"Создатели: (впиши сюда имена)", W / 2, 420, 24);
        if (button(L"Назад", W / 2, 520) || esc) g.st = State::Menu;
    } else if (g.st == State::Pause) {
        win->draw(dim);
        text(L"ПАУЗА", W / 2, 200, 72);
        if (button(L"Продолжить", W / 2, 320) || esc) g.st = State::Play;
        if (button(L"В меню", W / 2, 400)) g.st = State::Menu;
    } else if (g.st == State::LevelDone) {
        win->draw(dim);
        text(L"Уровень " + std::to_wstring(g.level) + L" пройден!", W / 2, 230, 64, sf::Color(255, 220, 90));
        if (button(L"Дальше", W / 2, 360)) g.nextLevel();
        if (button(L"В меню", W / 2, 440)) g.st = State::Menu;
    } else if (g.st == State::GameOver) {
        win->draw(dim);
        text(L"КОРАБЛЬ УНИЧТОЖЕН", W / 2, 230, 64, sf::Color(255, 90, 90));
        text(L"Счёт: " + std::to_wstring(g.score), W / 2, 300, 30);
        if (button(L"Заново", W / 2, 380)) { g.resetRun(); g.st = State::Play; }
        if (button(L"В меню", W / 2, 460)) g.st = State::Menu;
    }
}
