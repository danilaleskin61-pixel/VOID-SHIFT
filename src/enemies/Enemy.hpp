// [enemies] Враги и босс: создание, поведение, отрисовка.
#pragma once
#include "../common/Common.hpp"
#include "../weapons/Bullet.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <vector>

struct Enemy {
    V p;                  // позиция
    int type;             // 0 — летит вниз, 1 — змейкой, 2 — стреляет, 3 — босс
    float hp, maxhp;      // здоровье (задаётся при создании)
    float t = 0;          // время жизни (для синусоид)
    float shoot = 1.f;    // таймер до следующего выстрела
    float bx = 0;         // базовая X-координата для покачивания
    int atk = 0;          // счётчик атак босса (выбор паттерна)
    bool boss = false;
};

// Обычный враг случайного типа; чем выше level, тем больше hp.
Enemy spawnEnemy(int level);

// Босс 4-й волны: полоска hp и несколько видов атак.
Enemy spawnBoss(int level);

// Радиус попадания пули и радиус столкновения с кораблём игрока.
float enemyHitRadius(const Enemy& e);
float enemyRamRadius(const Enemy& e);

// Движение и стрельба одного врага (пули добавляются в bl).
void updateEnemy(Enemy& e, float dt, V playerPos, int level, std::vector<Bullet>& bl);

void drawEnemies(sf::RenderWindow& window, const std::vector<Enemy>& en);
