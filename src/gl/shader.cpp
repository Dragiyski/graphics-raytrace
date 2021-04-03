#include "shader.h"
#include <fstream>
#include <streambuf>
#include <string>

GLuint gl::shader::fromFile(GLenum type, const char *filename) {
    std::ifstream stream(filename);
    std::string source((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return fromSource(type, source.c_str());
}

GLuint gl::shader::fromSource(GLenum type, const char *source) {
    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    {
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status) {
            GLint length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            std::string log;
            log.reserve(length);
            glGetShaderInfoLog(shader, length, nullptr, log.data());
            throw parse_error(log.c_str());
        }
    }
    return shader;
}


gl::shader::parse_error::parse_error(const char *message) : runtime_error(message) {}
