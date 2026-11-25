// [textures]
#include "Textures.hpp"
#include <iostream>

bool Textures::load() {
    if (!space.loadFromFile("assets/textures/space.jpg") ||
        !ship.loadFromFile("assets/textures/spacecraft.png")) {
        std::cout << "Не найдены текстуры в assets/textures\n";
        return false;
    }
    return true;
}
