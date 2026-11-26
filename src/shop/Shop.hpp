// [shop] Planned upgrades. Barrier purchase is implemented in Game / Ui.
#pragma once
#include <string>
#include <vector>

struct Upgrade {
    std::wstring name;   // название в магазине
    int level = 0;       // сколько раз куплено
    int maxLevel = 5;
    int baseCost = 20;   // цена первого уровня
    int cost() const { return baseCost * (level + 1); }   // каждый уровень дороже
};

// Список улучшений из ТЗ: урон, скорострельность, скорость пуль, доп. жизни, дроны, барьер.
inline std::vector<Upgrade> defaultUpgrades() {
    return {
        {L"Урон", 0, 5, 30},
        {L"Скорострельность", 0, 5, 30},
        {L"Скорость пуль", 0, 5, 20},
        {L"Доп. жизни", 0, 3, 50},
        {L"Дрон с автоатакой", 0, 3, 80},
        {L"Защитный барьер", 0, 1, 70},
    };
}
