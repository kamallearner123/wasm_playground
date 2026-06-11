#include "engine.h"
#include "physics.h"
#include "score.h"
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <sstream>

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
GameEngine::GameEngine(const GameConfig& cfg) : cfg_(cfg) {
    bullets_.resize(cfg_.maxBullets);
    enemies_.resize(cfg_.maxEnemies);
    powerups_.resize(cfg_.maxPowerups);
}

// ─────────────────────────────────────────────
// Init
// ─────────────────────────────────────────────
void GameEngine::init() {
    player_.x = cfg_.screenW * 0.5f;
    player_.y = cfg_.screenH - 80.0f;
    playerHealth_ = 3;
    score_  = 0;
    level_  = 1;
    phase_  = GamePhase::PLAYING;
    spawnTimer_   = 0.0f;
    levelTimer_   = 0.0f;
    enemiesKilled_= 0;
    totalFrames_  = 0;
    speedBoostTimer_ = 0.0f;
    rapidFireTimer_  = 0.0f;
    for (auto& b : bullets_)  b.active = false;
    for (auto& e : enemies_)  e.active = false;
    for (auto& p : powerups_) p.active = false;
    spawnEnemyWave();
}

void GameEngine::reset() { init(); }

// ─────────────────────────────────────────────
// Set Render Callbacks
// ─────────────────────────────────────────────
void GameEngine::setDrawCallbacks(DrawRectFn dr, DrawTextFn dt) {
    drawRect_ = dr;
    drawText_ = dt;
}

// ─────────────────────────────────────────────
// Main Update (fixed timestep)
// ─────────────────────────────────────────────
void GameEngine::update(float dt) {
    if (phase_ != GamePhase::PLAYING && phase_ != GamePhase::BOSS_FIGHT) return;

    totalFrames_++;
    levelTimer_ += dt;

    updatePlayer(dt);
    updateBullets(dt);
    updateEnemies(dt);
    updatePowerups(dt);
    checkCollisions();

    // Power-up timers
    if (speedBoostTimer_ > 0) speedBoostTimer_ -= dt;
    if (rapidFireTimer_  > 0) rapidFireTimer_  -= dt;

    // Level progression: 10 enemies killed per level
    if (enemiesKilled_ >= level_ * 8 && phase_ != GamePhase::BOSS_FIGHT) {
        if (level_ < cfg_.totalLevels) {
            spawnBoss();
        }
    }
}

// ─────────────────────────────────────────────
// Player Update
// ─────────────────────────────────────────────
void GameEngine::updatePlayer(float dt) {
    float spd = player_.speed * (speedBoostTimer_ > 0 ? 1.6f : 1.0f);
    if (inputLeft  && player_.x > 20)                       player_.x -= spd * dt;
    if (inputRight && player_.x < cfg_.screenW - 20)        player_.x += spd * dt;
    if (inputUp    && player_.y > cfg_.screenH * 0.5f)      player_.y -= spd * dt;
    if (inputDown  && player_.y < cfg_.screenH - 20)        player_.y += spd * dt;

    // Auto-fire / manual fire
    fireTimer_ -= dt;
    float rate = (rapidFireTimer_ > 0) ? player_.fireRate * 0.3f : player_.fireRate;
    if (inputFire && fireTimer_ <= 0.0f) {
        fireBullet(player_.x, player_.y - player_.h * 0.5f, -500.0f, false);
        fireTimer_ = rate;
    }
}

// ─────────────────────────────────────────────
// Bullets Update
// ─────────────────────────────────────────────
void GameEngine::updateBullets(float dt) {
    for (auto& b : bullets_) {
        if (!b.active) continue;
        b.y += b.vy * dt;
        if (b.y < -20 || b.y > cfg_.screenH + 20) b.active = false;
    }
}

// ─────────────────────────────────────────────
// Enemies Update
// ─────────────────────────────────────────────
void GameEngine::updateEnemies(float dt) {
    // Spawn timer
    spawnTimer_ -= dt;
    if (spawnTimer_ <= 0 && phase_ == GamePhase::PLAYING) {
        spawnEnemyWave();
        spawnTimer_ = 3.0f - (level_ * 0.3f);
        if (spawnTimer_ < 0.8f) spawnTimer_ = 0.8f;
    }

    int alive = 0;
    for (auto& e : enemies_) {
        if (!e.active) continue;
        alive++;

        if (e.isBoss) {
            // Boss moves side to side, shoots in bursts
            e.x += e.vx * dt;
            e.y += e.vy * dt;
            if (e.x < 80 || e.x > cfg_.screenW - 80) { e.vx *= -1; }
            if (e.y > 150) { e.vy = 0; e.y = 150; }
            e.shootTimer -= dt;
            if (e.shootTimer <= 0) {
                // Spread shot
                fireBullet(e.x,      e.y + e.h * 0.5f, 300.0f, true);
                fireBullet(e.x - 20, e.y + e.h * 0.5f, 280.0f, true);
                fireBullet(e.x + 20, e.y + e.h * 0.5f, 280.0f, true);
                e.shootTimer = e.shootRate;
            }
        } else {
            // Regular enemy: sine-wave descent + occasional shot
            e.x += e.vx * dt + std::sin(levelTimer_ * 1.5f + e.y * 0.05f) * 1.2f;
            e.y += e.vy * dt;
            e.x = clampf(e.x, 20, cfg_.screenW - 20);
            e.shootTimer -= dt;
            if (e.shootTimer <= 0) {
                fireBullet(e.x, e.y + e.h * 0.5f, 220.0f + level_ * 20.0f, true);
                e.shootTimer = e.shootRate;
            }
            if (e.y > cfg_.screenH + 40) e.active = false;
        }
    }
}

// ─────────────────────────────────────────────
// Power-ups Update
// ─────────────────────────────────────────────
void GameEngine::updatePowerups(float dt) {
    for (auto& p : powerups_) {
        if (!p.active) continue;
        p.y += p.vy * dt;
        p.lifetime += dt;
        if (p.y > cfg_.screenH + 20 || p.lifetime > 10.0f) p.active = false;
    }
}

// ─────────────────────────────────────────────
// Collision Detection
// ─────────────────────────────────────────────
void GameEngine::checkCollisions() {
    AABB pBox = {player_.x - player_.w*0.5f, player_.y - player_.h*0.5f,
                 player_.w, player_.h};

    for (auto& b : bullets_) {
        if (!b.active) continue;
        AABB bBox = {b.x - b.w*0.5f, b.y - b.h*0.5f, b.w, b.h};

        if (!b.isEnemy) {
            // Player bullet vs enemies
            for (auto& e : enemies_) {
                if (!e.active) continue;
                AABB eBox = {e.x - e.w*0.5f, e.y - e.h*0.5f, e.w, e.h};
                if (bBox.overlaps(eBox)) {
                    b.active = false;
                    e.hp--;
                    if (e.hp <= 0) {
                        score_ += e.isBoss ? 500 * level_ : 100 * level_;
                        if (score_ > highScore_) highScore_ = score_;
                        e.active = false;
                        enemiesKilled_++;
                        // Chance to spawn power-up
                        if (rand() % 5 == 0) {
                            for (auto& p : powerups_) {
                                if (!p.active) {
                                    p = PowerUp{};
                                    p.x = e.x; p.y = e.y;
                                    p.type = rand() % 3;
                                    p.active = true;
                                    break;
                                }
                            }
                        }
                        if (e.isBoss) advanceLevel();
                    }
                    break;
                }
            }
        } else {
            // Enemy bullet vs player
            if (bBox.overlaps(pBox)) {
                b.active = false;
                playerHealth_--;
                if (playerHealth_ <= 0) phase_ = GamePhase::GAME_OVER;
            }
        }
    }

    // Enemy body vs player
    for (auto& e : enemies_) {
        if (!e.active) continue;
        AABB eBox = {e.x - e.w*0.5f, e.y - e.h*0.5f, e.w, e.h};
        if (eBox.overlaps(pBox)) {
            e.active = false;
            playerHealth_--;
            if (playerHealth_ <= 0) phase_ = GamePhase::GAME_OVER;
        }
    }

    // Power-up vs player
    for (auto& p : powerups_) {
        if (!p.active) continue;
        AABB puBox = {p.x - p.w*0.5f, p.y - p.h*0.5f, p.w, p.h};
        if (puBox.overlaps(pBox)) {
            p.active = false;
            if (p.type == 0 && playerHealth_ < 5) playerHealth_++;
            else if (p.type == 1) speedBoostTimer_ = 5.0f;
            else if (p.type == 2) rapidFireTimer_  = 5.0f;
            score_ += 50;
        }
    }
}

// ─────────────────────────────────────────────
// Spawning
// ─────────────────────────────────────────────
void GameEngine::fireBullet(float x, float y, float vy, bool isEnemy) {
    for (auto& b : bullets_) {
        if (!b.active) {
            b = Bullet{};
            b.x = x; b.y = y; b.vy = vy;
            b.isEnemy = isEnemy; b.active = true;
            return;
        }
    }
}

void GameEngine::spawnEnemyWave() {
    int count = 3 + level_ * 2;
    int spawned = 0;
    for (auto& e : enemies_) {
        if (!e.active && spawned < count) {
            e = Enemy{};
            e.x = 80.0f + (rand() % (cfg_.screenW - 160));
            e.y = -40.0f - spawned * 50.0f;
            e.hp = 1 + (level_ / 2);
            e.vy = 50.0f + level_ * 10.0f;
            e.vx = (rand() % 3 - 1) * 30.0f;
            e.shootRate = 2.0f - level_ * 0.15f;
            if (e.shootRate < 0.5f) e.shootRate = 0.5f;
            e.shootTimer = e.shootRate + (rand() % 100) * 0.01f;
            e.active = true;
            spawned++;
        }
    }
}

void GameEngine::spawnBoss() {
    for (auto& e : enemies_) {
        if (!e.active) {
            e = Enemy{};
            e.x = cfg_.screenW * 0.5f;
            e.y = -80;
            e.w = 80; e.h = 80;
            e.hp = 10 + level_ * 5;
            e.vx = 80.0f; e.vy = 60.0f;
            e.isBoss = true;
            e.shootRate = 1.2f;
            e.shootTimer = 1.0f;
            e.active = true;
            phase_ = GamePhase::BOSS_FIGHT;
            break;
        }
    }
}

void GameEngine::advanceLevel() {
    level_++;
    enemiesKilled_ = 0;
    if (level_ > cfg_.totalLevels) {
        phase_ = GamePhase::VICTORY;
    } else {
        phase_ = GamePhase::PLAYING;
        spawnEnemyWave();
    }
}

// ─────────────────────────────────────────────
// Render
// ─────────────────────────────────────────────
void GameEngine::render() {
    if (!drawRect_ || !drawText_) return;

    // Player — blue triangle-like shape
    drawRect_(player_.x - player_.w*0.5f, player_.y - player_.h*0.5f,
              player_.w, player_.h, "#4fc3f7");

    // Player bullets — yellow
    for (const auto& b : bullets_) {
        if (!b.active) continue;
        drawRect_(b.x - b.w*0.5f, b.y - b.h*0.5f, b.w, b.h,
                  b.isEnemy ? "#ff5252" : "#ffeb3b");
    }

    // Enemies
    for (const auto& e : enemies_) {
        if (!e.active) continue;
        drawRect_(e.x - e.w*0.5f, e.y - e.h*0.5f, e.w, e.h,
                  e.isBoss ? "#f44336" : "#9c27b0");
    }

    // Power-ups
    const char* puColors[] = {"#66bb6a","#ffa726","#26c6da"};
    for (const auto& p : powerups_) {
        if (!p.active) continue;
        drawRect_(p.x - p.w*0.5f, p.y - p.h*0.5f, p.w, p.h, puColors[p.type]);
    }
}

// ─────────────────────────────────────────────
// Stats (WASI)
// ─────────────────────────────────────────────
std::string GameEngine::getStatsString() const {
    std::ostringstream ss;
    ss << "Frame=" << totalFrames_
       << " Level=" << level_
       << " Score=" << score_
       << " HP=" << playerHealth_
       << " Killed=" << enemiesKilled_
       << " Phase=" << static_cast<int>(phase_);
    return ss.str();
}
