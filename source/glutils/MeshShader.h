#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glutils/GLError.h>
#include <glutils/GLSLProgram.h>
#include <string>

#include "MeshBufferTypes.h"

class MeshShader
{
public:
    MeshShader(MeshVertexAttribute attributes, const char* glslVersionString="#version 330") // 150, 330
    : m_attributes(attributes),
      m_glslVersionString(glslVersionString)
    {}

    bool load()
    {
        using std::string;
        string defines = m_glslVersionString+"\n" 
            + (has(m_attributes, MeshVertexAttribute::Normal) ? "#define HAS_ATTRIBUTE_NORMAL\n" : "")
            + (has(m_attributes, MeshVertexAttribute::Color)  ? "#define HAS_ATTRIBUTE_COLOR\n"  : "")
            + (has(m_attributes, MeshVertexAttribute::UV)     ? "#define HAS_ATTRIBUTE_UV\n"     : "");
        const string vertex_shader   = defines + c_vertex_shader;
        const string fragment_shader = defines + c_fragment_shader;

        bool ok = m_program.load( vertex_shader, fragment_shader );

        if(ok)
        {
            GLuint program = m_program.handle();
            m_loc_color   = glGetUniformLocation(program, "uniform_color");
            m_loc_shading = glGetUniformLocation(program, "shading");
            m_loc_mvp     = glGetUniformLocation(program, "m4_mvp");
            GL::checkGLError("MeshShader::load() - glGetUniformLocation");
        }

        return ok;
    }

    void setColor( float r, float g, float b, float a )
    {
        m_uniforms.color[0] = r;
        m_uniforms.color[1] = g;
        m_uniforms.color[2] = b;
        m_uniforms.color[3] = a;
        glUniform4fv(m_loc_color, 1, m_uniforms.color);
    }

    void bind( float* mvp=nullptr )
    {
        m_program.bind();

        // Note: For new (non Intel) HW location binding can be done directly in shader via layout directive.
        constexpr bool explicitBinding = false;
        if (explicitBinding)
        {
            glBindAttribLocation(m_program.handle(), 0, "v3_position");
            if (has(m_attributes, MeshVertexAttribute::Normal))
                glBindAttribLocation(m_program.handle(), 1, "v3_normal");
            if (has(m_attributes, MeshVertexAttribute::UV))
                glBindAttribLocation(m_program.handle(), 2, "v2_texcoord");
            if (has(m_attributes, MeshVertexAttribute::Color))
                glBindAttribLocation(m_program.handle(), 3, "v4_color");
            GL::checkGLError("MeshShader::bind() - glBindAttribLocation");
        }

        if( mvp )
            glUniformMatrix4fv(m_loc_mvp, 1, GL_FALSE, mvp);
        glUniform4fv(m_loc_color, 1, m_uniforms.color);
        glUniform1i(m_loc_shading, (int)m_uniforms.shading);
        GL::checkGLError("MeshShader::bind() - glGetUniformLocation");
    }

    struct Uniforms
    {
        float color[4] = { 1.f,1.f,1.f,1.f };
        bool shading = true;
    };

    Uniforms& uniforms() { return m_uniforms; }

private:
    MeshVertexAttribute m_attributes;
    std::string m_glslVersionString;
    Uniforms m_uniforms;

    GL::GLSLProgram m_program;
    GLint m_loc_color   =-1;
    GLint m_loc_shading =-1;
    GLint m_loc_mvp     =-1;

    // layout(location = 0) 
    const char* c_vertex_shader = R"(
in vec3 v3_position;
out vec3 position;
#ifdef HAS_ATTRIBUTE_NORMAL
in vec3 v3_normal;
out vec3 normal;
#endif
#ifdef HAS_ATTRIBUTE_UV
in vec2 v2_texcoord;
out vec2 uv;
#endif
#ifdef HAS_ATTRIBUTE_COLOR
in vec4 v4_color;
out vec4 color;
#endif
uniform mat4 m4_mvp;
void main() 
{
    gl_Position = m4_mvp*vec4(v3_position, 1.0);
#ifdef HAS_ATTRIBUTE_NORMAL
    normal = v3_normal;
#endif
#ifdef HAS_ATTRIBUTE_UV
    uv = v2_texcoord;
#endif
#ifdef HAS_ATTRIBUTE_COLOR
    color = v4_color;
#endif
    position = v3_position;
};
)";

    const char* c_fragment_shader = R"(
out vec4 frag_color;
#ifdef HAS_ATTRIBUTE_NORMAL
in vec3 normal;
#endif
#ifdef HAS_ATTRIBUTE_UV
in vec2 uv;
#endif
#ifdef HAS_ATTRIBUTE_COLOR
in vec4 color;
#endif
in vec3 position;
uniform bool shading;
uniform vec4 uniform_color;
void main() 
{
#ifdef HAS_ATTRIBUTE_NORMAL
    //float diffuse = shading ? abs(dot(normalize(normal),normalize(position-vec3(0,0,-5)))) : 1.0;
    float diffuse = shading ? max(0.0,dot(normalize(normal),normalize(position-vec3(0,0,-5)))) : 1.0;
#else
    float diffuse = 1.0;
#endif
    vec4 out_color = uniform_color;
#ifdef HAS_ATTRIBUTE_COLOR
    out_color *= color;
#endif
    frag_color = diffuse*out_color;
};
)";

};
