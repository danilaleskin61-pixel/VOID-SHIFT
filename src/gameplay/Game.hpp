// [gameplay] Состояние забега: игрок, волны, столкновения, дропы, эффекты.
#pragma once
#include "../common/Common.hpp"
#include "../enemies/Enemy.hpp"
#include "../weapons/Bullet.hpp"
#include "../textures/Textures.hpp"
#include "Barrier.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

// Экраны игры
enum class State { Menu, Settings, Play, Pause, LevelDone, GameOver, Shop };

struct Drop { V p; int kind; };       // 0 coin, 1 weapon, 2 life, 3 rage, 4 energy
struct Fx   { V p; float t, life, r; sf::Color c; };     // вспышка/взрыв (круг, который растёт и гаснет)

class Game {
public:
    explicit Game(const Textures& textures);

    void resetRun();     // новая игра с нуля
    void nextLevel();    // переход на следующий уровень
    void update(float dt);                  // обновление (работает только в State::Play)
    void draw(sf::RenderWindow& window);    // мир: дропы, враги, пули, корабль, эффекты
    bool inRun() const {
        return st != State::Menu && st != State::Settings &&
               (st != State::Shop || shopReturn != State::Menu);
    }
    void openShop();
    void closeShop();
    bool buyBarrier();
    void toggleBarrier();
    bool saveProgress() const;
    const Barrier& getBarrier() const { return barrier; }

    State st = State::Menu;

    // --- данные игрока (читает интерфейс для HUD) ---
    V pp{W / 2, H - 120};
    int lives = 3, coins = 0, wpn = 1, level = 1, wave = 1, score = 0;
    float inv = 0;       // таймер неуязвимости после удара
    float rage = 0;      // таймер ярости (неуязвимость + быстрая стрельба)

    std::vector<Enemy> en;   // враги (интерфейс рисует по ним полоску босса)

private:
    void startWave();
    enum class Damage { Bullet, Collision };
    void hurt(Damage damage);
    void loadProgress();
    void onKill(const Enemy& e);     // враг убит: очки, взрыв, дроп
    void addFx(V p, float life, float r, sf::Color c);

    const Textures& textures;
    sf::Sprite ship;
    Barrier barrier;
    State shopReturn = State::Menu;
    std::vector<Bullet> bl;
    std::vector<Drop> dr;
    std::vector<Fx> fx;
    int toSpawn = 0;
    float fireT = 0, spawnT = 0;
};
