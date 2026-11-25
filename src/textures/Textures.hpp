// [textures] Загрузка всех картинок игры в одном месте.
#pragma once
#include <SFML/Graphics/Texture.hpp>

struct Textures {
    sf::Texture space;   // фон космоса
    sf::Texture ship;    // корабль игрока

    // false, если какого-то файла нет. Новые текстуры добавляй сюда.
    bool load();
};
