#pragma once

#define ENABLE_IMGUI

#include <functional>
#include <iostream>
#include <map>


struct GLAppSettings
{
    bool fullscreen = false;
    bool vsync = true; ///< Vertical sync
    int samples = 8; ///< MSAA samples
};


struct GLFWwindow;

struct MouseEvent
{
    enum class Type {
        ButtonPress,
        ButtonRelease,
        Move
    };
    enum class Button {
        None,
        Left,
        Right,
        Middle
    };

    Type type;
    double x, y;
    Button button;
};

typedef std::function<void(MouseEvent)> MouseFunction;

class GLFWApp
{
public:
    static const char* getGLSLVersionString();
protected:
    static std::map<GLFWwindow*, MouseFunction> s_mouseFunctions;
    static void glfw_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void glfw_error_callback(int error, const char* description);

public:
    GLFWApp();
    ~GLFWApp();

    bool create(GLAppSettings settings=GLAppSettings());

    void setMouseFunction(MouseFunction f);

    bool isFullscreen() const;
    void setFullscreen(bool enable);
    
    bool running();

    int width() const { return m_width; }
    int height() const { return m_height; }

    void beginFrame();
    void endFrame();

    /// Returns time since last call to tic() in seconds and 0.0 on first call
    double tic() const;

protected:
    bool setupOpenGLLoader(std::ostream& os = std::cout);
    void print_gl_info(std::ostream& os = std::cout);

private:
    GLFWwindow* m_window = nullptr;
    int m_width=0;
    int m_height=0;
    int m_windowWidth=0;
    int m_windowHeight=0;
    int m_windowPosX=0;
    int m_windowPosY=0;
};
