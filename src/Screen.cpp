#include "Screen.h"
#include "global.h"
#include <GL/glx.h>
#include <cstdio>
#include "literal.h"
#include "gl/program.h"
#include "gl/shader.h"

namespace dragiyski::raytrace {
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

        const char *glGetDebugType(GLenum value) {
            using literal::operator ""_string;
            switch (value) {
#define RAYTRACE_GL_DEBUG_ENUM(gl_enum) \
            case gl_enum: return #gl_enum##_string.after<"GL_DEBUG_TYPE_"_string.length()>();
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_TYPE_ERROR);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_TYPE_PORTABILITY);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_TYPE_PERFORMANCE);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_TYPE_OTHER);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_TYPE_MARKER);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_TYPE_PUSH_GROUP);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_TYPE_POP_GROUP);
#undef RAYTRACE_GL_DEBUG_ENUM
                default: return "";
            }
        }

        const char *glGetDebugSource(GLenum value) {
            using literal::operator ""_string;
            constexpr auto prefix_length = "GL_DEBUG_SOURCE_"_string.length();
            switch (value) {
#define RAYTRACE_GL_DEBUG_ENUM(gl_enum) \
            case gl_enum: return #gl_enum##_string.after<"GL_DEBUG_SOURCE_"_string.length()>();
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SOURCE_API);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SOURCE_WINDOW_SYSTEM);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SOURCE_SHADER_COMPILER);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SOURCE_THIRD_PARTY);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SOURCE_APPLICATION);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SOURCE_OTHER);
#undef RAYTRACE_GL_DEBUG_ENUM
                default: return "";
            }
        }

        const char *glGetDebugSeverity(GLenum value) {
            using literal::operator ""_string;
            constexpr auto prefix_length = "GL_DEBUG_SEVERITY_"_string.length();
            switch (value) {
#define RAYTRACE_GL_DEBUG_ENUM(gl_enum) \
            case gl_enum: return #gl_enum##_string.after<"GL_DEBUG_SEVERITY_"_string.length()>();
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SEVERITY_HIGH);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SEVERITY_MEDIUM);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SEVERITY_LOW);
                RAYTRACE_GL_DEBUG_ENUM(GL_DEBUG_SEVERITY_NOTIFICATION);
#undef RAYTRACE_GL_DEBUG_ENUM
                default: return "";
            }
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
                stderr, "[gl][%d][%s][%s][%s]: %.*s\n",
                id,
                glGetDebugSource(source),
                glGetDebugSeverity(severity),
                glGetDebugType(type),
                length,
                message
            );
        }

        bool is_gl_debug_callback_installed = false;

        void ensureDebugMessageCallback() {
            if (is_gl_debug_callback_installed) {
                return;
            }
            is_gl_debug_callback_installed = true;
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(displayErrors, nullptr);
        }

        std::filesystem::path projectDir(PROJECT_SOURCE_DIR);
    }

    std::map<uint32_t, std::shared_ptr<Screen>> Screen::window_screen_map;

    std::shared_ptr<Screen> Screen::New(const char *title) {
        if (!SDL_WasInit(SDL_INIT_VIDEO)) {
            if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
                throw sdl_error(SDL_GetError());
            }
        }
        auto display = getBestDisplay();
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

        if (SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0) < 0) {
            throw sdl_error(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1) < 0) {
            throw sdl_error(SDL_GetError());
        }

        auto window = SDL_CreateWindow(
            title,
            windowBounds.x,
            windowBounds.y,
            windowBounds.w,
            windowBounds.h,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
        );
        if (window == nullptr) {
            throw sdl_error(SDL_GetError());
        }
        auto context = SDL_GL_CreateContext(window);
        if (context == nullptr) {
            SDL_DestroyWindow(window);
            throw sdl_error(SDL_GetError());
        }
        std::shared_ptr<Screen> ptr(new Screen(window, context));
        window_screen_map[SDL_GetWindowID(window)] = ptr;
        return ptr;
    }

    Screen::Screen(SDL_Window *window, SDL_GLContext context) // NOLINT(cppcoreguidelines-pro-type-member-init)
        : m_window(window), m_context(context), m_is_initialized(false), m_need_resize(true) {
    }

    void Screen::notify(const SDL_Event &event) {
        if (event.type == SDL_WINDOWEVENT) {
            auto it = window_screen_map.find(event.window.windowID);
            if (it != window_screen_map.end()) {
                it->second->notifyWindow(event);
            }
        }
    }

    void Screen::notifyWindow(const SDL_Event &event) {
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
                update();
            } else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                m_need_resize = true;
                update();
            }
        }
    }

    void Screen::update() {
        SDL_GL_MakeCurrent(m_window, m_context);
        if (!m_is_initialized) {
            initialize();
        }
        if (m_need_resize) {
            resize();
        }
        paint();
    }

    void Screen::initialize() {
        ensureDebugMessageCallback();
        if (SDL_GL_SetSwapInterval(1) < 0) {
            throw sdl_error(SDL_GetError());
        }
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

        g_program_present = gl::program::create(
            gl::shader::fromFile(
                GL_VERTEX_SHADER,
                std::filesystem::resolve("var/present/vertex.glsl", projectDir).c_str()
            ),
            gl::shader::fromFile(
                GL_FRAGMENT_SHADER,
                std::filesystem::resolve("var/present/fragment.glsl", projectDir).c_str()
            )
        );

        g_program_clear = gl::program::create(
            gl::shader::fromFile(
                GL_COMPUTE_SHADER,
                std::filesystem::resolve("var/raytrace/clear.glsl", projectDir).c_str()
            )
        );

        g_program_screen = gl::program::create(
            gl::shader::fromFile(
                GL_COMPUTE_SHADER,
                std::filesystem::resolve("var/raytrace/screen.glsl", projectDir).c_str()
            )
        );

        g_program_raytrace_sphere = gl::program::create(
            gl::shader::fromFile(
                GL_COMPUTE_SHADER,
                std::filesystem::resolve("var/raytrace/shape/sphere.glsl", projectDir).c_str()
            )
        );

        g_program_light_point = gl::program::create(
            gl::shader::fromFile(
                GL_COMPUTE_SHADER,
                std::filesystem::resolve("var/raytrace/light.glsl", projectDir).c_str()
            )
        );

        glGenBuffers(1, &g_buffer_vertex_screen);
        glGenBuffers(1, &g_buffer_index_screen);
        glGenVertexArrays(1, &g_array_screen);

        glBindVertexArray(g_array_screen);

        glBindBuffer(GL_ARRAY_BUFFER, g_buffer_vertex_screen);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_index_screen);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), reinterpret_cast<const void *>(0));

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &g_texture_ray);
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &g_texture_trace);
        glCreateTextures(GL_TEXTURE_RECTANGLE, 1, &g_texture_trace_index);
        glCreateTextures(GL_TEXTURE_RECTANGLE, 1, &g_texture_screen);

        glCreateQueries(GL_TIME_ELAPSED, 1, &g_query_time_measure);

        glClearColor(0.0, 0.0, 0.0, 1.0);
    }

    void Screen::resize() {
        int width, height;
        SDL_GL_GetDrawableSize(m_window, &width, &height);

        glViewport(0, 0, width, height);
        glBindTexture(GL_TEXTURE_2D_ARRAY, g_texture_ray);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, width, height, 2, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D_ARRAY, g_texture_trace);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, width, height, 4, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        glBindTexture(GL_TEXTURE_RECTANGLE, g_texture_trace_index);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
        glBindTexture(GL_TEXTURE_RECTANGLE, g_texture_screen);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);
    }

    void Screen::paint() {
        glClear(GL_COLOR_BUFFER_BIT);

        bool is_running = false;
        int width, height;
        SDL_GL_GetDrawableSize(m_window, &width, &height);

        int minSize = std::min(width, height);
        if (minSize > 0) {
            is_running = true;
            glBeginQuery(GL_TIME_ELAPSED, g_query_time_measure);

            {
                glUseProgram(g_program_clear);
                glBindImageTexture(
                    0,
                    g_texture_trace,
                    0,
                    GL_TRUE,
                    0,
                    GL_WRITE_ONLY,
                    GL_RGBA32F
                );
                glBindImageTexture(
                    1,
                    g_texture_screen,
                    0,
                    GL_TRUE,
                    0,
                    GL_WRITE_ONLY,
                    GL_RGBA32F
                );
                glBindImageTexture(
                    2,
                    g_texture_trace_index,
                    0,
                    GL_TRUE,
                    0,
                    GL_WRITE_ONLY,
                    GL_R32UI
                );
                glDispatchCompute(width, height, 1);
            }
            {
                float fieldOfView = 30.0 / 180.0 * std::acos(-1);
                float viewSize[2] = {float(width) / float(minSize), float(height) / float(minSize)};
                float viewLength = std::sqrt(viewSize[0] * viewSize[0] + viewSize[1] * viewSize[1]);
                float screenRadius = viewLength / std::tan(fieldOfView * 0.5);
                glUseProgram(g_program_screen);
                glUniform2i(glGetUniformLocation(g_program_screen, "screenSize"), width, height);
                glUniform2fv(glGetUniformLocation(g_program_screen, "viewSize"), 1, viewSize);
                glUniform1f(glGetUniformLocation(g_program_screen, "screenRadius"), screenRadius);
                glBindImageTexture(
                    0,
                    g_texture_ray,
                    0,
                    GL_TRUE,
                    0,
                    GL_READ_WRITE,
                    GL_RGBA32F
                );
                glDispatchCompute(width, height, 1);

                glUseProgram(g_program_raytrace_sphere);
                glUniform1ui(glGetUniformLocation(g_program_raytrace_sphere, "id"), 1u);
                glUniform3f(glGetUniformLocation(g_program_raytrace_sphere, "position"), 0.0, 0.0, -6.0);
                glUniform1f(glGetUniformLocation(g_program_raytrace_sphere, "radius"), 1.5);
                glUniform3f(glGetUniformLocation(g_program_raytrace_sphere, "color"), 1.0, 0.0, 0.0);
                glBindImageTexture(
                    glGetUniformLocation(g_program_raytrace_sphere, "image_ray"),
                    g_texture_ray,
                    0,
                    GL_TRUE,
                    0,
                    GL_READ_ONLY,
                    GL_RGBA32F
                );
                glBindImageTexture(
                    glGetUniformLocation(g_program_raytrace_sphere, "image_trace"),
                    g_texture_trace,
                    0,
                    GL_TRUE,
                    0,
                    GL_WRITE_ONLY,
                    GL_RGBA32F
                );
                glBindImageTexture(
                    glGetUniformLocation(g_program_raytrace_sphere, "image_trace_index"),
                    g_texture_trace_index,
                    0,
                    GL_TRUE,
                    0,
                    GL_WRITE_ONLY,
                    GL_R32UI
                );
                glDispatchCompute(width, height, 1);
            }

            {
                glUseProgram(g_program_light_point);
                glUniform3f(glGetUniformLocation(g_program_light_point, "lightPosition"), -4.0, 6.0, 4.0);
                glUniform3f(glGetUniformLocation(g_program_light_point, "lightColor"), 1.0, 1.0, 1.0);
                glBindImageTexture(
                    0,
                    g_texture_trace,
                    0,
                    GL_TRUE,
                    0,
                    GL_READ_ONLY,
                    GL_RGBA32F
                );
                glBindImageTexture(
                    glGetUniformLocation(g_program_light_point, "image_trace_index"),
                    g_texture_trace_index,
                    0,
                    GL_TRUE,
                    0,
                    GL_READ_ONLY,
                    GL_R32UI
                );
                auto location_screen = glGetUniformLocation(g_program_light_point, "image_screen");
                glBindImageTexture(
                    2,
                    g_texture_screen,
                    0,
                    GL_TRUE,
                    0,
                    GL_WRITE_ONLY,
                    GL_RGBA32F
                );
                glDispatchCompute(width, height, 1);
            }
            {
                glUseProgram(g_program_present);
                glBindImageTexture(
                    0,
                    g_texture_screen,
                    0,
                    GL_TRUE,
                    0,
                    GL_READ_ONLY,
                    GL_RGBA32F
                );
                glBindVertexArray(g_array_screen);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(0));
                glBindVertexArray(0);
            }

            glUseProgram(0);

            glEndQuery(GL_TIME_ELAPSED);
        }

        SDL_GL_SwapWindow(m_window);

        if (is_running) {
            GLuint64 time_elapsed;
            glGetQueryObjectui64v(g_query_time_measure, GL_QUERY_RESULT, &time_elapsed);
            fprintf(stderr, "[frame.time][%d][%d]: %lu ns\n", width, height, time_elapsed);
        }

        glFinish();
    }

    Screen::~Screen() = default;
}
