#ifndef TRACKBALL2_H
#define TRACKBALL2_H

#include <glm/glm.hpp> // glm::vec3, glm::vec4, glm::ivec4, glm::mat4
#include <glm/gtc/quaternion.hpp> // glm::fquat

/// Trackball camera control, uses glm template library.
/// Call start(), update() and stop() during interaction and retrieve camera
/// matrix via getCameraMatrix().
class Trackball2
{
public:
    enum Mode { None, Rotate, Translate, Zoom };
  
    int getMode() const { return m_mode; }

    void setZoom( float zoom ) { m_zoom = zoom; }
    float getZoom() const { return m_zoom; }

    void setSpeed( float speed ) { m_speed = speed; }
    float getSpeed() const { return m_speed; }

    /// Set viewport size in pixels.
    /// Only required for update function with pixel positions as input.
    void setViewSize( int w, int h )
    {
        m_viewWidth  = w;
        m_viewHeight = h;
    }

    void start( int ax, int ay, int mode );
    void start( float ax, float ay, int mode )
    {
        m_ax = ax;
        m_ay = ay;
        m_mode = mode;
    }

    void stop()
    {
        if( !m_immediateUpdate && m_mode == Rotate )
        {
            m_qrot = glm::normalize( glm::cross( m_cur_qrot, m_qrot ) );
            m_cur_qrot = glm::fquat(1,0,0,0);
        }
        m_mode = None;
    }

    /// Perform trackball update given two pixel view position.
    /// Input coordinates are pixel coordinates and require that width/height
    /// of view is set correctly via \a setViewSize() in advance.
    void update( int bx, int by );
    
    /// Perform trackball update given two normalized view coordinates.
    void update( float bx, float by )
    {
        constexpr float eps = 2*std::numeric_limits<float>::epsilon();
        if( fabs(bx-m_ax)<eps && fabs(by-m_ay)<eps )
            return;

        update( m_ax, m_ay, bx, by, m_mode );
        if( m_immediateUpdate )
        {
            m_ax = bx;
            m_ay = by;
        }
    }
    
    /// Return current homogeneous trackball transformation matrix
    glm::mat4 getModelviewMatrix() const;

    /// Return pure rotation part of current trackball matrix, \sa getModelviewMatrix
    glm::mat3 getRotationMatrix() const;

protected:
    void update( int ax, int ay, int bx, int by, int mode );   
    void update( float ax, float ay, float bx, float by, int mode );

    /// Ortho-project coordinate from normalized cube onto a sphere
    glm::vec3 projectToSphere( float x, float y );
    /// Find equivalent single rotation between unit quaternions q1 and q2
    glm::fquat combineRotations( glm::fquat q1, glm::fquat q2 );
    
private:
    int m_viewWidth = 0;
    int m_viewHeight = 0;
    int m_mode = None;

    float m_ax = 0.f; // Previous positions 
    float m_ay = 0.f; 

    float m_trackballRadius = .8f;

    glm::fquat m_qrot     = glm::fquat(1,0,0,0); // Rotation quaternion
    glm::fquat m_cur_qrot = glm::fquat(1,0,0,0); // Current, additional rotation user is performing
    glm::vec3  m_trans    = {}; // Translation
    float      m_zoom  = 5.f;   // Zoom factor
    float      m_speed = 10.f;  // Speed of translation and zoom (depends on model size)

    bool m_immediateUpdate;
};

#endif // TRACKBALL2_H
