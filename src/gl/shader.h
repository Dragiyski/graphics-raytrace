#ifndef RAYTRACE_SHADER_H
#define RAYTRACE_SHADER_H

#include <stdexcept>
#include <GL/gl.h>

namespace gl::shader {
        GLuint fromFile(GLenum type, const char *filename);
        GLuint fromSource(GLenum type, const char *source);

        class parse_error : public std::runtime_error {
        public:
            explicit parse_error(const char *);
            ~parse_error() override = default;
        };
    }


#endif //RAYTRACE_SHADER_H
