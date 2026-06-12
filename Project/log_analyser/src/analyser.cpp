/**
 * Log Analyser — C++ core compiled to WebAssembly
 *
 * The browser reads a log file from disk (File API), sends the
 * text content into this C++ WASM module for parsing, and
 * displays rich statistics entirely client-side.
 *
 * Exported functions (called from JS):
 *   analyserInit()           — reset state
 *   analyserFeedLine(ptr)    — process one log line
 *   analyserGetStats() → ptr — JSON stats string
 *   analyserFree(ptr)        — free returned string
 */

#include <emscripten/emscripten.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

// ─────────────────────────────────────────────
// Log Level Classification
// ─────────────────────────────────────────────
static const char* LEVELS[] = { "ERROR", "WARN", "WARNING", "INFO", "DEBUG", "TRACE", "FATAL", "CRITICAL" };
static const int   N_LEVELS = 8;

enum Level { L_ERROR=0, L_WARN, L_INFO, L_DEBUG, L_TRACE, L_FATAL, L_OTHER };

static Level classifyLine(const char* line) {
    if (strstr(line, "ERROR")    || strstr(line, "error"))    return L_ERROR;
    if (strstr(line, "CRITICAL") || strstr(line, "critical")) return L_FATAL;
    if (strstr(line, "FATAL")    || strstr(line, "fatal"))    return L_FATAL;
    if (strstr(line, "WARN")     || strstr(line, "warn"))     return L_WARN;
    if (strstr(line, "INFO")     || strstr(line, "info"))     return L_INFO;
    if (strstr(line, "DEBUG")    || strstr(line, "debug"))    return L_DEBUG;
    if (strstr(line, "TRACE")    || strstr(line, "trace"))    return L_TRACE;
    return L_OTHER;
}

// ─────────────────────────────────────────────
// Global Stats (WASM linear memory)
// ─────────────────────────────────────────────
static struct {
    int totalLines;
    int errors;
    int warnings;
    int info;
    int debug;
    int trace;
    int fatal;
    int other;
    int uniquePatterns;  // rough count of unique error patterns
    char lastError[256];
    char lastWarn [256];
    char firstLine[256];
    bool hasFirst;
} gStats;

// ─────────────────────────────────────────────
// Exported Functions
// ─────────────────────────────────────────────
extern "C" {

EMSCRIPTEN_KEEPALIVE
void analyserInit() {
    memset(&gStats, 0, sizeof(gStats));
}

EMSCRIPTEN_KEEPALIVE
void analyserFeedLine(const char* line) {
    if (!line || !*line) return;
    gStats.totalLines++;

    if (!gStats.hasFirst) {
        strncpy(gStats.firstLine, line, 255);
        gStats.firstLine[255] = '\0';
        gStats.hasFirst = true;
    }

    Level lv = classifyLine(line);
    switch(lv) {
        case L_ERROR: gStats.errors++;
                      strncpy(gStats.lastError, line, 255);
                      gStats.lastError[255] = '\0'; break;
        case L_WARN:  gStats.warnings++;
                      strncpy(gStats.lastWarn,  line, 255);
                      gStats.lastWarn[255]  = '\0'; break;
        case L_INFO:  gStats.info++;   break;
        case L_DEBUG: gStats.debug++;  break;
        case L_TRACE: gStats.trace++;  break;
        case L_FATAL: gStats.fatal++;  break;
        default:      gStats.other++;  break;
    }
}

EMSCRIPTEN_KEEPALIVE
const char* analyserGetStats() {
    // Build JSON manually (no sprintf format strings for safety)
    static char buf[2048];
    snprintf(buf, sizeof(buf),
        "{"
        "\"total\":%d,"
        "\"errors\":%d,"
        "\"warnings\":%d,"
        "\"info\":%d,"
        "\"debug\":%d,"
        "\"trace\":%d,"
        "\"fatal\":%d,"
        "\"other\":%d,"
        "\"health\":\"%s\","
        "\"lastError\":\"%.*s\","
        "\"lastWarn\":\"%.*s\""
        "}",
        gStats.totalLines,
        gStats.errors,
        gStats.warnings,
        gStats.info,
        gStats.debug,
        gStats.trace,
        gStats.fatal,
        gStats.other,
        (gStats.fatal > 0 || gStats.errors > 10) ? "CRITICAL" :
        (gStats.errors > 0) ? "DEGRADED" :
        (gStats.warnings > 5) ? "WARNING" : "HEALTHY",
        80, gStats.lastError,
        80, gStats.lastWarn
    );
    char* out = (char*)malloc(strlen(buf) + 1);
    strcpy(out, buf);
    return out;
}

EMSCRIPTEN_KEEPALIVE
void analyserFree(const char* ptr) { free((void*)ptr); }

EMSCRIPTEN_KEEPALIVE
int analyserGetTotal()    { return gStats.totalLines; }
EMSCRIPTEN_KEEPALIVE
int analyserGetErrors()   { return gStats.errors; }
EMSCRIPTEN_KEEPALIVE
int analyserGetWarnings() { return gStats.warnings; }
EMSCRIPTEN_KEEPALIVE
int analyserGetInfo()     { return gStats.info; }
EMSCRIPTEN_KEEPALIVE
int analyserGetFatal()    { return gStats.fatal; }

} // extern "C"
