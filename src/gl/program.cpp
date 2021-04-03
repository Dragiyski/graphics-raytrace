#include "program.h"
#include <string>
#include <vector>

void gl::program::destroy(GLuint program) {
    if (!glIsProgram(program)) {
        return;
    }
    GLint shaderCount;
    glGetProgramiv(program, GL_ATTACHED_SHADERS, &shaderCount);
    std::vector<GLuint> shaders;
    shaders.reserve(shaderCount);
    shaders.resize(shaderCount);
    glGetAttachedShaders(program, shaderCount, nullptr, shaders.data());
    for(const auto &shader : shaders) {
        glDeleteShader(shader);
    }
    glDeleteProgram(program);
}

gl::program::link_error::link_error(const char *message) : runtime_error(message) {}
