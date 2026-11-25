// [weapons] Одна пуля (и игрока, и врага).
#pragma once
#include "../common/Common.hpp"

struct Bullet {
    V p, v;             // позиция и скорость (пикселей в секунду)
    int dmg;            // урон
    bool enemy;         // true — пуля врага, false — пуля игрока
    bool dead = false;  // помечена на удаление
};
