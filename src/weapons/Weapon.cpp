// [weapons]
#include "Weapon.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <algorithm>
#include <cmath>

void playerFire(std::vector<Bullet>& bl, V pos, int level) {
    for (int i = 0; i < level; i++) {
        // угол каждой пули: веер по 7° между соседними, симметрично вокруг центра
        float a = (i - (level - 1) / 2.f) * 7.f * PI / 180.f;
        bl.push_back({pos + V(0, -40), {std::sin(a) * 900.f, -std::cos(a) * 900.f}, 1, false});
    }
}

void enemyShoot(std::vector<Bullet>& bl, V pos, float ang, float sp) {
    bl.push_back({pos, {std::cos(ang) * sp, std::sin(ang) * sp}, 1, true});
}

void updateBullets(std::vector<Bullet>& bl, float dt) {
    for (auto& b : bl) {
        b.p += b.v * dt;
        if (b.p.x < -50 || b.p.x > W + 50 || b.p.y < -50 || b.p.y > H + 50) b.dead = true;
    }
}

void removeDeadBullets(std::vector<Bullet>& bl) {
    bl.erase(std::remove_if(bl.begin(), bl.end(), [](const Bullet& b) { return b.dead; }), bl.end());
}

void drawBullets(sf::RenderWindow& window, const std::vector<Bullet>& bl) {
    for (auto& b : bl) {
        sf::CircleShape c(b.enemy ? 7.f : 5.f);
        c.setOrigin({c.getRadius(), c.getRadius()});
        c.setPosition(b.p);
        // красные — вражеские, голубые — наши
        c.setFillColor(b.enemy ? sf::Color(255, 90, 90) : sf::Color(120, 230, 255));
        window.draw(c);
    }
}
