// [textures]
#include "Textures.hpp"
#include <iostream>

bool Textures::load() {
    if (!space.loadFromFile("assets/textures/space.jpg") ||
        !ship.loadFromFile("assets/textures/spacecraft.png")) {
        std::cout << "Не найдены текстуры в assets/textures\n";
        return false;
    }
    const char* files[] = {
        "assets/textures/enemy_basic.png",
        "assets/textures/enemy_weaver.png",
        "assets/textures/enemy_shooter.png",
        "assets/textures/boss.png",
    };
    for (std::size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i].loadFromFile(files[i])) {
            std::cerr << "Missing texture: " << files[i] << '\n';
            return false;
        }
        enemies[i].setSmooth(false);  // Keep the pixel-art edges sharp.
    }
    return true;
}
