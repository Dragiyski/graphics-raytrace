#include <iostream>
#include <stdexcept>
#include "global.h"
#include "Screen.h"

int main(int argc, char *argv[]) {
    using namespace dragiyski::raytrace;
    Screen::New("Raytrace");
    try {
        if (SDL_Init(SDL_INIT_EVENTS) < 0) {
            throw sdl_error(SDL_GetError());
        }
        SDL_Event event;
        while (true) {
            if (SDL_WaitEvent(&event) < 0) {
                throw sdl_error(SDL_GetError());
            }
            Screen::notify(event);
            if (event.type == SDL_QUIT) {
                break;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
