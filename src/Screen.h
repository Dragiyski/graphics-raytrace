#ifndef RAYTRACE_SCREEN_H
#define RAYTRACE_SCREEN_H

#include <map>
#include <memory>
#include <SDL2/SDL.h>
#include <GL/gl.h>

namespace dragiyski::raytrace {
    class Screen {
    private:
        SDL_Window *m_window;
        SDL_GLContext m_context;
        bool m_is_initialized;
        bool m_need_resize;
        GLuint g_buffer_vertex_screen, g_buffer_index_screen, g_array_screen, g_program_present, g_texture_screen;
        GLuint g_program_clear, g_program_screen, g_texture_ray, g_texture_trace, g_texture_trace_index;
        GLuint g_program_raytrace_sphere;
        GLuint g_program_light_point;
        GLuint g_query_time_measure;
        static std::map<uint32_t, std::shared_ptr<Screen>> window_screen_map;
    private:
        explicit Screen(SDL_Window *, SDL_GLContext);
    public:
        virtual ~Screen();
    public:
        static std::shared_ptr<Screen> New(const char *title);
        static void notify(const SDL_Event &);
    private:
        void notifyWindow(const SDL_Event &);
    public:
        void update();
    protected:
        void initialize();
        void resize();
        void paint();
        void release();
    };
}

#endif //RAYTRACE_SCREEN_H
