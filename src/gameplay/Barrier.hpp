// Barrier rules independent of SFML, so they can be tested without a window.
#pragma once
#include <algorithm>
#include <cmath>

class Barrier {
public:
    static constexpr int kPrice = 70;
    static constexpr float kMaxEnergy = 100.f;
    static constexpr float kEnergyPerSecond = 12.5f;
    static constexpr float kOrbEnergy = 25.f;
    static constexpr float kCooldown = 45.f;

    bool owned() const { return unlocked; }
    bool active() const { return enabled; }
    float energy() const { return energyLeft; }
    float cooldown() const { return cooldownLeft; }

    // A permanent purchase: never charge twice for the same barrier.
    bool buy(int& coins) {
        if (unlocked || coins < kPrice) return false;
        coins -= kPrice;
        unlocked = true;
        return true;
    }

    void restorePurchase(bool purchased) { unlocked = purchased; }

    // Starting a new run preserves the purchase but resets combat resources.
    void resetRun() {
        enabled = false;
        energyLeft = kMaxEnergy;
        cooldownLeft = 0.f;
    }

    bool toggle() {
        if (enabled) {
            stop();
            return true;
        }
        if (!unlocked || cooldownLeft > 0.f || energyLeft <= 0.f) return false;
        enabled = true;
        return true;
    }

    // Any shutdown starts the cooldown; toggling cannot bypass this rule.
    void stop() {
        if (!enabled) return;
        enabled = false;
        cooldownLeft = kCooldown;
    }

    void update(float dt) {
        if (!std::isfinite(dt) || dt <= 0.f) return;
        cooldownLeft = std::max(0.f, cooldownLeft - dt);
        if (enabled) {
            energyLeft = std::max(0.f, energyLeft - kEnergyPerSecond * dt);
            if (energyLeft <= 0.f) stop();
        }
    }

    void collectEnergy() {
        energyLeft = std::min(kMaxEnergy, energyLeft + kOrbEnergy);
    }

private:
    bool unlocked = false;
    bool enabled = false;
    float energyLeft = kMaxEnergy;
    float cooldownLeft = 0.f;
};
