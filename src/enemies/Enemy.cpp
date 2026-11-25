// [enemies]
#include "Enemy.hpp"
#include "../weapons/Weapon.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <cmath>

Enemy spawnEnemy(int level) {
    Enemy e{};
    e.type = rng() % 3;
    e.p = {rnd(60, W - 60), -40.f};
    e.bx = e.p.x;
    e.shoot = rnd(0.5f, 1.5f);
    // здоровье зависит от типа и растёт с уровнем
    e.hp = e.maxhp = e.type == 0 ? 2.f + level : e.type == 1 ? 3.f + level : 5.f + 2.f * level;
    return e;
}

Enemy spawnBoss(int level) {
    Enemy b{};
    b.p = {W / 2, -120.f};
    b.type = 3;
    b.boss = true;
    b.hp = b.maxhp = 60.f + 45.f * level;
    b.shoot = 2.f;
    return b;
}

float enemyHitRadius(const Enemy& e) { return e.boss ? 75.f : 26.f; }
float enemyRamRadius(const Enemy& e) { return e.boss ? 85.f : 40.f; }

void updateEnemy(Enemy& e, float dt, V pp, int level, std::vector<Bullet>& bl) {
    e.t += dt;
    if (e.type == 0) {
        // тип 0: просто летит вниз
        e.p.y += (140 + level * 6) * dt;
    } else if (e.type == 1) {
        // тип 1: летит вниз, покачиваясь влево-вправо
        e.p.y += 120 * dt;
        e.p.x = e.bx + std::sin(e.t * 3) * 110;
    } else if (e.type == 2) {
        // тип 2: выезжает, зависает на высоте 200 и стреляет в игрока
        if (e.p.y < 200) e.p.y += 150 * dt; else e.p.x = e.bx + std::sin(e.t * 1.5f) * 80;
        e.shoot -= dt;
        if (e.shoot <= 0) {
            e.shoot = 1.8f;
            enemyShoot(bl, e.p, std::atan2(pp.y - e.p.y, pp.x - e.p.x), 300);
        }
    } else {
        // босс: выезжает сверху, потом ходит по горизонтали
        if (e.p.y < 130) e.p.y += 100 * dt; else e.p.x = W / 2 + std::sin(e.t * 0.8f) * 400;
        e.shoot -= dt;
        if (e.shoot <= 0 && e.p.y >= 130) {
            bool low = e.hp < e.maxhp / 2;      // меньше половины hp — атакует чаще
            e.shoot = low ? 0.9f : 1.4f;
            int pat = e.atk++ % 3;              // три паттерна по кругу
            if (pat == 0) {
                // веер вниз
                for (int i = -3; i <= 3; i++) enemyShoot(bl, e.p, PI / 2 + i * 0.18f, 260);
            } else if (pat == 1) {
                // быстрая очередь по игроку
                float a = std::atan2(pp.y - e.p.y, pp.x - e.p.x);
                for (int i = -1; i <= 1; i++) enemyShoot(bl, e.p, a + i * 0.1f, 380);
            } else if (low) {
                // кольцо пуль (только на второй половине боя)
                for (int i = 0; i < 14; i++) enemyShoot(bl, e.p, i * 2 * PI / 14, 220);
            } else {
                // узкий веер
                for (int i = -2; i <= 2; i++) enemyShoot(bl, e.p, PI / 2 + i * 0.3f, 300);
            }
        }
    }
}

void drawEnemies(sf::RenderWindow& window, const std::vector<Enemy>& en) {
    // цвет по типу: 0 красный, 1 оранжевый, 2 фиолетовый, 3 босс
    static const sf::Color cols[4] = {{220, 80, 80}, {230, 150, 60}, {170, 90, 220}, {200, 40, 120}};
    for (auto& e : en) {
        float r = enemyHitRadius(e);
        // форма зависит от типа: треугольник, ромб, шестиугольник, восьмиугольник (босс)
        sf::CircleShape c(r, e.type == 0 ? 3 : e.type == 1 ? 4 : e.boss ? 8 : 6);
        c.setOrigin({r, r});
        c.setPosition(e.p);
        c.setRotation(sf::degrees(e.boss ? 0.f : 180.f));
        c.setFillColor(cols[e.type]);
        c.setOutlineThickness(3);
        c.setOutlineColor(sf::Color(255, 255, 255, 160));
        window.draw(c);
    }
}
