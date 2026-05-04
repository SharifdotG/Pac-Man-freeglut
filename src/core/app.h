#pragma once

namespace core {

// Single entry point invoked from main.cpp. Owns the GLUT init dance, the
// fixed-step Clock, and the freeglut callbacks. Returns the process exit
// code.
int run_app(int argc, char **argv);

} // namespace core
