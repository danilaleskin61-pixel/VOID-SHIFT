// [gameplay]
#include "Game.hpp"
#include "../weapons/Weapon.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
constexpr float kBarrierRadius = 64.f;
constexpr float kPlayerHitRadius = 22.f;
constexpr float kEnemyBulletRadius = 7.f;
}

Game::Game(const Textures& tex, Sfx& s) : textures(tex), ship(tex.ship), sfx(s) {
    ship.setOrigin(V(tex.ship.getSize()) / 2.f);
    ship.setScale({0.2f, 0.2f});
    loadProgress();
    sfx.play("gamestart", 80.f);
}

void Game::loadProgress() {
    std::ifstream file("save/progress.txt");
    std::string version;
    int savedCoins = 0, purchased = 0;
    if (file >> version >> savedCoins >> purchased &&
        version == "VOID_SHIFT_V1" && savedCoins >= 0 && savedCoins <= 100000000 &&
        (purchased == 0 || purchased == 1)) {
        coins = savedCoins;
        barrier.restorePurchase(purchased == 1);
    }
}

bool Game::saveProgress() const {
    std::error_code error;
    std::filesystem::create_directories("save", error);
    if (error) return false;
    std::ofstream file("save/progress.txt");
    file << "VOID_SHIFT_V1\n" << coins << '\n' << barrier.owned() << '\n';
    file.flush();
    return file.good();
}

void Game::openShop() {
    if (st == State::Shop || st == State::Settings) return;
    shopReturn = st;
    st = State::Shop;
}

void Game::closeShop() { if (st == State::Shop) st = shopReturn; }

bool Game::buyBarrier() {
    if (st != State::Shop || !barrier.buy(coins)) return false;
    if (!saveProgress()) std::cerr << "Could not save shop progress\n";
    return true;
}

void Game::toggleBarrier() {
    if (st == State::Play) barrier.toggle();
}

void Game::addFx(V p, float life, float r, sf::Color c) { fx.push_back({p, 0, life, r, c}); }

void Game::startWave() {
    bl.clear();
    toSpawn = wave < 4 ? 5 + level * 2 : 0; // волны 1-3: обычные враги
    spawnT = 1.f;
    if (wave == 4) en.push_back(spawnBoss(level)); // волна 4: босс
}

void Game::resetRun() {
    en.clear(); bl.clear(); dr.clear(); fx.clear();
    pp = {W / 2, H - 120};
    lives = 3; wpn = 1; level = 1; wave = 1; score = 0; inv = 0; rage = 0;
    barrier.resetRun();
    fireT = 0.f;
    startWave();
}

void Game::nextLevel() {
    level++; wave = 1; dr.clear();
    startWave();
    st = State::Play;
}

void Game::hurt(Damage damage) {
    if (st != State::Play) return;
    if (barrier.active()) {
        if (damage == Damage::Collision) {
            barrier.stop();
            inv = std::max(inv, 0.7f); // The shield absorbs this impact.
            addFx(pp, 0.4f, 90.f, sf::Color(60, 170, 255));
        }
        return;
    }
    if (inv > 0 || rage > 0) return; // неуязвимость / ярость
    
    lives--;
    sfx.play("loselife", 80.f); // <-- Звук потери жизни
    inv = 2.f; // 2 секунды мигания
    addFx(pp, 0.5f, 50, sf::Color(255, 150, 40));
    
    if (lives <= 0) {
        st = State::GameOver;
        sfx.play("gameover", 100.f); // <-- Звук смерти (Game Over)
        if (!saveProgress()) std::cerr << "Could not save progress\n";
    }
}

void Game::onKill(const Enemy& e) {
    score += e.boss ? 500 : 10 * level;
    sfx.play("explosion", 70.f); // <-- Звук взрыва врага
    addFx(e.p, e.boss ? 1.0f : 0.5f, e.boss ? 140.f : 45.f, sf::Color(255, 170, 50)); // взрыв врага
    if (e.boss) {
        coins += 30 + level * 10;
        if (!saveProgress()) std::cerr << "Could not save progress\n";
        return;
    }
    // Coin 55%, weapon 7%, life 3%, rage 4%, blue energy sphere 15%.
    float r = rnd(0, 1);
    if (r < 0.55f) dr.push_back({e.p, 0});
    else if (r < 0.62f) dr.push_back({e.p, 1});
    else if (r < 0.65f) dr.push_back({e.p, 2});
    else if (r < 0.69f) dr.push_back({e.p, 3});
    else if (r < 0.84f) dr.push_back({e.p, 4});
}

void Game::update(float dt) {
    if (st != State::Play) return;

    // --- управление: стрелки или WASD ---
    using K = sf::Keyboard::Key;
    float sp = 380.f * dt;
    if (sf::Keyboard::isKeyPressed(K::Left) || sf::Keyboard::isKeyPressed(K::A)) pp.x -= sp;
    if (sf::Keyboard::isKeyPressed(K::Right) || sf::Keyboard::isKeyPressed(K::D)) pp.x += sp;
    if (sf::Keyboard::isKeyPressed(K::Up) || sf::Keyboard::isKeyPressed(K::W)) pp.y -= sp;
    if (sf::Keyboard::isKeyPressed(K::Down) || sf::Keyboard::isKeyPressed(K::S)) pp.y += sp;
    pp.x = std::clamp(pp.x, 40.f, W - 40.f);
    pp.y = std::clamp(pp.y, 40.f, H - 40.f);
    inv = std::max(0.f, inv - dt);
    rage = std::max(0.f, rage - dt);
    barrier.update(dt);

    // --- автоогонь (в ярости стреляем вдвое быстрее) ---
    fireT -= dt;
    if (fireT <= 0) {
        fireT = rage > 0 ? 0.09f : 0.18f;
        playerFire(bl, pp, wpn);
        sfx.play("lasershot", 50.f); // <-- Звук выстрела
    }

    // --- спавн врагов текущей волны ---
    spawnT -= dt;
    if (toSpawn > 0 && spawnT <= 0) {
        spawnT = 0.9f; toSpawn--;
        en.push_back(spawnEnemy(level));
    }

    // --- враги ---
    for (auto& e : en) updateEnemy(e, dt, pp, level, bl);
    // обычные враги, улетевшие вниз, пропадают
    en.erase(std::remove_if(en.begin(), en.end(), [](const Enemy& e) { return !e.boss && e.p.y > H + 80; }), en.end());

    // --- пули и попадания ---
    updateBullets(bl, dt);
    for (auto& b : bl) {
        if (b.dead) continue;
        if (b.enemy) {
            float radius = barrier.active() ? kBarrierRadius + kEnemyBulletRadius
                                            : kPlayerHitRadius;
            if (len2(b.p, pp) < radius * radius) {
                b.dead = true;
                if (barrier.active())
                    addFx(b.p, 0.12f, 12.f, sf::Color(100, 210, 255));
                hurt(Damage::Bullet);
            }
            continue;
        }
        for (auto& e : en) {
            float r = enemyHitRadius(e);
            if (e.hp > 0 && len2(b.p, e.p) < r * r) {
                b.dead = true;
                e.hp -= (float)b.dmg;
                addFx(b.p, 0.15f, 14, sf::Color(255, 240, 120)); // маленькая вспышка попадания
                break;
            }
        }
    }
    removeDeadBullets(bl);

    // --- смерть врагов и столкновение с кораблём ---
    for (auto& e : en) {
        float rr = barrier.active() ? kBarrierRadius + enemyHitRadius(e)
                                    : enemyRamRadius(e);
        if (e.hp <= 0) onKill(e);
        else if (len2(e.p, pp) < rr * rr) {
            hurt(Damage::Collision);
            if (!e.boss) { e.hp = 0; onKill(e); }
        }
    }
    en.erase(std::remove_if(en.begin(), en.end(), [](const Enemy& e) { return e.hp <= 0; }), en.end());

    // --- дропы: падают вниз, подбираются касанием ---
    for (auto& d : dr) d.p.y += 70 * dt;
    for (auto& d : dr) if (len2(d.p, pp) < 36.f * 36.f) {
        if (d.kind == 0) {
            coins++;
            sfx.play("coin", 60.f); // <-- Звук монеты
            if (!saveProgress()) std::cerr << "Could not save progress\n";
        }
        else if (d.kind == 1) { wpn = std::min(8, wpn + 1); sfx.play("upgrade", 70.f); } // апгрейд оружия
        else if (d.kind == 2) { lives = std::min(9, lives + 1); sfx.play("extralife", 70.f); } // доп жизнь
        else if (d.kind == 3) { rage = 6.f; sfx.play("upgrade", 70.f); } // ярость
        else if (d.kind == 4) { barrier.collectEnergy(); sfx.play("coin", 80.f); } // энергия щита
        d.p.y = 9999; // помечаем на удаление
    }
    dr.erase(std::remove_if(dr.begin(), dr.end(), [](const Drop& d) { return d.p.y > H + 20; }), dr.end());

    // --- эффекты ---
    for (auto& f : fx) f.t += dt;
    fx.erase(std::remove_if(fx.begin(), fx.end(), [](const Fx& f) { return f.t >= f.life; }), fx.end());

    // --- конец волны / уровня ---
    if (st == State::Play && toSpawn == 0 && en.empty()) {
        if (++wave > 4) st = State::LevelDone; else startWave();
    }
}

void Game::draw(sf::RenderWindow& window) {
    if (!inRun()) return;

    // Weapons are purple now; blue is reserved for energy spheres.
    static const sf::Color cols[5] = {{255, 210, 40}, {190, 100, 255},
        {80, 230, 90}, {255, 70, 70}, {40, 160, 255}};
    for (auto& d : dr) {
        sf::CircleShape c(10); c.setOrigin({10, 10}); c.setPosition(d.p);
        c.setOutlineThickness(2); c.setOutlineColor(sf::Color::White);
        c.setFillColor(cols[d.kind]);
        window.draw(c);
        if (d.kind == 4) {
            sf::CircleShape core(4.f);
            core.setOrigin({4.f, 4.f}); core.setPosition(d.p);
            core.setFillColor(sf::Color(180, 235, 255));
            window.draw(core);
        }
    }
    drawEnemies(window, en, textures.enemies);
    drawBullets(window, bl);

    // корабль мигает, пока действует неуязвимость
    if (st != State::GameOver && (inv <= 0 || std::fmod(inv, 0.2f) < 0.1f)) {
        ship.setPosition(pp);
        ship.setColor(rage > 0 ? sf::Color(255, 140, 140) : sf::Color::White); // в ярости краснеет
        window.draw(ship);
    }
    if (barrier.active() && st != State::GameOver) {
        sf::CircleShape ring(kBarrierRadius, 64);
        ring.setOrigin({kBarrierRadius, kBarrierRadius});
        ring.setPosition(pp);
        ring.setFillColor(sf::Color(40, 140, 255, 35));
        ring.setOutlineColor(sf::Color(100, 220, 255, 220));
        ring.setOutlineThickness(3.f);
        window.draw(ring);
    }
    // взрывы: круг растёт и затухает
    for (auto& f : fx) {
        float k = f.t / f.life, r = f.r * (0.3f + 0.7f * k);
        sf::CircleShape c(r); c.setOrigin({r, r}); c.setPosition(f.p);
        c.setFillColor(sf::Color(f.c.r, f.c.g, f.c.b, (uint8_t)(220 * (1 - k))));
        window.draw(c);
    }
}
