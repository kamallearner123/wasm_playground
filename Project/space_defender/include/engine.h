#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <string>

// ─────────────────────────────────────────────
// Entity ID
// ─────────────────────────────────────────────
using EntityID = uint32_t;
static constexpr EntityID NULL_ENTITY = 0;

// ─────────────────────────────────────────────
// Game State
// ─────────────────────────────────────────────
enum class GamePhase : uint8_t {
    MENU, PLAYING, PAUSED, BOSS_FIGHT, GAME_OVER, VICTORY
};

struct GameConfig {
    int   screenW     = 800;
    int   screenH     = 600;
    float fixedDt     = 1.0f / 60.0f;  // 60 fps fixed step
    int   maxBullets  = 64;
    int   maxEnemies  = 32;
    int   maxPowerups = 8;
    int   totalLevels = 5;
};

// ─────────────────────────────────────────────
// Game Engine
// ─────────────────────────────────────────────
class GameEngine {
public:
    explicit GameEngine(const GameConfig& cfg);
    ~GameEngine() = default;

    // Lifecycle
    void init();
    void update(float dt);       // Called each fixed timestep
    void reset();

    // State queries
    GamePhase  getPhase()  const { return phase_; }
    int        getScore()  const { return score_; }
    int        getHealth() const { return playerHealth_; }
    int        getLevel()  const { return level_; }
    bool       isOver()    const { return phase_ == GamePhase::GAME_OVER; }

    // Input (set by platform layer)
    bool inputLeft  = false;
    bool inputRight = false;
    bool inputUp    = false;
    bool inputDown  = false;
    bool inputFire  = false;

    // Render callback (platform provides drawing primitives)
    using DrawRectFn = std::function<void(float x, float y, float w, float h,
                                          const char* color)>;
    using DrawTextFn = std::function<void(float x, float y, const char* text,
                                          const char* color)>;
    void setDrawCallbacks(DrawRectFn dr, DrawTextFn dt);
    void render();

    // WASI-mode: get text stats for logging
    std::string getStatsString() const;

    const GameConfig& config() const { return cfg_; }

private:
    void updatePlayer(float dt);
    void updateBullets(float dt);
    void updateEnemies(float dt);
    void updatePowerups(float dt);
    void checkCollisions();
    void spawnEnemyWave();
    void spawnBoss();
    void advanceLevel();
    void fireBullet(float x, float y, float vy, bool isEnemy);
    void renderEntities();

    GameConfig cfg_;
    GamePhase  phase_       = GamePhase::MENU;
    int        score_       = 0;
    int        highScore_   = 0;
    int        level_       = 1;
    int        playerHealth_= 3;
    float      accumulator_ = 0.0f;
    float      levelTimer_  = 0.0f;
    float      spawnTimer_  = 0.0f;
    float      fireTimer_   = 0.0f;
    int        enemiesKilled_= 0;
    int        totalFrames_ = 0;

    DrawRectFn drawRect_;
    DrawTextFn drawText_;

    // Entities (flat arrays — cache-friendly ECS-lite)
    struct Player {
        float x, y, w = 40, h = 40;
        float speed = 300.0f;
        float fireRate = 0.25f; // seconds between shots
    } player_;

    struct Bullet {
        float x, y, w = 6, h = 14;
        float vy;
        bool  active = false;
        bool  isEnemy = false;
    };

    struct Enemy {
        float x, y, w = 36, h = 36;
        float vx = 0, vy = 60.0f;
        int   hp = 1;
        bool  active = false;
        bool  isBoss = false;
        float shootTimer = 0.0f;
        float shootRate  = 1.5f;
        int   aiState    = 0; // 0=patrol,1=attack
    };

    struct PowerUp {
        float x, y, w = 22, h = 22;
        float vy = 80.0f;
        bool  active = false;
        int   type = 0; // 0=health, 1=speedBoost, 2=rapidFire
        float lifetime = 0.0f;
    };

    std::vector<Bullet>  bullets_;
    std::vector<Enemy>   enemies_;
    std::vector<PowerUp> powerups_;

    // Power-up state
    float speedBoostTimer_ = 0.0f;
    float rapidFireTimer_  = 0.0f;
};
