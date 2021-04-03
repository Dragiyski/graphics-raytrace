#include "Application.h"
#include <fstream>
#include <iostream>
#include <functional>
#include <type_traits>
#include <utility>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "gl/program.h"
#include "gl/shader.h"
#include "gl/main.h"

namespace std::filesystem {
    path resolve(const path &input, const path &base) {
        if (input.is_absolute()) {
            return input;
        }
        path work = base.is_absolute() ? base : absolute(base);
        if (!is_directory(work)) {
            work = work.parent_path();
        }
        return weakly_canonical(work / input);
    }
}

namespace {
    int getBestDisplay() {
        int displayCount = SDL_GetNumVideoDisplays();
        if (displayCount < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (displayCount < 1) {
            throw std::runtime_error("No display found");
        }
        int selected = 0;
        int resolution = 0;
        for (int i = 0; i < displayCount; ++i) {
            SDL_DisplayMode mode;
            if (SDL_GetCurrentDisplayMode(i, &mode) < 0) {
                throw std::runtime_error(SDL_GetError());
            }
            auto res = mode.w * mode.h;
            if (res > resolution) {
                selected = i;
                resolution = res;
            }
        }
        return selected;
    }

    void printOpenGLInfo() {
        std::cout << "gl.vendor = " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "gl.version = " << glGetString(GL_VERSION) << std::endl;
        std::cout << "gl.shader_version = " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "gl.renderer = " << glGetString(GL_RENDERER) << std::endl;
    }

    void GLAPIENTRY displayErrors(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar *message,
        const void *userParam
    ) {
        fprintf(
            stderr, "[gl][%d][%d][%d]%s: %s\n",
            id,
            source,
            severity,
            (GL_DEBUG_TYPE_ERROR ? "error" : ""),
            message
        );
    }

    class finally {
    private:
        std::function<void()> m_function;
    public:
        explicit finally(std::function<void()> function) : m_function(std::move(function)) {}

        ~finally() {
            m_function();
        }
    };
}

sdl_error::sdl_error(const char *message) : std::runtime_error(message) {}

Application::Application() = default;

Application::~Application() {
    if (m_resources.contains("gl.context")) {
        auto context = std::any_cast<SDL_GLContext>(m_resources["gl.context"]);
        m_resources.erase("gl.context");
        if (context != nullptr) {
            SDL_GL_DeleteContext(context);
        }
    }
    if (m_resources.contains("window")) {
        auto window = std::any_cast<SDL_Window *>(m_resources["window"]);
        m_resources.erase("window");
        if (window != nullptr) {
            SDL_DestroyWindow(window);
        }
    }
}

void Application::run() {
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    int display = getBestDisplay();
    SDL_Rect screenBounds;
    if (SDL_GetDisplayUsableBounds(display, &screenBounds) < 0) {
        throw sdl_error(SDL_GetError());
    }
    SDL_Rect windowBounds;
    windowBounds.w = screenBounds.w / 2;
    windowBounds.h = screenBounds.h / 2;
    windowBounds.w = std::min(windowBounds.w, screenBounds.w);
    windowBounds.h = std::min(windowBounds.h, screenBounds.h);
    windowBounds.x = screenBounds.x + (screenBounds.w - windowBounds.w) / 2;
    windowBounds.y = screenBounds.y + (screenBounds.h - windowBounds.h) / 2;

    auto window = SDL_CreateWindow(
        "Raytrace",
        windowBounds.x,
        windowBounds.y,
        windowBounds.w,
        windowBounds.h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if (window == nullptr) {
        throw sdl_error(SDL_GetError());
    }
    m_resources["window"] = window;

    auto context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        throw sdl_error(SDL_GetError());
    }
    m_resources["gl.context"] = context;

    SDL_GL_MakeCurrent(window, context);
    initializeGL();
    finally on_app_exit([&]() { releaseGL(); });

    SDL_Event event;
    while (true) {
        if (SDL_WaitEvent(&event) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (event.type == SDL_QUIT) {
            break;
        }
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
                paintGL();
            }
        }
    }
}

void Application::initializeGL() {
    std::filesystem::path projectDir(PROJECT_SOURCE_DIR);
    SDL_GL_SetSwapInterval(1);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(displayErrors, this);
    printOpenGLInfo();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    m_resources["gl.program"] = gl::resource_map();
    m_resources["gl.vertexBuffer"] = gl::resource_map();
    m_resources["gl.vertexArray"] = gl::resource_map();
    m_resources["gl.vertexArraySize"] = gl::size_map();

    {
        auto &programs = std::any_cast<gl::resource_map &>(m_resources["gl.program"]);
        programs["main"] = gl::program::create(
            gl::shader::fromFile(GL_VERTEX_SHADER, std::filesystem::resolve("var/vertex.glsl", projectDir).c_str()),
            gl::shader::fromFile(GL_FRAGMENT_SHADER, std::filesystem::resolve("var/fragment.glsl", projectDir).c_str())
        );
    }
    {
        auto &bufferStore = std::any_cast<gl::resource_map &>(m_resources["gl.vertexBuffer"]);
        auto &arrayStore = std::any_cast<gl::resource_map &>(m_resources["gl.vertexArray"]);
        auto &arraySizeStore = std::any_cast<gl::size_map &>(m_resources["gl.vertexArraySize"]);
        GLuint gl_buffer[2];
        GLuint gl_array;
        glGenBuffers(2, gl_buffer);
        glGenVertexArrays(1, &gl_array);

        GLfloat vertexData[] = {
            -1.0, -1.0,
            +1.0, -1.0,
            +1.0, +1.0,
            -1.0, +1.0
        };

        GLubyte indexData[] = {
            0, 1, 2,
            0, 2, 3
        };

        glBindVertexArray(gl_array);

        glBindBuffer(GL_ARRAY_BUFFER, gl_buffer[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_buffer[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<const void *>(0));

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        bufferStore["screen.vertex_buffer"] = gl_buffer[0];
        bufferStore["screen.index_buffer"] = gl_buffer[1];
        arrayStore["screen"] = gl_array;
        arraySizeStore["screen"] = static_cast<GLsizei>(sizeof(indexData) / sizeof(std::remove_all_extents<decltype(indexData)>::type));
    }
}

void Application::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    auto program = std::any_cast<gl::resource_map &>(m_resources["gl.program"])["main"];
    auto array = std::any_cast<gl::resource_map &>(m_resources["gl.vertexArray"])["screen"];
    auto size = std::any_cast<gl::size_map &>(m_resources["gl.vertexArraySize"])["screen"];

    glUseProgram(program);
    glBindVertexArray(array);
    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(0));
    glBindVertexArray(0);
    glUseProgram(0);

    if (m_resources.contains("window")) {
        SDL_GL_SwapWindow(std::any_cast<SDL_Window *>(m_resources["window"]));
    }
}

void Application::releaseGL() {
    SDL_GL_MakeCurrent(
        std::any_cast<SDL_Window *>(m_resources["window"]),
        std::any_cast<SDL_GLContext>(m_resources["gl.context"])
    );
    if (m_resources.contains("gl.program")) {
        auto &programs = std::any_cast<gl::resource_map &>(m_resources["gl.program"]);
        for (const auto &program : programs) {
            gl::program::destroy(program.second);
        }
        m_resources.erase("gl.program");
    }
    if (m_resources.contains("gl.vertexArray")) {
        auto &gl_arrays = std::any_cast<gl::resource_map &>(m_resources["gl.vertexArray"]);
        for (const auto &gl_array : gl_arrays) {
            glDeleteVertexArrays(1, &gl_array.second);
        }
        m_resources.erase("gl.vertexArray");
    }
    if (m_resources.contains("gl.vertexBuffer")) {
        auto &gl_buffers = std::any_cast<gl::resource_map &>(m_resources["gl.vertexBuffer"]);
        for (const auto &gl_buffer : gl_buffers) {
            glDeleteVertexArrays(1, &gl_buffer.second);
        }
        m_resources.erase("gl.vertexBuffer");
    }
}