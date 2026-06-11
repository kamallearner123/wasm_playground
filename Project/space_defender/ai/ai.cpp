#include "ai.h"
#include <cmath>

EnemyAI::EnemyAI(bool isBoss) : isBoss_(isBoss) {
    state_ = AIState::PATROL;
}

AIDecision EnemyAI::tick(const AIContext& ctx) {
    stateTimer_    += ctx.dt;
    shootCooldown_ -= ctx.dt;

    switch (state_) {
        case AIState::PATROL:  return patrolTick(ctx);
        case AIState::CHASE:   return chaseTick(ctx);
        case AIState::ATTACK:  return attackTick(ctx);
        case AIState::RETREAT: return retreatTick(ctx);
        default: return AIDecision{};
    }
}

AIDecision EnemyAI::patrolTick(const AIContext& ctx) {
    sinePhase_ += ctx.dt * 2.0f;
    AIDecision d;
    d.moveX = std::sin(sinePhase_) * 0.5f;
    d.moveY = 0.3f;

    float dist = std::sqrt((ctx.selfX - ctx.playerX) * (ctx.selfX - ctx.playerX) +
                           (ctx.selfY - ctx.playerY) * (ctx.selfY - ctx.playerY));
    if (dist < 250.0f) {
        state_      = AIState::CHASE;
        stateTimer_ = 0;
    }
    return d;
}

AIDecision EnemyAI::chaseTick(const AIContext& ctx) {
    AIDecision d;
    float dx = ctx.playerX - ctx.selfX;
    float dy = ctx.playerY - ctx.selfY;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.01f) { d.moveX = dx / len; d.moveY = dy / len; }

    // Transition to attack when close
    if (len < 120.0f) {
        state_      = AIState::ATTACK;
        stateTimer_ = 0;
    }
    // Give up chase after 3s if far
    if (stateTimer_ > 3.0f && len > 300.0f) {
        state_      = AIState::PATROL;
        stateTimer_ = 0;
    }
    return d;
}

AIDecision EnemyAI::attackTick(const AIContext& ctx) {
    AIDecision d;
    d.moveX = 0; d.moveY = 0.1f; // hover slowly
    if (shootCooldown_ <= 0) {
        d.shouldShoot  = true;
        shootCooldown_ = isBoss_ ? 0.4f : 1.0f;
    }

    // Retreat if HP critical (boss only)
    if (isBoss_ && ctx.selfHp < ctx.maxHp / 3) {
        state_      = AIState::RETREAT;
        stateTimer_ = 0;
    }
    // Return to patrol after 4s
    if (stateTimer_ > 4.0f) {
        state_      = AIState::PATROL;
        stateTimer_ = 0;
    }
    return d;
}

AIDecision EnemyAI::retreatTick(const AIContext& ctx) {
    AIDecision d;
    float dx = ctx.selfX - ctx.playerX;
    float dy = ctx.selfY - ctx.playerY;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.01f) { d.moveX = dx / len * 1.5f; d.moveY = dy / len * 0.5f; }

    if (shootCooldown_ <= 0) { // Shoot while retreating
        d.shouldShoot  = true;
        shootCooldown_ = 0.6f;
    }
    if (stateTimer_ > 2.0f) {
        state_      = AIState::ATTACK;
        stateTimer_ = 0;
        attackPhase_++;
    }
    return d;
}
