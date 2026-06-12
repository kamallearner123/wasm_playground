#pragma once
#include <string>
#include <unordered_map>
#include <ctime>

// ─────────────────────────────────────────────
// Token Bucket Rate Limiter (per client IP)
// ─────────────────────────────────────────────
struct Bucket {
    int   tokens;
    float lastRefill; // seconds since epoch (approximated via frame counter)
};

class RateLimiter {
public:
    // maxTokens: max requests per window
    // refillRate: tokens restored per second
    RateLimiter(int maxTokens = 10, float refillRate = 1.0f)
        : maxTokens_(maxTokens), refillRate_(refillRate) {}

    // Returns true if request is allowed, false if rate-limited
    bool allow(const std::string& clientIP, float nowSeconds) {
        auto& b = buckets_[clientIP];
        if (b.tokens == 0 && b.lastRefill == 0) {
            // First request for this IP
            b.tokens     = maxTokens_;
            b.lastRefill = nowSeconds;
        }
        // Refill tokens based on elapsed time
        float elapsed = nowSeconds - b.lastRefill;
        int   refill  = static_cast<int>(elapsed * refillRate_);
        if (refill > 0) {
            b.tokens     = std::min(maxTokens_, b.tokens + refill);
            b.lastRefill = nowSeconds;
        }
        if (b.tokens > 0) {
            b.tokens--;
            return true;
        }
        return false;
    }

    int remaining(const std::string& clientIP) const {
        auto it = buckets_.find(clientIP);
        return (it != buckets_.end()) ? it->second.tokens : maxTokens_;
    }

    int maxTokens() const { return maxTokens_; }

private:
    int   maxTokens_;
    float refillRate_;
    std::unordered_map<std::string, Bucket> buckets_;
};
