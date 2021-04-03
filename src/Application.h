#ifndef RAYTRACE_APPLICATION_H
#define RAYTRACE_APPLICATION_H

#include <any>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>

class Application {
    std::map<std::string, std::any> m_resources;
protected:
    void initializeGL();
    void paintGL();
    void releaseGL();
public:
    explicit Application();
    virtual ~Application();
public:
    void run();
};

class sdl_error : public std::runtime_error {
public:
    explicit sdl_error(const char *message);
    ~sdl_error() override = default;
};

#endif //RAYTRACE_APPLICATION_H
