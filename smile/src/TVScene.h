#ifndef TVScene_H
#define TVScene_H

#include <ngl/Obj.h>
#include "scene.h"

class TVScene : public Scene
{
public:
    TVScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

private:
    GLuint m_diffTex, m_specTex, m_anasTex;
    std::unique_ptr<ngl::Obj> m_mesh;
    int amountVertexData = 0;

    /// Initialise a texture
    void initTexture(const GLuint& /*texUnit*/, GLuint &/*texId*/, const char */*filename*/);
};

#endif // TVScene_H
