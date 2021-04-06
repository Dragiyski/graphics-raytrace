#ifndef RAYTRACE_GLOBAL_H
#define RAYTRACE_GLOBAL_H

#include <stdexcept>
#include <filesystem>

namespace dragiyski::raytrace {
    class sdl_error : public std::runtime_error {
    public:
        explicit sdl_error(const char *message);
        ~sdl_error() override = default;
    };
}

namespace std::filesystem {
    path resolve(const path &input, const path &base);
}

#endif //RAYTRACE_GLOBAL_H
