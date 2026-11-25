// VOID SHIFT — космический шутер. SFML 3, C++17.
// Здесь только окно и главный цикл; вся логика лежит в src/<модуль>/.
#include "background/Background.hpp"
#include "gameplay/Game.hpp"
#include "interface/Ui.hpp"
#include "textures/Textures.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <filesystem>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

// Делаем рабочей папкой папку с exe, чтобы assets/ находились при запуске откуда угодно (ярлык, двойной клик)
static void setWorkingDirToExe() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) std::filesystem::current_path(std::filesystem::path(buf).parent_path());
#endif
}

int main() {
    setWorkingDirToExe();
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "VOID SHIFT");
    // вид 1280x720: игра масштабируется под любое разрешение/полный экран
    auto setup = [&] {
        window.setVerticalSyncEnabled(true);   // vsync вместо ограничения в 60 fps
        window.setKeyRepeatEnabled(false);
        window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {W, H})));
    };
    setup();

    Textures tex;
    if (!tex.load()) return 1;
    Ui ui;
    if (!ui.loadFont()) return 1;

    Background background(tex.space);
    Game game(tex);
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = std::min(clock.restart().asSeconds(), 0.05f);   // ограничиваем dt, чтобы не было скачков

        // --- события ---
        bool clicked = false, esc = false, shieldPressed = false;
        while (const std::optional ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) window.close();
            if (auto* k = ev->getIf<sf::Event::KeyPressed>()) {
                if (k->code == sf::Keyboard::Key::Escape) esc = true;
                if (k->code == sf::Keyboard::Key::E) shieldPressed = true;
            }
            if (auto* m = ev->getIf<sf::Event::MouseButtonPressed>()) if (m->button == sf::Mouse::Button::Left) clicked = true;
        }
        // Esc в игре = пауза (флаг гасим, чтобы экран паузы сразу не закрылся)
        if (esc && game.st == State::Play) { game.st = State::Pause; esc = false; }
        if (shieldPressed) game.toggleBarrier();

        ui.begin(window, clicked);

        // --- фон живёт везде, кроме паузы ---
        if (game.st != State::Pause && game.st != State::Shop)
            background.update(dt, game.st == State::Menu || game.st == State::Settings);

        // --- обновление и отрисовка ---
        game.update(dt);
        window.clear();
        background.draw(window);
        game.draw(window);
        ui.drawHud(game);
        ui.drawScreens(game, esc);

        // --- переключение полного экрана ---
        if (ui.fullscreenToggled) {
            ui.fullscreenToggled = false;
            if (ui.full) window.create(sf::VideoMode::getDesktopMode(), "VOID SHIFT", sf::Style::Default, sf::State::Fullscreen);
            else window.create(sf::VideoMode({1280, 720}), "VOID SHIFT");
            setup();
        }
        window.display();
    }
    if (!game.saveProgress()) std::cerr << "Could not save progress\n";
    return 0;
}
