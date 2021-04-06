#include "global.h"

dragiyski::raytrace::sdl_error::sdl_error(const char *message) : std::runtime_error(message) {}

std::filesystem::path std::filesystem::resolve(const path &input, const path &base) {
    if (input.is_absolute()) {
        return input;
    }
    path work = base.is_absolute() ? base : absolute(base);
    if (!is_directory(work)) {
        work = work.parent_path();
    }
    return weakly_canonical(work / input);
}