/**
 * Space Defender — WASI Entry Point
 * Compiled with: clang++ --target=wasm32-wasi
 * Run with:      wasmtime --dir data::data wasi_defender.wasm -- --frames 300
 *
 * Demonstrates:
 *  - Same C++ engine running headlessly (no graphics)
 *  - WASI filesystem capability grants (--dir flag)
 *  - WASI environment variable access (--env flag)
 *  - Saving scores to a file
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include "../include/engine.h"

// ─────────────────────────────────────────────
// Null drawing callbacks for headless mode
// ─────────────────────────────────────────────
static void noopDraw(float, float, float, float, const char*) {}
static void noopText(float, float, const char*, const char*)  {}

// ─────────────────────────────────────────────
// Parse CLI arguments
// ─────────────────────────────────────────────
struct WasiArgs {
    int         frames     = 600;
    std::string outputFile = "data/scores.txt";
    int         seed       = 42;
    bool        verbose    = false;
};

static WasiArgs parseArgs(int argc, char** argv) {
    WasiArgs a;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--frames") == 0 && i+1 < argc)
            a.frames = atoi(argv[++i]);
        else if (strcmp(argv[i], "--output") == 0 && i+1 < argc)
            a.outputFile = argv[++i];
        else if (strcmp(argv[i], "--seed") == 0 && i+1 < argc)
            a.seed = atoi(argv[++i]);
        else if (strcmp(argv[i], "--verbose") == 0)
            a.verbose = true;
    }
    return a;
}

// ─────────────────────────────────────────────
// Simulate AI-driven input (no human)
// ─────────────────────────────────────────────
static void simulateInput(GameEngine& eng, int frame) {
    // Auto-play: always fire, move in a sine pattern
    eng.inputFire  = true;
    eng.inputLeft  = (frame % 120) < 60;
    eng.inputRight = (frame % 120) >= 60;
    eng.inputUp    = false;
    eng.inputDown  = false;
}

// ─────────────────────────────────────────────
// WASI main()
// ─────────────────────────────────────────────
int main(int argc, char** argv) {
    WasiArgs args = parseArgs(argc, argv);

    // Read env variable (host must pass via --env PILOT=name)
    const char* pilot = getenv("PILOT");
    if (!pilot) pilot = "AutoPilot";

    printf("╔══════════════════════════════════════╗\n");
    printf("║     Space Defender — WASI Runtime    ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║ Pilot  : %-29s║\n", pilot);
    printf("║ Frames : %-29d║\n", args.frames);
    printf("║ Output : %-29s║\n", args.outputFile.c_str());
    printf("║ Seed   : %-29d║\n", args.seed);
    printf("╚══════════════════════════════════════╝\n\n");

    srand(args.seed);

    GameConfig cfg;
    cfg.screenW = 800;
    cfg.screenH = 600;

    GameEngine engine(cfg);
    engine.setDrawCallbacks(noopDraw, noopText);
    engine.init();

    // ─── Simulation Loop ───────────────────────
    printf("Simulating %d frames...\n\n", args.frames);
    printf("%-8s %-8s %-8s %-6s %-6s\n", "Frame", "Score", "Health", "Level", "Phase");
    printf("%-8s %-8s %-8s %-6s %-6s\n", "------", "------", "------", "-----", "------");

    int logInterval = args.frames / 20;
    if (logInterval < 1) logInterval = 1;

    for (int f = 0; f < args.frames; f++) {
        simulateInput(engine, f);
        engine.update(cfg.fixedDt);

        if (args.verbose || f % logInterval == 0) {
            printf("%-8d %-8d %-8d %-6d %-6d\n",
                   f, engine.getScore(), engine.getHealth(),
                   engine.getLevel(), (int)engine.getPhase());
        }

        if (engine.isOver()) {
            printf("\n[GAME OVER at frame %d]\n", f);
            break;
        }
    }

    // ─── Save Score to File (WASI filesystem access) ───
    printf("\n[WASI] Attempting to write score to: %s\n", args.outputFile.c_str());

    std::ofstream out(args.outputFile);
    if (out.is_open()) {
        out << "=== Space Defender — WASI Score Report ===\n";
        out << "Pilot    : " << pilot << "\n";
        out << "Frames   : " << args.frames << "\n";
        out << "Seed     : " << args.seed   << "\n";
        out << "Score    : " << engine.getScore()  << "\n";
        out << "Health   : " << engine.getHealth() << "\n";
        out << "Level    : " << engine.getLevel()  << "\n";
        out << "Status   : " << (engine.isOver() ? "GAME OVER" : "SURVIVED") << "\n";
        out << "Stats    : " << engine.getStatsString() << "\n";
        out.close();
        printf("[WASI] ✅ Score saved successfully.\n");
        printf("[WASI]    This required --dir capability grant from Wasmtime.\n");
        printf("[WASI]    Without --dir, this would fail with 'permission denied'.\n");
    } else {
        printf("[WASI] ❌ Could not open file (missing --dir flag in wasmtime command?)\n");
        printf("[WASI]    Try: wasmtime --dir data::data wasi_defender.wasm\n");
    }

    // ─── Final Report ──────────────────────────
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║         Final Statistics             ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║ Final Score  : %-22d║\n", engine.getScore());
    printf("║ Final Health : %-22d║\n", engine.getHealth());
    printf("║ Level Reached: %-22d║\n", engine.getLevel());
    printf("║ Result       : %-22s║\n", engine.isOver() ? "GAME OVER" : "SURVIVED");
    printf("╚══════════════════════════════════════╝\n");

    return engine.isOver() ? 1 : 0;
}
