#ifndef RAYTRACE_PROGRAM_H
#define RAYTRACE_PROGRAM_H

#include <array>
#include <stdexcept>
#include <tuple>
#include <GL/gl.h>

namespace gl::program {
    void destroy(GLuint program);

    class link_error : public std::runtime_error {
    public:
        explicit link_error(const char *);

        ~link_error() override = default;
    };


    template<typename ... Shaders>
    GLuint create(Shaders ...shaders) {
        auto program = glCreateProgram();
        (glAttachShader(program, shaders), ...);
        glLinkProgram(program);
        {
            GLint status;
            glGetProgramiv(program, GL_LINK_STATUS, &status);
            if (!status) {
                GLint length;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
                std::string log;
                log.reserve(length);
                glGetProgramInfoLog(program, length, nullptr, log.data());
                throw link_error(log.c_str());
            }
        }
        return program;
    }
}

#endif //RAYTRACE_PROGRAM_H
