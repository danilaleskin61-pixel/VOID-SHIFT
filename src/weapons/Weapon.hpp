// [weapons] Стрельба, движение и отрисовка пуль.
#pragma once
#include "Bullet.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <vector>

// Выстрел игрока. level = число пуль веером (1..8) — это и есть "скейл оружия".
void playerFire(std::vector<Bullet>& bl, V pos, int level);

// Выстрел врага под углом ang (радианы) со скоростью sp.
void enemyShoot(std::vector<Bullet>& bl, V pos, float ang, float sp);

// Двигает пули; вылетевшие за экран помечает dead.
void updateBullets(std::vector<Bullet>& bl, float dt);

// Удаляет помеченные пули.
void removeDeadBullets(std::vector<Bullet>& bl);

void drawBullets(sf::RenderWindow& window, const std::vector<Bullet>& bl);
