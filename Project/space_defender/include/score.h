#pragma once
#include <string>

// ─────────────────────────────────────────────
// ScoreManager
// ─────────────────────────────────────────────
class ScoreManager {
public:
    ScoreManager() = default;

    void reset()                { score_ = 0; multiplier_ = 1; combo_ = 0; }
    void addKill(bool isBoss)   {
        int pts = isBoss ? 500 : 100;
        pts *= multiplier_;
        score_ += pts;
        combo_++;
        if (combo_ >= 5 && multiplier_ < 8) multiplier_ *= 2;
        if (score_ > highScore_) highScore_ = score_;
    }
    void addPowerup()           { score_ += 50 * multiplier_; }
    void resetCombo()           { combo_ = 0; multiplier_ = 1; }

    int score()      const { return score_; }
    int highScore()  const { return highScore_; }
    int multiplier() const { return multiplier_; }
    int combo()      const { return combo_; }

    std::string toString() const;

private:
    int score_     = 0;
    int highScore_ = 0;
    int multiplier_= 1;
    int combo_     = 0;
};

// ─────────────────────────────────────────────
// ObjectPool — Pre-allocated fixed-size pool
// ─────────────────────────────────────────────
template<typename T, int N>
class ObjectPool {
public:
    ObjectPool() { pool_.resize(N); }

    T* acquire() {
        for (auto& obj : pool_)
            if (!obj.active) { obj = T{}; obj.active = true; return &obj; }
        return nullptr; // pool exhausted
    }

    void release(T* obj) { if (obj) obj->active = false; }

    template<typename Fn>
    void forEach(Fn fn) { for (auto& obj : pool_) if (obj.active) fn(obj); }

    int activeCount() const {
        int n = 0;
        for (const auto& obj : pool_) if (obj.active) n++;
        return n;
    }

private:
    std::vector<T> pool_;
};
