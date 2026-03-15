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

// @todo: Fix to take SimpleGeometry const&
void copySimpleGeometryToMeshBuffer(SimpleGeometry& simpleGeometry, MeshBuffer& meshBuffer)
{
    meshBuffer.setNumVertices( simpleGeometry.num_vertices() );
    meshBuffer.setNumIndices( simpleGeometry.num_faces() * 3 );
    meshBuffer.resize(simpleGeometry.num_vertices(), simpleGeometry.num_faces());
    meshBuffer.setVertices(std::span<float>(simpleGeometry.get_vertex_ptr(), simpleGeometry.num_vertices()*3));
    meshBuffer.setNormals(std::span<float>(simpleGeometry.get_normal_ptr(), simpleGeometry.num_vertices()*3));
    meshBuffer.setIndices(std::span<unsigned>(simpleGeometry.get_index_ptr(), simpleGeometry.num_faces()*3));
}

class GeometryScene
{
public:  
    GeometryScene(std::shared_ptr<SimpleGeometry> simpleGeometry)
    : geometry(simpleGeometry)
    {
        assert(simpleGeometry != nullptr);
        assert(geometry->num_vertices() > 0);
    }
    
    bool create()
    {
        meshBuffer = std::make_shared<MeshBuffer>();
        copySimpleGeometryToMeshBuffer(*geometry, *meshBuffer);
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
        meshBuffer = std::make_shared<MeshBuffer>();
        copySimpleGeometryToMeshBuffer(*geometry, *meshBuffer);
        glmesh.setMeshBuffer(meshBuffer);
        glmesh.prepare();

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

    SimpleGeometry* getSimpleGeometry()
    {
        return geometry.get();
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

    auto icosahedron = std::make_shared<Icosahedron>();
    auto penrose = std::make_shared<Penrose>();
    auto superquadric = std::make_shared<Superquadric>();
    auto sphericalHarmonics = std::make_shared<SphericalHarmonics>();
    auto sphereFunction = std::make_shared<SphericalHarmonicsFunction>();
    
    int const DefaultSubdivisionLevels = 1;

    icosahedron->create(DefaultSubdivisionLevels);
    penrose->create(DefaultSubdivisionLevels);
    superquadric->create(DefaultSubdivisionLevels);
    sphericalHarmonics->create(DefaultSubdivisionLevels);
    sphereFunction->create(DefaultSubdivisionLevels);

    std::vector const scenes = { 
        std::make_shared<GeometryScene>(icosahedron),
        std::make_shared<GeometryScene>(superquadric),
        std::make_shared<GeometryScene>(sphericalHarmonics),
        std::make_shared<GeometryScene>(sphereFunction),
        std::make_shared<GeometryScene>(penrose)
    };

    for(auto scene : scenes)
        if(!scene->create())
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
        int sceneIndex = 0;
        float clear_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        bool animate = false;
        bool wireframe = true;
        bool blending = true;
        bool culling = true;
        float zoom = 3.f;
        float t = 0.f;
    } globals;

    while (app.running())
    {
        GeometryScene& scene = *scenes[globals.sceneIndex % scenes.size()].get();

        app.beginFrame();
        GL::clearGLError("main - begin of main loop");

        // Gui
        {
            ImGui::Begin("geometry2");

            ImGui::SliderInt("Scene", &globals.sceneIndex, 0, scenes.size()-1);
      
            auto fuzzy_equal = [](float a, double b)
            {
                return std::fabs(a - static_cast<float>(b)) < 1e-6;
            };

            ImGui::SeparatorText("Scene specific parameters");
            if(auto icosahedron = dynamic_cast<Icosahedron*>(scene.getSimpleGeometry()))
            {
                float x = globals.animate ? globals.t : icosahedron->getPlatonicConstantsX();
                float z = icosahedron->getPlatonicConstantsZ();
                ImGui::SliderFloat("X", &x, 0.003f, 5.0f);
                ImGui::SliderFloat("Z", &z, 0.5f, 5.0f);

                if(ImGui::Button("Phi"))
                {
                    x = 1.6180339887498948482045f; // golden ratio
                    z = 1.0;
                }

                if( globals.animate
                || !fuzzy_equal(x, icosahedron->getPlatonicConstantsX()) 
                || !fuzzy_equal(z, icosahedron->getPlatonicConstantsZ()) )
                {
                    icosahedron->clear();
                    icosahedron->setPlatonicConstants(x, z);
                    icosahedron->create();
                    scene.update();
                }
            }
            
            if(auto superquadric = dynamic_cast<Superquadric*>(scene.getSimpleGeometry()))
            {
                float alpha = superquadric->getAlpha();
                float beta = superquadric->getBeta();
                ImGui::SliderFloat("alpha", &alpha, 0.01, 3.99);
                ImGui::SliderFloat("beta", &beta, 0.01, 3.99);

                if(ImGui::Button("Q0"))
                {
                    alpha = 0.5;
                    beta = 0.5;
                }

                if(!fuzzy_equal(alpha, superquadric->getAlpha()) 
                || !fuzzy_equal(beta, superquadric->getBeta()))
                {
                    superquadric->clear();
                    superquadric->setQuadric(alpha, beta);
                    superquadric->create();
                    scene.update();
                }
            }
            
            if(auto sphericalHarmonics = dynamic_cast<SphericalHarmonics*>(scene.getSimpleGeometry()))
            {
                int L = sphericalHarmonics->getL();
                int M = sphericalHarmonics->getM();
                ImGui::SliderInt("L", &L, 0, 10);
                ImGui::SliderInt("M", &M, 0, L);

                if(ImGui::Button("LM"))
                {
                    L = 4;
                    M = 0;
                }

                if(L != sphericalHarmonics->getL() || M != sphericalHarmonics->getM())
                {
                    sphericalHarmonics->clear();
                    sphericalHarmonics->setLM(L, M);
                    sphericalHarmonics->create();
                    scene.update();
                }
            }

            if(auto sphericalHarmonicsFunction = dynamic_cast<SphericalHarmonicsFunction*>(scene.getSimpleGeometry()))
            {
                bool dirty = false;
                if(ImGui::Button("Zero"))
                {
                    sphericalHarmonicsFunction->resetCoefficients();
                    dirty = true;
                }
                ImGui::SameLine();
                if(ImGui::Button("Rand"))
                {
                    sphericalHarmonicsFunction->randomizeCoefficients();
                    dirty = true;
                }
                ImGui::SameLine();
                if(ImGui::Button("Sym"))
                {
                    sphericalHarmonicsFunction->symmetrizeCoefficients();
                    dirty = true;
                }

                if(dirty)
                {
                    sphericalHarmonicsFunction->update();
                    scene.update();
                }
            }

            auto changeLevel = [&](int levelChange)
            {
                auto& geometry = *scene.getSimpleGeometry();
                geometry.clear();
                geometry.create(levelChange == 0 ? DefaultSubdivisionLevels : std::max(geometry.getLevels() + levelChange, 0)); 
                scene.update();
            };
            
            ImGui::SeparatorText("Subdivision levels (if applicable)");
            ImGui::Text(std::to_string(scene.getSimpleGeometry()->getLevels()).c_str());
            ImGui::SameLine();
            if(ImGui::Button("Reset")) changeLevel(0);
            ImGui::SameLine();
            if(ImGui::Button("-")) changeLevel(-1);
            ImGui::SameLine();
            if(ImGui::Button("+")) changeLevel(+1);

            ImGui::SeparatorText("Common options");            
            ImGui::Checkbox("Animate", &globals.animate);
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
            constexpr float secondsPerCycle = 4.f;
            constexpr float tmax = 5.f;
            globals.t += (float)app.tic()*tmax/secondsPerCycle;
            while (globals.t >= tmax) globals.t -= tmax;
        }      
    }

    return 0;
}
