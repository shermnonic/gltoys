// toy-hello
#include <iostream>
#include <cstdio>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

const int MY_GL_VERSION_MAJOR = 3;
const int MY_GL_VERSION_MINOR = 2;
const char* MY_GLSL_VERSION = "#version 150";
#define MY_GLEW_GL_VERSION GLEW_VERSION_3_2
#define MY_GLSL_VERSION_STR "#version 150\n"

struct MeshShader
{
    const char* vertex_shader =
    MY_GLSL_VERSION_STR
    "in vec3 vp;"
    "void main() {"
    "  gl_Position = vec4(vp, 1.0);"
    "}";

    const char* fragment_shader =
    MY_GLSL_VERSION_STR
    "out vec4 frag_colour;"
    "void main() {"
    "  frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
    "}";

    bool compile()
    {
        int compileSuccess;

        vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertex_shader, NULL);
        glCompileShader(vs);

        glGetShaderiv(vs, GL_COMPILE_STATUS, &compileSuccess);
        if( !compileSuccess ) return false;
        
        fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragment_shader, NULL);
        glCompileShader(fs);

        glGetShaderiv(vs, GL_COMPILE_STATUS, &compileSuccess);
        if( !compileSuccess ) return false;

        return true;
    }

    bool link()
    {
        program = glCreateProgram();
        glAttachShader(program, fs);
        glAttachShader(program, vs);
        glLinkProgram(program);

        int linkSuccess;
        glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
        return linkSuccess;
    }

    GLuint program=0;
    GLuint vs=0;
    GLuint fs=0;
};

struct MeshObject
{
    std::vector<float> vertices = {
           0.0f, 0.5f, 0.0f,
           0.5f,-0.5f,0.0f,
          -0.5f,-0.5f,0.0f };

    bool create()
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        return glGetError() == GL_NO_ERROR;
    }

    GLuint vbo=0;
    GLuint vao=0;
};

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void print_gl_info()
{
  printf("Vendor:          %s\n", glGetString(GL_VENDOR));
  printf("Renderer:        %s\n", glGetString(GL_RENDERER));
  printf("Version OpenGL:  %s\n", glGetString(GL_VERSION));
  printf("Version GLSL:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

bool setupOpenGLLoader()
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
          std::cout << "Driver supports OpenGL " << MY_GL_VERSION_MAJOR << "." << MY_GL_VERSION_MINOR << std::endl;
        }
    }
    return true;
}

int main(int argc, char* argv[])
{
    // Setup GLFW window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MY_GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MY_GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // required on Mac

    GLFWwindow* window = glfwCreateWindow(1280,720,"toy-marching-noise",nullptr,nullptr);
    if (!window)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable vsync

    if( !setupOpenGLLoader() )
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    print_gl_info();

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(MY_GLSL_VERSION);

    float clear_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
    bool wireframe = false;

    MeshObject object;
    MeshShader shader;
    bool ok = object.create() && shader.compile() && shader.link();

    while(!glfwWindowShouldClose(window) && ok)
    {
        glfwPollEvents();

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            static float f = 0.0f;

            ImGui::Begin("toy");
            //ImGui::Text("Blabla");
            ImGui::Checkbox("Wireframe", &wireframe);
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::Button("Button"))
            {}
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        ImGui::Render();
        
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glPolygonMode( GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL );

        glUseProgram(shader.program);
        glBindVertexArray(object.vao);
        glDrawArrays(GL_TRIANGLES,0,3);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
