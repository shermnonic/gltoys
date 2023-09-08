// toy-mnoise 2019
// TODO:
// [x] avoid realloc and vector copy
// [x] compute threads
// [x] fix gpu issues and flickering
// [x] trackball control
// [x] control mnoise position (but not noise function yet)
// [x] offscreen hd render target (no line thickness scaling, only .tga format)
// [x] obj export
// [ ] svg render target
// [ ] for pure svg cli decouple parallel compute and GL render code
#include <algorithm>
#include <array>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <imgui.h>
#include "GLFWApp.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glutils/MeshShader.h> 
#include <glutils/MeshBufferIO.h> // writeOBJ()

#include <glutils/GLError.h>
#include <glutils/GLSLProgram.h>
#include <glutils/OffscreenRendering.h>
#include <glutils/Trackball2.h>

#include <utils/TGA.h>

#include "MCubesObjectRenderer.h"


void writeOBJtoFile(std::string filename, const MeshBuffer& meshBuffer)
{
    std::ofstream of(filename);
    if (of.is_open())
    {
        writeOBJ(of,meshBuffer);
        of.close();
    }
}


struct MCubesParameters
{
    int resolution = 4;
    float scale = 1.f;
    float iso = .5f;
    float posx = 0.f;
    float posy = 0.f;
    float posz = 0.f;
};

class MCubesScene
{
public:
    const unsigned numRows = 4;

    bool create()
    {
        if(!m_mcubes.create(numRows))
        {
            std::cerr << "Error creating mesh object" << std::endl;
            return false;
        }
        if(!m_shader.load())
        {
            std::cerr << "Error compiling/linking shader" << std::endl;
            return false;
        }


        return GL::checkGLError("main - init");
    }

    void render( const glm::mat4& modelview, const glm::mat4& projection )
    {
        glm::mat4 MVP = projection * modelview;
        m_shader.bind(glm::value_ptr(MVP));
        int n=(int)m_mcubes.numObjects;
        for(int i=0; i < n; ++i)
        {
            if( debug )
                m_shader.setColor(i/(float)(n-1),.5f,1.f-i/(float)(n-1),1.f);
            m_mcubes.draw(i);
        }
    }

    void update(MCubesParameters params)
    {
        update(params.posx+123.3456f,params.posy+732.5489f,params.posz+129.3983f,
            1.f/(params.scale*(2<<(params.resolution-1))-.5f),params.iso,params.resolution);
    }

    void update(float x, float y, float z, float scale, float iso, int pot)
    {
        m_mcubes.update(x,y,z,scale,iso,pot);
        m_isComputing = m_mcubes.isComputing;
    }

    std::string info()
    {
        std::stringstream os;
        int n=(int)m_mcubes.numObjects;
        for(int i=0; i < n; ++i)
        {
            os << "slice " << i 
               << " #verts " << m_mcubes.objects[i]->numVertices()
               << " #indices " << m_mcubes.objects[i]->numIndices()
               << std::endl;
        }
        return os.str();
    }

    MeshShader::Uniforms& uniforms()
    {
        return m_shader.uniforms();
    }

    bool isComputing() const { return m_isComputing; }

    bool debug = false;

    void saveOBJ(std::string filename)
    {
        if(m_mcubes.objects.size()==1)
        {
            writeOBJtoFile(filename, *m_mcubes.objects.at(0).get());
        }
        else
        {
            MeshBuffer meshBuffer;
            std::filesystem::path path(filename);
            for (size_t i = 0; i < m_mcubes.objects.size(); ++i)
            {
                if (const MeshBuffer* ptr = m_mcubes.objects.at(i).get())
                {
                    MeshBuffer tmp = *ptr;
                    meshBuffer.merge(tmp);
                }
            }
            writeOBJtoFile(path.string(), meshBuffer);
        }
    }

private:
    int m_width = 0;
    int m_height = 0;
    MCubesObjectRenderer m_mcubes;
    MeshShader m_shader{MeshVertexAttribute::Normal, GLFWApp::getGLSLVersionString()};
    bool m_isComputing = false;
};


int main(int argc, char* argv[])
{
    // Setup GLFW window
    GLFWApp app;
    if( !app.create() )
        return 1;

    Trackball2 trackball;
    int mousex = 0;
    int mousey = 0;
    app.setMouseFunction([&trackball, &mousex, &mousey](MouseEvent e) 
    {
        if (e.type == MouseEvent::Type::Move)
        {
            mousex = (int)e.x;
            mousey = (int)e.y;
            trackball.update(mousex, mousey);
        }
        else if (e.type == MouseEvent::Type::ButtonPress)
        {
            ImGuiIO& io = ImGui::GetIO();
            if(!io.WantCaptureMouse)
                trackball.start(mousex, mousey, Trackball2::Rotate);
        }
        else if (e.type == MouseEvent::Type::ButtonRelease)
        {
            trackball.stop();
        }
    });

    MCubesScene scene;
    if( !scene.create() )
        return 2;

    MCubesParameters params;
    struct Globals
    {
        float clear_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        bool wireframe = false;
        float zoom = 2.f;
        bool animate = true;
    } 
    globals;

    enum class Preset {
        Reset,
        Needles
    };
    auto setPreset = [&params, &globals](Preset p) {
        switch(p)
        {
        case Preset::Needles:
            params.resolution = 4;
            params.scale = 0.181f;
            params.iso = -0.108f;
            globals.zoom = 1.66f;
            globals.wireframe = true;
            break;
        case Preset::Reset:
            params = {};
            globals = {};
            break;
        default:
            break;
        }
    };

    auto renderFrame = [&](int width, int height)
    {
        glViewport(0, 0, width, height);
        glClearColor(globals.clear_color[0], globals.clear_color[1], globals.clear_color[2], globals.clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GL::checkGLError("main - glClear()");
        
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glPolygonMode( GL_FRONT_AND_BACK, globals.wireframe ? GL_LINE : GL_FILL );
        GL::checkGLError("main - glPolygonMode()");

        scene.update(params);
        float aspect = width/(float)height;
        scene.render(glm::translate( glm::mat4(1.0), glm::vec3(0.f,0.f,-globals.zoom) )
                        * glm::mat4(trackball.getRotationMatrix()),
                        glm::perspective(glm::radians(45.f), aspect, .1f, 100.f));
    };

    bool ui_disabled = true;
    bool trigger_offscreen_rendering_screenshot = false;
    float computing_duration = 0.f;
    while(app.running())
    {
        app.beginFrame();
        GL::clearGLError("main - begin of main loop");

        // Gui
        {
            ImGui::Begin("mnoise");
            if (ui_disabled)
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

            if(ImGui::CollapsingHeader("About"))
            {
                ImGui::Text("Marching noise effect by www.386dx25.de.");
                ImGui::BulletText("Play around with the overdraw parameter.");
                ImGui::BulletText("Be careful when increasing resolution!");
                ImGui::BulletText("Rotate via left mouse button.");
            }

            ImGui::SeparatorText("Presets");
            if(ImGui::Button("Reset")) setPreset(Preset::Reset);
            ImGui::SameLine();
            if(ImGui::Button("Needles")) setPreset(Preset::Needles);

            ImGui::SeparatorText("Geometry");
            ImGui::SliderFloat("Overdraw",&params.scale,.1f,2.f);
            ImGui::SliderInt("Resolution",&params.resolution,1,7);
            ImGui::SliderFloat("Isovalue",&params.iso,-1.f,1.f);
            if (ImGui::Button("Save .obj"))
                scene.saveOBJ("mnoise.obj");

            if(ImGui::Button("Save .tga"))
                trigger_offscreen_rendering_screenshot = true;

            ImGui::SeparatorText("Camera");
            ImGui::Checkbox("Animate",&globals.animate);
            ImGui::SliderFloat("Zoom",&globals.zoom,.1f,4.2f);
            ImGui::InputFloat("PosX", &params.posx, 0.01f, 1.f, "%1.3f");
            ImGui::InputFloat("PosY", &params.posy, 0.01f, 1.f, "%1.3f");
            ImGui::InputFloat("PosZ", &params.posz, 0.01f, 1.f, "%1.3f");

            ImGui::SeparatorText("Rendering");
            ImGui::Checkbox("Wireframe", &globals.wireframe);
            ImGui::Checkbox("Shading", &scene.uniforms().shading);
            if (ImGui::Button("Fullscreen"))
                app.setFullscreen(!app.isFullscreen());
            ImGui::ColorEdit3("Foreground", scene.uniforms().color);
            ImGui::ColorEdit3("Background", globals.clear_color);

            ImGui::SeparatorText("Advanced");
            if(ImGui::CollapsingHeader("Stats & debug"))
            {
                ImGui::Checkbox("Debug colors",&scene.debug);
                ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::Text(scene.info().c_str());
            }

            if (ui_disabled)
                ImGui::PopStyleVar();
            ImGui::End();
        }
        ImGui::Render();
        GL::clearGLError("main - ImGui::Render()");

        float dt = (float)app.tic();

        // Frame
        {
            int width = app.width();
            int height = app.height();

            trackball.setViewSize(width, height);

            renderFrame(width, height);

            // Avoid flicker by disabling UI only if computation takes longer than a few frames
            bool is_computing_now = scene.isComputing();
            if(is_computing_now)
            {
                computing_duration += dt;
                if(computing_duration >= 10.f/60.f)
                {
                    ui_disabled = true;
                }
            }
            else
            {
                computing_duration = 0.f;
                ui_disabled = false;
            }
        }

        app.endFrame();

        // Offscreen rendering
        if(trigger_offscreen_rendering_screenshot)
        {
            trigger_offscreen_rendering_screenshot = false;

            auto getScreenshotFilename = []()
            {
                return L"toy-mnoise." + std::to_wstring(time(0)) + L".tga";
            };

            auto scaleLongestEdgeMaxSize = [](size_t width, size_t height) -> std::array<size_t, 2>
            {
                constexpr size_t MaxSize = 8096;
                const double aspect = width /(double)height;
                if(aspect >= 1.0) 
                {
                    return {MaxSize, static_cast<size_t>(MaxSize / aspect)};
                }
                else
                {
                    return {static_cast<size_t>(MaxSize * aspect), MaxSize};
                }
            };

            const auto [width, height] = scaleLongestEdgeMaxSize((size_t)app.width(), (size_t)app.height());

            OffscreenRendering renderer(width, height);
            renderer.initGL();
            renderer.renderBegin();

            renderFrame((int)width, (int)height);

            renderer.renderEnd();
            renderer.getRawImageData();

            auto format = renderer.getChannels()==4 ? TGAFormat::RGBA : TGAFormat::RGB;
            saveTGA(std::filesystem::path(getScreenshotFilename()), format, renderer.getWidth(), renderer.getHeight(), renderer.getRawImageData());

            renderer.destroyGL();
        }

        if (globals.animate)
        {
            const float speed = 0.2f;
            params.posz -= speed * dt;
        }
    }

    return 0;
}


