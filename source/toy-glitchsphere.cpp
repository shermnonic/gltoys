#include <imgui.h>
#include <GLFWApp.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glutils/GLError.h>
#include <glutils/GLMeshObject.h>
#include <glutils/MeshShader.h>
#include <glutils/MeshBufferIO.h> // writeOBJ()
#include <glutils/Trackball2.h>

#include <GlitchSphereGeometry.h>

#include <fstream>

struct GlitchSphereParameters
{
    int resolution = 46;
    float t = 0.f;
    float lambda = 1.f;
    int colormap = 0;
};

class GlitchSphereScene
{
public:
    bool create()
    {
        m_geometry = std::make_shared<GlitchSphereGeometry>(MeshVertexAttribute::Color | MeshVertexAttribute::Normal | MeshVertexAttribute::UV);
        m_glmesh.setMeshBuffer(m_geometry);
        m_shader = std::make_shared<MeshShader>(m_geometry->getAttributes(), GLFWApp::getGLSLVersionString());
        
        if (!m_shader || !m_shader->load())
        {
            std::cerr << "Error compiling/linking shader" << std::endl;
            return false;
        }

        return GL::checkGLError("GlitchSphereScene::create()");
    }

    void update(const GlitchSphereParameters& params)
    {
        const float radius = 1.f;
        m_geometry->createSphereGeometryWithGlitch(0.f, 0.f, 0.f, radius, params.resolution, params.t, params.lambda, params.colormap);
        m_glmesh.setDirty();
        m_glmesh.prepare();
    }

    void render(const glm::mat4& modelview, const glm::mat4& projection)
    {
        glm::mat4 MVP = projection * modelview;
        m_shader->bind(glm::value_ptr(MVP));
        m_glmesh.draw();
    }

    MeshShader::Uniforms& uniforms()
    {
        return m_shader->uniforms();
    }

    const MeshBuffer* meshBuffer()
    {
        return m_geometry.get();
    }

private:
    std::shared_ptr<GlitchSphereGeometry> m_geometry;
    std::shared_ptr<MeshShader> m_shader;
    GLMeshObject m_glmesh;
};

int main(int argc, char* argv[])
{
    GLFWApp app;
    if (!app.create())
        return 1;

    GlitchSphereScene scene;
    if (!scene.create())
        return 2;

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
            if(!ImGui::IsMouseHoveringAnyWindow())
                trackball.start(mousex, mousey, Trackball2::Rotate);
        }
        else if (e.type == MouseEvent::Type::ButtonRelease)
        {
            trackball.stop();
        }
    });
    
    struct Globals 
    {
        float clear_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        bool animate = false;
        bool wireframe = true;
        bool blending = true;
        bool culling = true;
        float zoom = 3.f;
    } globals;

    GlitchSphereParameters params;

    const float tmax = 2 * 3.1415f;

    while (app.running())
    {
        app.beginFrame();
        GL::clearGLError("main - begin of main loop");

        // Gui
        {
            ImGui::Begin("glitchsphere");
            ImGui::Checkbox("Wireframe", &globals.wireframe);
            ImGui::Checkbox("Shading", &scene.uniforms().shading);
            ImGui::Checkbox("Blending", &globals.blending);
            ImGui::Checkbox("Culling", &globals.culling);
            ImGui::Checkbox("Animate", &globals.animate);
            ImGui::SliderFloat("Zoom", &globals.zoom, 1.f, 10.0f);
            ImGui::SliderInt("Resolution", &params.resolution, 1, 128);
            ImGui::SliderFloat("Param t", &params.t, 0.f, tmax);
            ImGui::SliderFloat("Param lambda", &params.lambda, 0.f, 10.f);
            ImGui::SliderInt("Colormap", &params.colormap, 0, 10);
            if (ImGui::Button("Save"))
            {
                const MeshBuffer* mb = scene.meshBuffer();
                if (mb)
                {
                    std::ofstream of("glitchsphere.obj");
                    if (of.is_open())
                    {
                        writeOBJ(of, *mb);
                        of.close();
                    }
                }
            }
            if (ImGui::Button("Fullscreen"))
                app.setFullscreen(!app.isFullscreen());
            ImGui::ColorEdit3("Foreground", scene.uniforms().color);
            ImGui::ColorEdit3("Background", globals.clear_color);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        ImGui::Render();
        GL::clearGLError("main - ImGui::Render()");


        scene.update(params);

        // Frame
        {
            int width = app.width();
            int height = app.height();

            trackball.setViewSize(width, height);

            glViewport(0, 0, width, height);
            glClearColor(globals.clear_color[0], globals.clear_color[1], globals.clear_color[2], globals.clear_color[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            GL::checkGLError("main - glClear()");

            glEnable(GL_DEPTH_TEST);

            if (globals.culling)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);
            
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            if (globals.blending)
                glEnable(GL_BLEND);
            else
                glDisable(GL_BLEND);

            glPolygonMode(GL_FRONT_AND_BACK, globals.wireframe ? GL_LINE : GL_FILL);
            GL::checkGLError("main - glPolygonMode()");

            float aspect = width / (float)height;
            scene.render(
                glm::translate(glm::mat4(1.0), glm::vec3(0.f, 0.f, -globals.zoom))
                * glm::mat4(trackball.getRotationMatrix()),
                glm::perspective(glm::radians(45.f), aspect, .1f, 100.f));
        }

        app.endFrame();


        if (globals.animate)
        {
            const float secondsPerCycle = 4.f;
            params.t += (float)app.tic()*tmax/secondsPerCycle;
            while (params.t >= tmax) params.t -= tmax;
        }
    }

    return 0;
}
