// [background-effects]
#include "Background.hpp"
#include <algorithm>
#include <cstdint>

Background::Background(const sf::Texture& t) : sprite(t) {
    // растягиваем фон так, чтобы он закрывал весь экран
    float s = std::max(W / t.getSize().x, H / t.getSize().y);
    sprite.setScale({s, s});
    height = t.getSize().y * s;
    bodies.push_back({{900, 250}, 110, 15, 0, 0, sf::Color(70, 110, 190)});  // стартовая планета
}

void Background::update(float dt, bool menuLike) {
    scroll += 25.f * dt;
    if (scroll >= height) scroll -= height;
    bodyT -= dt; rockT -= dt;

    // метеориты появляются часто (в меню — ещё чаще)
    if (rockT <= 0) {
        rockT = menuLike ? 0.35f : 1.1f;
        bodies.push_back({{rnd(0, W), -40.f}, rnd(8, 26), rnd(90, 220), rnd(0, 360), 2, sf::Color(120, 105, 95)});
    }
    // изредка — большая планета или станция
    if (bodyT <= 0) {
        bodyT = rnd(8, 16);
        if (rng() % 2)
            bodies.push_back({{rnd(100, W - 100), -200.f}, rnd(60, 150), 18, 0, 0,
                sf::Color((uint8_t)rnd(60, 200), (uint8_t)rnd(60, 200), (uint8_t)rnd(80, 230))});
        else
            bodies.push_back({{rnd(80, W - 80), -200.f}, 60, 45, 0, 1, sf::Color(150, 160, 175)});
    }
    for (auto& b : bodies) { b.p.y += b.speed * dt; b.rot += 40 * dt; }
    // удаляем тех, кто улетел за экран
    bodies.erase(std::remove_if(bodies.begin(), bodies.end(),
                 [](const Body& b) { return b.p.y > H + b.r + 250; }), bodies.end());
}

void Background::draw(sf::RenderWindow& window) {
    // фон рисуем дважды со сдвигом — получается бесшовная прокрутка
    for (int i = 0; i < 2; i++) {
        sprite.setPosition({0, scroll - height + i * height});
        window.draw(sprite);
    }
    for (auto& b : bodies) {
        if (b.kind == 0) {
            // планета: круг + тёмный полукруг сбоку для объёма
            sf::CircleShape c(b.r); c.setOrigin({b.r, b.r}); c.setPosition(b.p); c.setFillColor(b.c); window.draw(c);
            sf::CircleShape sh(b.r); sh.setOrigin({b.r, b.r}); sh.setPosition(b.p + V(b.r * 0.35f, 0));
            sh.setFillColor(sf::Color(0, 0, 0, 110)); window.draw(sh);
        } else if (b.kind == 1) {
            // станция: корпус + две солнечные панели
            sf::RectangleShape body({110, 26}); body.setOrigin({55, 13}); body.setPosition(b.p); body.setFillColor(b.c); window.draw(body);
            for (int s = -1; s <= 1; s += 2) {
                sf::RectangleShape pn({10, 60}); pn.setOrigin({5, 30}); pn.setPosition(b.p + V(s * 40.f, 0));
                pn.setFillColor(sf::Color(60, 100, 200)); window.draw(pn);
            }
        } else {
            // метеорит: вращающийся семиугольник
            sf::CircleShape c(b.r, 7); c.setOrigin({b.r, b.r}); c.setPosition(b.p);
            c.setRotation(sf::degrees(b.rot)); c.setFillColor(b.c); window.draw(c);
        }
    }
}
