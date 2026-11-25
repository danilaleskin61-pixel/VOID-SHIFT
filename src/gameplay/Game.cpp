// [gameplay]
#include "Game.hpp"
#include "../weapons/Weapon.hpp"
#include <algorithm>
#include <cmath>

Game::Game(const sf::Texture& shipTex) : ship(shipTex) {
    ship.setOrigin(V(shipTex.getSize()) / 2.f);
    ship.setScale({0.2f, 0.2f});
}

void Game::addFx(V p, float life, float r, sf::Color c) { fx.push_back({p, 0, life, r, c}); }

void Game::startWave() {
    bl.clear();
    toSpawn = wave < 4 ? 5 + level * 2 : 0;   // волны 1-3: обычные враги
    spawnT = 1.f;
    if (wave == 4) en.push_back(spawnBoss(level));   // волна 4: босс
}

void Game::resetRun() {
    en.clear(); bl.clear(); dr.clear(); fx.clear();
    pp = {W / 2, H - 120};
    lives = 3; coins = 0; wpn = 1; level = 1; wave = 1; score = 0; inv = 0; rage = 0;
    startWave();
}

void Game::nextLevel() {
    level++; wave = 1; dr.clear();
    startWave();
    st = State::Play;
}

void Game::hurt() {
    if (inv > 0 || rage > 0) return;   // неуязвимость / ярость
    lives--;
    inv = 2.f;                         // 2 секунды мигания
    addFx(pp, 0.5f, 50, sf::Color(255, 150, 40));
    if (lives <= 0) st = State::GameOver;
}

void Game::onKill(const Enemy& e) {
    score += e.boss ? 500 : 10 * level;
    addFx(e.p, e.boss ? 1.0f : 0.5f, e.boss ? 140.f : 45.f, sf::Color(255, 170, 50));   // взрыв врага
    if (e.boss) { coins += 30 + level * 10; return; }
    // шансы дропа: монета 55%, оружие 7%, жизнь 3%, ярость 4%
    float r = rnd(0, 1);
    if (r < 0.55f) dr.push_back({e.p, 0});
    else if (r < 0.62f) dr.push_back({e.p, 1});
    else if (r < 0.65f) dr.push_back({e.p, 2});
    else if (r < 0.69f) dr.push_back({e.p, 3});
}

void Game::update(float dt) {
    if (st != State::Play) return;

    // --- управление: стрелки или WASD ---
    using K = sf::Keyboard::Key;
    float sp = 380.f * dt;
    if (sf::Keyboard::isKeyPressed(K::Left)  || sf::Keyboard::isKeyPressed(K::A)) pp.x -= sp;
    if (sf::Keyboard::isKeyPressed(K::Right) || sf::Keyboard::isKeyPressed(K::D)) pp.x += sp;
    if (sf::Keyboard::isKeyPressed(K::Up)    || sf::Keyboard::isKeyPressed(K::W)) pp.y -= sp;
    if (sf::Keyboard::isKeyPressed(K::Down)  || sf::Keyboard::isKeyPressed(K::S)) pp.y += sp;
    pp.x = std::clamp(pp.x, 40.f, W - 40.f);
    pp.y = std::clamp(pp.y, 40.f, H - 40.f);
    inv = std::max(0.f, inv - dt);
    rage = std::max(0.f, rage - dt);

    // --- автоогонь (в ярости стреляем вдвое быстрее) ---
    fireT -= dt;
    if (fireT <= 0) {
        fireT = rage > 0 ? 0.09f : 0.18f;
        playerFire(bl, pp, wpn);
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
        if (b.enemy) {
            if (len2(b.p, pp) < 22.f * 22.f) { b.dead = true; hurt(); }
            continue;
        }
        for (auto& e : en) {
            float r = enemyHitRadius(e);
            if (e.hp > 0 && len2(b.p, e.p) < r * r) {
                b.dead = true;
                e.hp -= (float)b.dmg;
                addFx(b.p, 0.15f, 14, sf::Color(255, 240, 120));   // маленькая вспышка попадания
                break;
            }
        }
    }
    removeDeadBullets(bl);

    // --- смерть врагов и столкновение с кораблём ---
    for (auto& e : en) {
        float rr = enemyRamRadius(e);
        if (e.hp <= 0) onKill(e);
        else if (len2(e.p, pp) < rr * rr) { hurt(); if (!e.boss) { e.hp = 0; onKill(e); } }
    }
    en.erase(std::remove_if(en.begin(), en.end(), [](const Enemy& e) { return e.hp <= 0; }), en.end());

    // --- дропы: падают вниз, подбираются касанием ---
    for (auto& d : dr) d.p.y += 70 * dt;
    for (auto& d : dr) if (len2(d.p, pp) < 36.f * 36.f) {
        if (d.kind == 0) coins++;
        else if (d.kind == 1) wpn = std::min(8, wpn + 1);     // апгрейд оружия, максимум 8
        else if (d.kind == 2) lives = std::min(9, lives + 1);
        else rage = 6.f;                                      // ярость на 6 секунд
        d.p.y = 9999;                                         // помечаем на удаление
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

    // дропы: жёлтый — монета, голубой — оружие, зелёный — жизнь, красный — ярость
    static const sf::Color cols[4] = {{255, 210, 40}, {60, 220, 255}, {80, 230, 90}, {255, 70, 70}};
    for (auto& d : dr) {
        sf::CircleShape c(10); c.setOrigin({10, 10}); c.setPosition(d.p);
        c.setOutlineThickness(2); c.setOutlineColor(sf::Color::White);
        c.setFillColor(cols[d.kind]);
        window.draw(c);
    }
    drawEnemies(window, en);
    drawBullets(window, bl);

    // корабль мигает, пока действует неуязвимость
    if (st != State::GameOver && (inv <= 0 || std::fmod(inv, 0.2f) < 0.1f)) {
        ship.setPosition(pp);
        ship.setColor(rage > 0 ? sf::Color(255, 140, 140) : sf::Color::White);   // в ярости краснеет
        window.draw(ship);
    }
    // взрывы: круг растёт и затухает
    for (auto& f : fx) {
        float k = f.t / f.life, r = f.r * (0.3f + 0.7f * k);
        sf::CircleShape c(r); c.setOrigin({r, r}); c.setPosition(f.p);
        c.setFillColor(sf::Color(f.c.r, f.c.g, f.c.b, (uint8_t)(220 * (1 - k))));
        window.draw(c);
    }
}
