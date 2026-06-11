/**
 * Unit Tests — Space Defender Engine (Native Build)
 * Build: g++ -std=c++20 -I../include tests/test_engine.cpp engine/engine.cpp ai/ai.cpp -o test_engine
 */

#include <cassert>
#include <iostream>
#include <string>
#include "engine.h"
#include "physics.h"
#include "ai.h"
#include "score.h"

// ─────────────────────────────────────────────
// Test helpers
// ─────────────────────────────────────────────
static int passed = 0, failed = 0;

#define TEST(name, expr) do { \
    if (expr) { std::cout << "  ✅ " << name << "\n"; passed++; } \
    else      { std::cout << "  ❌ FAIL: " << name << " [line " << __LINE__ << "]\n"; failed++; } \
} while(0)

// ─────────────────────────────────────────────
// Vec2 & Physics Tests
// ─────────────────────────────────────────────
void testPhysics() {
    std::cout << "\n[Physics]\n";

    Vec2 a{3, 4};
    TEST("Vec2 length",    std::abs(a.length() - 5.0f) < 0.001f);

    Vec2 n = a.normalized();
    TEST("Vec2 normalized", std::abs(n.length() - 1.0f) < 0.001f);

    Vec2 sum = Vec2{1,2} + Vec2{3,4};
    TEST("Vec2 add",       sum.x == 4 && sum.y == 6);

    AABB box1{0, 0, 10, 10};
    AABB box2{5, 5, 10, 10};
    AABB box3{15, 0, 10, 10};
    TEST("AABB overlap",   box1.overlaps(box2));
    TEST("AABB no-overlap",!box1.overlaps(box3));

    TEST("clampf lo", clampf(-5, 0, 10) == 0);
    TEST("clampf hi", clampf(15, 0, 10) == 10);
    TEST("clampf mid",clampf(5,  0, 10) == 5);
}

// ─────────────────────────────────────────────
// Score Manager Tests
// ─────────────────────────────────────────────
void testScore() {
    std::cout << "\n[ScoreManager]\n";

    ScoreManager sm;
    TEST("Initial score", sm.score() == 0);
    TEST("Initial multiplier", sm.multiplier() == 1);

    sm.addKill(false); // +100
    TEST("After 1 kill", sm.score() == 100);

    // Build combo to 5 → multiplier doubles
    for (int i = 0; i < 4; i++) sm.addKill(false);
    TEST("Multiplier after 5 combo", sm.multiplier() == 2);

    sm.resetCombo();
    TEST("Multiplier after reset", sm.multiplier() == 1);

    sm.addKill(true); // boss kill
    TEST("Boss kill score > regular", sm.score() > 600);

    TEST("High score tracked", sm.highScore() == sm.score());
}

// ─────────────────────────────────────────────
// AI Tests
// ─────────────────────────────────────────────
void testAI() {
    std::cout << "\n[EnemyAI]\n";

    EnemyAI ai;
    TEST("Initial state: PATROL", ai.currentState() == AIState::PATROL);

    // Far from player → stays in PATROL
    AIContext ctx;
    ctx.playerX = 400; ctx.playerY = 400;
    ctx.selfX   = 400; ctx.selfY   = -200; // far away
    ctx.selfHp  = 3;   ctx.maxHp   = 3;
    ctx.dt      = 1.0f / 60.0f;
    ctx.level   = 1;
    ctx.timeSinceSpawn = 0;

    auto d = ai.tick(ctx);
    TEST("Patrol: no shoot when far", !d.shouldShoot);

    // Close to player → transitions to CHASE after tick
    // dist = sqrt((400-400)^2 + (350-400)^2) = 50 < 250, triggers on first tick
    ctx.selfX = 400; ctx.selfY = 350;
    ai.tick(ctx); // one tick is enough to evaluate distance
    TEST("Close → CHASE transition", ai.currentState() == AIState::CHASE);
}

// ─────────────────────────────────────────────
// Game Engine Integration Test
// ─────────────────────────────────────────────
void testEngine() {
    std::cout << "\n[GameEngine]\n";

    GameConfig cfg;
    cfg.screenW = 800; cfg.screenH = 600;
    GameEngine eng(cfg);
    eng.setDrawCallbacks(
        [](float,float,float,float,const char*){},
        [](float,float,const char*,const char*){}
    );

    eng.init();
    TEST("Initial phase: PLAYING", eng.getPhase() == GamePhase::PLAYING);
    TEST("Initial score: 0", eng.getScore() == 0);
    TEST("Initial health: 3", eng.getHealth() == 3);
    TEST("Initial level: 1", eng.getLevel() == 1);

    // Run 60 frames
    eng.inputFire = true;
    for (int i = 0; i < 60; i++) eng.update(cfg.fixedDt);
    TEST("Engine runs 60 frames without crash", true);

    // Reset
    eng.reset();
    TEST("After reset: score = 0", eng.getScore() == 0);
    TEST("After reset: health = 3", eng.getHealth() == 3);

    // Stats string for WASI
    std::string stats = eng.getStatsString();
    TEST("Stats string non-empty", !stats.empty());
    TEST("Stats contains Frame=", stats.find("Frame=") != std::string::npos);
}

// ─────────────────────────────────────────────
// main
// ─────────────────────────────────────────────
int main() {
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║  Space Defender — Unit Test Suite    ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    testPhysics();
    testScore();
    testAI();
    testEngine();

    std::cout << "\n──────────────────────────────────────\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";

    return failed > 0 ? 1 : 0;
}
