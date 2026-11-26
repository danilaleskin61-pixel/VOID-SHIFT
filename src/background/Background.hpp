// [background-effects] Прокручивающийся космос + планеты, станции и метеориты на фоне.
#pragma once
#include "../common/Common.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

// Объект заднего плана
struct Body {
    V p;                 // позиция
    float r, speed, rot; // радиус, скорость падения вниз, угол поворота
    int kind;            // 0 планета, 1 станция, 2 метеорит
    sf::Color c;
};

class Background {
public:
    explicit Background(const sf::Texture& space);

    // menuLike = true в меню: метеоритов больше, чтобы фон был живее
    void update(float dt, bool menuLike);
    void draw(sf::RenderWindow& window);

private:
    sf::Sprite sprite;
    float height = 0;      // высота фона после масштабирования
    float scroll = 0;      // текущее смещение прокрутки
    float bodyT = 2;       // таймер до новой планеты/станции
    float rockT = 0;       // таймер до нового метеорита
    std::vector<Body> bodies;
};
