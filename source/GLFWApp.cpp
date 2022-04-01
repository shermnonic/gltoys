#include "GLFWApp.h"

const int MY_GL_VERSION_MAJOR = 3;
const int MY_GL_VERSION_MINOR = 2;
#define MY_GLEW_GL_VERSION GLEW_VERSION_3_2
const char* MY_GLSL_VERSION = "#version 150";

#ifdef ENABLE_IMGUI
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>


const char* GLFWApp::getGLSLVersionString()
{
    return MY_GLSL_VERSION;
}


std::map<GLFWwindow*, MouseFunction> GLFWApp::s_mouseFunctions;

void GLFWApp::glfw_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (s_mouseFunctions.count(window))
    {
        MouseEvent e{ MouseEvent::Type::Move, xpos, ypos, MouseEvent::Button::None };
        auto& f = s_mouseFunctions[window];
        f(e);
    }
}

void GLFWApp::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (s_mouseFunctions.count(window))
    {
        const static std::map<int, MouseEvent::Button> m{
            { (int)GLFW_MOUSE_BUTTON_LEFT,   MouseEvent::Button::Left   },
            { (int)GLFW_MOUSE_BUTTON_MIDDLE, MouseEvent::Button::Middle },
            { (int)GLFW_MOUSE_BUTTON_RIGHT,  MouseEvent::Button::Right  }
        };
        if (m.count(button))
        {
            MouseEvent e{ action==GLFW_PRESS ? MouseEvent::Type::ButtonPress : MouseEvent::Type::ButtonRelease, -1, -1, m.at(button) };
            auto& f = s_mouseFunctions[window];
            f(e);
        }
    }
};

void GLFWApp::glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}



GLFWApp::GLFWApp() 
{}
    
GLFWApp::~GLFWApp()
{
    if( m_window )
        glfwDestroyWindow(m_window);
    glfwTerminate();
}


bool GLFWApp::create(GLAppSettings settings)
{
    // Setup GLFW window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MY_GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MY_GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // required on Mac
    glfwWindowHint(GLFW_SAMPLES, settings.samples);

    int width = 1280;
    int height = 720;

    GLFWmonitor* monitor = nullptr; // only needed to be specified for fullscreen
    if(settings.fullscreen)
    {
        int count=0;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        if(count>1 && monitors)
        {
            // Use 2nd monitor by default
            // Assume full HD
            monitor = monitors[1];
            width = 1920;
            height = 1080;
        }
        else
        {
            monitor = glfwGetPrimaryMonitor();
        }
    }

    m_window = glfwCreateWindow(width,height,"386dx25",monitor,nullptr);
    if (!m_window)
        return false;
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(settings.vsync ? 1 : 0);

    glfwSetCursorPosCallback(m_window, glfw_cursor_pos_callback);
    glfwSetMouseButtonCallback(m_window, glfw_mouse_button_callback);

    if( !setupOpenGLLoader() )
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }

    print_gl_info();

#ifdef ENABLE_IMGUI
    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(MY_GLSL_VERSION);

    //io.Fonts->AddFontFromFileTTF("Cherry Monospace-Light.ttf", 18);
    //io.Fonts->AddFontFromFileTTF("Hack-Regular.ttf", 13);
    //io.Fonts->AddFontFromFileTTF("Flexi_IBM_VGA_True_437.ttf", 14);
    //io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 14);
#endif        
    return true;
}

void GLFWApp::setMouseFunction(MouseFunction f)
{
    s_mouseFunctions[m_window] = f;
}

bool GLFWApp::isFullscreen() const
{
    return glfwGetWindowMonitor(m_window) != nullptr;
}

GLFWmonitor* getCurrentMonitor(GLFWwindow* window)
{
    // simplified variant of https://stackoverflow.com/questions/21421074/how-to-create-a-full-screen-window-on-the-current-monitor-with-glfw
    // identify on which monitor the upper left corner of the window is positioned
    int wx=0, wy=0;
    glfwGetWindowPos(window, &wx, &wy);
    int numMonitors=0;
    GLFWmonitor** monitors = glfwGetMonitors(&numMonitors);
    for(int i=0; i < numMonitors; ++i)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        int mx=0, my=0;
        glfwGetMonitorPos(monitors[i], &mx, &my);
        int mw = mode->width,
            mh = mode->height;

        if(wx >= mx && wx < mx+mw &&
           wy >= my && wy < my+mh)
            return monitors[i];
    }
    return glfwGetPrimaryMonitor();
}

void GLFWApp::setFullscreen(bool enable)
{
    if(isFullscreen() == enable)
        return;

    if(enable)
    {
        glfwGetWindowPos(m_window,&m_windowPosX,&m_windowPosY);

        m_windowWidth = m_width;
        m_windowHeight = m_height;

        GLFWmonitor* monitor = getCurrentMonitor(m_window);
        const GLFWvidmode* mode = glfwGetVideoMode(monitor); 
        glfwSetWindowMonitor(m_window,monitor,0,0,mode->width,mode->height,mode->refreshRate);
    }
    else
    {
        GLFWmonitor* monitor = nullptr;
        glfwSetWindowMonitor(m_window,monitor,m_windowPosX,m_windowPosY,m_windowWidth,m_windowHeight,GLFW_DONT_CARE);
    }
}
    
bool GLFWApp::running()
{
    return !glfwWindowShouldClose(m_window);
}

void GLFWApp::beginFrame()
{
    glfwPollEvents();
    glfwGetFramebufferSize(m_window, &m_width, &m_height);
#ifdef ENABLE_IMGUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
#endif
}
    
void GLFWApp::endFrame()
{
#ifdef ENABLE_IMGUI
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
    glfwSwapBuffers(m_window);
}

double GLFWApp::tic() const
{
    static double lastTime = 0.0;
    const double currentTime = glfwGetTime();
    const bool firstInvocation = lastTime == 0.0;
    if(!firstInvocation)
    {
        const double delta = currentTime - lastTime;
        lastTime = currentTime;
        return delta;
    }
    else
    {
        lastTime = currentTime;
        return 0.0;
    }
}

bool GLFWApp::setupOpenGLLoader( std::ostream& os )
{
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(GLEW_OK != err)
    {
        std::cerr <<"Error: "<< glewGetErrorString(err) << std::endl;
        return false;
    } 
    else 
    {
        if(MY_GLEW_GL_VERSION)
        {
            os << "Driver supports OpenGL " << MY_GL_VERSION_MAJOR << "." << MY_GL_VERSION_MINOR << std::endl;
        }
    }
    return true;
}


void GLFWApp::print_gl_info( std::ostream& os )
{
    os << "Vendor:          " << glGetString(GL_VENDOR)    << std::endl;
    os << "Renderer:        " << glGetString(GL_RENDERER)  << std::endl;
    os << "Version OpenGL:  " << glGetString(GL_VERSION)   << std::endl;
    os << "Version GLSL:    " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl; 
}
