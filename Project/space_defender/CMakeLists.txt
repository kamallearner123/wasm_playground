cmake_minimum_required(VERSION 3.20)
project(SpaceDefender CXX)
set(CMAKE_CXX_STANDARD 20)

# ─────────────────────────────────────────────
# Common source files (platform-agnostic engine)
# ─────────────────────────────────────────────
set(ENGINE_SOURCES
    engine/engine.cpp
    ai/ai.cpp
)

set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
)

# ─────────────────────────────────────────────
# Detect build target
# ─────────────────────────────────────────────
if(DEFINED EMSCRIPTEN)
    # ── Browser / WASM Build ──────────────────
    message(STATUS "Building for Browser (Emscripten/WASM)")

    add_executable(game
        browser/main_browser.cpp
        ${ENGINE_SOURCES}
    )

    target_include_directories(game PRIVATE ${INCLUDE_DIRS})

    set_target_properties(game PROPERTIES
        OUTPUT_NAME "game"
        SUFFIX ".js"
    )

    target_link_options(game PRIVATE
        "-sWASM=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sEXPORTED_FUNCTIONS=['_main','_getScore','_getHealth','_getLevel','_getPhase','_resetGame','_setFireInput']"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString']"
        "-sUSE_SDL=0"
        "--shell-file=${CMAKE_SOURCE_DIR}/browser/index.html"
        "-O2"
    )

elseif(CMAKE_SYSTEM_NAME STREQUAL "WASI" OR WASI_BUILD)
    # ── WASI Build ────────────────────────────
    message(STATUS "Building for WASI runtime")

    add_executable(wasi_defender
        wasi/main_wasi.cpp
        ${ENGINE_SOURCES}
    )

    target_include_directories(wasi_defender PRIVATE ${INCLUDE_DIRS})
    target_compile_options(wasi_defender PRIVATE -O2)

else()
    # ── Native Build (for tests) ──────────────
    message(STATUS "Building native (tests)")

    add_executable(test_engine
        tests/test_engine.cpp
        ${ENGINE_SOURCES}
    )
    target_include_directories(test_engine PRIVATE ${INCLUDE_DIRS})
endif()
