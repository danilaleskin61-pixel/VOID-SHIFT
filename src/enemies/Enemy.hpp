// [textures] Загрузка всех картинок игры в одном месте.
#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <array>

struct Textures {
    sf::Texture space;   // фон космоса
    sf::Texture ship;    // корабль игрока
    // 0: interceptor, 1: weaver, 2: shooter, 3: boss.
    std::array<sf::Texture, 4> enemies;

    // false, если какого-то файла нет. Новые текстуры добавляй сюда.
    bool load();
};
