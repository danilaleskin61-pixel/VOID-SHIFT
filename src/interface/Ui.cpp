// [interface]
#include "Ui.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

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
    const auto& barrier = g.getBarrier();
    text(L"Энергия: " + std::to_wstring(static_cast<int>(std::ceil(barrier.energy()))) + L"/100",
         20, H - 80, 20, sf::Color(120, 215, 255), false);
    sf::RectangleShape energyBack({230.f, 14.f});
    energyBack.setPosition({20.f, H - 60.f});
    energyBack.setFillColor(sf::Color(15, 40, 75));
    win->draw(energyBack);
    sf::RectangleShape energyFill({230.f * barrier.energy() / Barrier::kMaxEnergy, 14.f});
    energyFill.setPosition(energyBack.getPosition());
    energyFill.setFillColor(sf::Color(40, 165, 255));
    win->draw(energyFill);
    std::wstring status;
    if (!barrier.owned()) status = L"Барьер: купи в магазине (Esc)";
    else if (barrier.active()) status = L"Барьер ВКЛ | E — выключить";
    else if (barrier.cooldown() > 0.f)
        status = L"Барьер: КД " + std::to_wstring(static_cast<int>(std::ceil(barrier.cooldown()))) + L" сек.";
    else if (barrier.energy() <= 0.f) status = L"Нет энергии: подбирай синие сферы";
    else status = L"Барьер готов | E — включить";
    text(status, 20, H - 25, 18, sf::Color(150, 220, 255), false);
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
        if (button(L"Магазин", W / 2, 370)) g.openShop();
        if (button(L"Настройки", W / 2, 450)) g.st = State::Settings;
        if (button(L"Выйти", W / 2, 530)) win->close();
    } else if (g.st == State::Settings) {
        text(L"Настройки", W / 2, 130, 64);
        if (button(full ? L"Полный экран: ВКЛ" : L"Полный экран: ВЫКЛ", W / 2, 290, 420)) {
            full = !full;
            fullscreenToggled = true;   // окно пересоздаст main
        }
        text(L"WASD / стрелки — движение. Стрельба автоматическая.", W / 2, 365, 22, sf::Color(170, 200, 220));
        text(L"E — барьер, Esc — пауза и магазин.", W / 2, 395, 22, sf::Color(170, 200, 220));
        text(L"Создатели: Алёшкин Данил, Алёшкин Илдар, Мельников Александр", W / 2, 420, 24);
        if (button(L"Назад", W / 2, 520) || esc) g.st = State::Menu;
    } else if (g.st == State::Pause) {
        win->draw(dim);
        text(L"ПАУЗА", W / 2, 200, 72);
        if (button(L"Продолжить", W / 2, 320) || esc) g.st = State::Play;
        if (button(L"Магазин", W / 2, 400)) g.openShop();
        if (button(L"В меню", W / 2, 480)) g.st = State::Menu;
    } else if (g.st == State::LevelDone) {
        win->draw(dim);
        text(L"Уровень " + std::to_wstring(g.level) + L" пройден!", W / 2, 230, 64, sf::Color(255, 220, 90));
        if (button(L"Дальше", W / 2, 360)) g.nextLevel();
        if (button(L"Магазин", W / 2, 440)) g.openShop();
        if (button(L"В меню", W / 2, 520)) g.st = State::Menu;
    } else if (g.st == State::GameOver) {
        win->draw(dim);
        text(L"КОРАБЛЬ УНИЧТОЖЕН", W / 2, 230, 64, sf::Color(255, 90, 90));
        text(L"Счёт: " + std::to_wstring(g.score), W / 2, 300, 30);
        if (button(L"Заново", W / 2, 380)) { g.resetRun(); g.st = State::Play; }
        if (button(L"Магазин", W / 2, 460)) g.openShop();
        if (button(L"В меню", W / 2, 540)) g.st = State::Menu;
    } else if (g.st == State::Shop) {
        win->draw(dim);
        text(L"МАГАЗИН", W / 2, 110, 64, sf::Color(140, 210, 255));
        text(L"Монеты: " + std::to_wstring(g.coins), W / 2, 185, 28, sf::Color(255, 210, 40));
        text(L"Защитный барьер", W / 2, 265, 36, sf::Color(110, 215, 255));
        text(L"Блокирует пули. Столкновение с врагом ломает барьер.", W / 2, 325, 22);
        text(L"E — включить / выключить. КД после отключения: 45 сек.", W / 2, 365, 22);
        text(L"Энергия: 100. Расход: 12,5/сек. Синяя сфера: +25.", W / 2, 405, 22);
        text(L"Покупка и монеты сохраняются между запусками.", W / 2, 450, 20, sf::Color(180, 190, 210));
        if (g.getBarrier().owned()) {
            text(L"БАРЬЕР УЖЕ КУПЛЕН", W / 2, 535, 28, sf::Color(100, 230, 150));
        } else if (g.coins >= Barrier::kPrice) {
            if (button(L"Купить — " + std::to_wstring(Barrier::kPrice) + L" монет", W / 2, 535, 420))
                g.buyBarrier();
        } else {
            text(L"Нужно " + std::to_wstring(Barrier::kPrice) + L" монет. Не хватает: " +
                 std::to_wstring(Barrier::kPrice - g.coins), W / 2, 535, 25, sf::Color(255, 170, 100));
        }
        if (button(L"Назад", W / 2, 625) || esc) g.closeShop();
    }
}
