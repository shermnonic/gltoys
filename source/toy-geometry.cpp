// toy-geometry (based on toy-glitchsphere)
#include <imgui.h>
#include <GLFWApp.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glutils/GLError.h>
#include <glutils/GLMeshObject.h>
#include <glutils/MeshShader.h>
#include <glutils/MeshBufferIO.h> // writeOBJ()
#include <glutils/Trackball2.h>

#include <geometry/Geometry2.h>

#include <fstream>
#include <span>

class GeometryScene
{
private:
    void copyGeometryToMeshBuffer()
    {
        if(meshBuffer && geometry)
        {
            meshBuffer->setNumVertices( geometry->num_vertices() );
            meshBuffer->setNumIndices( geometry->num_faces() * 3 );
            meshBuffer->resize(geometry->num_vertices(), geometry->num_faces());
            meshBuffer->setVertices(std::span<float>(geometry->get_vertex_ptr(), geometry->num_vertices()*3));
            meshBuffer->setNormals(std::span<float>(geometry->get_normal_ptr(), geometry->num_vertices()*3));
            meshBuffer->setIndices(std::span<unsigned>(geometry->get_index_ptr(), geometry->num_faces()*3));
        }
    }

public:  
    bool create()
    {
        geometry = std::make_shared<Icosahedron>();
        SimpleGeometry* simpleGeometry = geometry.get();
        if(auto icosahedron = dynamic_cast<Icosahedron*>(simpleGeometry))
        {
            icosahedron->create(1);
        }
        else
        {
            return false;
        }

        meshBuffer = std::make_shared<MeshBuffer>();

        copyGeometryToMeshBuffer();

        glmesh.setMeshBuffer(meshBuffer);
        glmesh.prepare();
        shader = std::make_shared<MeshShader>(meshBuffer->getAttributes(), GLFWApp::getGLSLVersionString());
        
        if (!shader || !shader->load())
        {
            std::cerr << "Error compiling/linking shader" << std::endl;
            return false;
        }

        return GL::checkGLError("GeometryScene::create()");
    }

    void update()
    {
        glmesh.setDirty();
        glmesh.prepare();
    }

    void render(const glm::mat4& modelview, const glm::mat4& projection)
    {
        glm::mat4 MVP = projection * modelview;
        shader->bind(glm::value_ptr(MVP));
        glmesh.draw();
    }

    MeshShader::Uniforms& getUniforms()
    {
        return shader->uniforms();
    }

    const MeshBuffer* getMeshBuffer()
    {
        return meshBuffer.get();
    }

private:
    std::shared_ptr<SimpleGeometry> geometry;
    std::shared_ptr<MeshShader> shader;
    std::shared_ptr<MeshBuffer> meshBuffer;
    GLMeshObject glmesh;
};

int main(int argc, char* argv[])
{
    GLFWApp app;
    if (!app.create())
        return 1;

    GeometryScene scene;
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
            ImGuiIO& io = ImGui::GetIO();
            if(!io.WantCaptureMouse)
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
        bool wireframe = true;
        bool blending = true;
        bool culling = true;
        float zoom = 3.f;
    } globals;

    while (app.running())
    {
        app.beginFrame();
        GL::clearGLError("main - begin of main loop");

        // Gui
        {
            ImGui::Begin("geometry2");
            if(ImGui::CollapsingHeader("About"))
            {
                ImGui::Text("Glitchy geometries by www.386dx25.de.");
                ImGui::BulletText("Rotate via left mouse button.");
            }
            ImGui::Checkbox("Wireframe", &globals.wireframe);
            ImGui::Checkbox("Shading", &scene.getUniforms().shading);
            ImGui::Checkbox("Blending", &globals.blending);
            ImGui::Checkbox("Culling", &globals.culling);
            ImGui::SliderFloat("Zoom", &globals.zoom, 1.f, 10.0f);
            if (ImGui::Button("Save .obj"))
            {
                if (auto const* mb = scene.getMeshBuffer())
                {
                    std::ofstream of("geometry2.obj");
                    if (of.is_open())
                    {
                        writeOBJ(of, *mb);
                        of.close();
                    }
                }
            }
            if (ImGui::Button("Fullscreen"))
                app.setFullscreen(!app.isFullscreen());
            ImGui::ColorEdit3("Foreground", scene.getUniforms().color);
            ImGui::ColorEdit3("Background", globals.clear_color);
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        ImGui::Render();
        GL::clearGLError("main - ImGui::Render()");


        // scene.update(params);

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
    }

    return 0;
}
