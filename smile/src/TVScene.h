#ifndef TVScene_H
#define TVScene_H

#include <ngl/Obj.h>
#include <GLFW/glfw3.h>
#include "scene.h"

class TVScene : public Scene
{
public:
  TVScene();

  /// Called when the scene needs to be painted
  void paintGL() noexcept;

  GLvoid resizeGL(GLint width, GLint height) noexcept;

  /// Called when the scene is to be initialised
  void initGL() noexcept;

  void handleKey(int _key);

  void animate(int *steps, int *frame, int textureID, std::string pathbegin, std::string pathend,int numberOfFrames, int speed);

private:
  GLuint m_rboDepthStencil[2];
  GLuint m_framebufferTex[2];
  GLuint m_framebuffer[2];
  GLuint m_prealiasedframebuffer[2];

  GLuint m_controls, m_title,m_testscreen;

  float xscale=1.0f;
  float yscale=1.0f;
  float brightness =1.0f;
  int channel =0;
  int maxChannel = 6;
  int tvon = 1;
  int tvstate = 0;
  int tvsteps=0;


  bool changechannel=false;
  bool frame = false;
  std::unique_ptr<ngl::Obj> m_anistropicMesh, m_matteMesh, m_screenMesh, m_screenQuad, m_wood;
  int step=0;

  /// Initialise a texture
  void initTexture(const GLuint& /*texUnit*/, GLuint &/*texId*/, const char */*filename*/);
};

#endif // TVScene_H
