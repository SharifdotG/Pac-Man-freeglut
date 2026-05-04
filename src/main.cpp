// Pac-Man (freeglut + C++17) — entry point.
//
// Everything interesting lives in core::run_app. Keep this file boring on
// purpose: easier to swap entry points (test harness, headless runner) later.

#include "core/app.h"

int main(int argc, char **argv) { return core::run_app(argc, argv); }
