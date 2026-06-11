#pragma once
#include <cstdint>
#include "physics.h"

// ─────────────────────────────────────────────
// Enemy AI States
// ─────────────────────────────────────────────
enum class AIState : uint8_t {
    PATROL,   // Move in sine wave pattern
    CHASE,    // Rush toward player
    ATTACK,   // Hover and shoot repeatedly
    RETREAT,  // Move away when HP is low (boss only)
    DEAD
};

struct AIContext {
    float playerX, playerY;     // Player position
    float selfX,   selfY;       // Enemy position
    int   selfHp,  maxHp;
    float dt;
    int   level;
    float timeSinceSpawn;
};

struct AIDecision {
    float moveX = 0, moveY = 0;  // Normalized direction [-1, 1]
    bool  shouldShoot = false;
};

// ─────────────────────────────────────────────
// EnemyAI — Finite State Machine
// ─────────────────────────────────────────────
class EnemyAI {
public:
    explicit EnemyAI(bool isBoss = false);

    // Tick FSM, returns a decision for this frame
    AIDecision tick(const AIContext& ctx);

    AIState currentState() const { return state_; }

private:
    AIDecision patrolTick (const AIContext& ctx);
    AIDecision chaseTick  (const AIContext& ctx);
    AIDecision attackTick (const AIContext& ctx);
    AIDecision retreatTick(const AIContext& ctx);

    AIState state_      = AIState::PATROL;
    bool    isBoss_     = false;
    float   stateTimer_ = 0.0f;
    float   sinePhase_  = 0.0f;
    float   shootCooldown_ = 0.0f;
    int     attackPhase_ = 0; // Boss multi-phase
};
