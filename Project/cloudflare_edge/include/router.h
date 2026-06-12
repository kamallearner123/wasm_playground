#pragma once
#include <string>
#include <functional>
#include <vector>

// ─────────────────────────────────────────────
// Simple URL Router
// ─────────────────────────────────────────────
struct Request {
    std::string method;  // GET, POST, etc.
    std::string path;    // e.g. "/api/hello"
    std::string query;   // e.g. "value=hello"
    std::string body;    // request body (POST)
    std::string clientIP;
};

struct Response {
    int         status  = 200;
    std::string body;
    std::string contentType = "application/json";
};

using HandlerFn = std::function<Response(const Request&)>;

struct Route {
    std::string method;
    std::string path;
    HandlerFn   handler;
};

class Router {
public:
    void add(const std::string& method, const std::string& path, HandlerFn h) {
        routes_.push_back({method, path, h});
    }

    Response dispatch(const Request& req) const {
        for (const auto& r : routes_) {
            if (r.method == req.method && r.path == req.path)
                return r.handler(req);
        }
        // 404
        return {404, "{\"error\":\"Not Found\",\"path\":\"" + req.path + "\"}"};
    }

private:
    std::vector<Route> routes_;
};
