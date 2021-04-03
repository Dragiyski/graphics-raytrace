#ifndef RAYTRACE_GL_MAIN_H
#define RAYTRACE_GL_MAIN_H

#include <map>
#include <string>
#include <SDL2/SDL_video.h>
#include <GL/gl.h>

namespace gl {
    namespace {
        template<typename R, typename ... Args>
        struct FunctionTraits {
            using pointer_type =

            R(GLAPIENTRY *)(Args...);
        };
    }

    template<typename R, typename ... Args>
    typename FunctionTraits<R, Args...>::pointer_type SDL_GL_Function(const char *name) {
        return reinterpret_cast<typename FunctionTraits<R, Args...>::pointer_type>(SDL_GL_GetProcAddress(name));
    }

    typedef std::map<std::string, GLuint> resource_map;
    typedef std::map<std::string, GLsizei> size_map;
}

#endif //RAYTRACE_GL_MAIN_H
