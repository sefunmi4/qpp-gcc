#pragma once

#include <string>

// Compiles the given .qpp source file and executes the resulting binary.
// Returns the exit code of the executed program, or a non-zero value on
// failure.
int compile_and_run(const std::string& source_path);

