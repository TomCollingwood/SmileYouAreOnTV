#ifndef TVScene_H
#define TVScene_H

#include <ngl/Obj.h>
#include <GLFW/glfw3.h>
#include <ngl/ShaderLib.h>
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
  GLuint m_rboDepthStencil[4]; //SSAO + 2FXAA
  GLuint m_FXAAframebufferTex[2];
  GLuint m_SSAOframebufferTex;
  GLuint m_PingPongframebuffer[2];
  GLuint m_SSAOframebuffer;
  GLuint m_controls, m_title,m_testscreen;

  GLuint m_SSAOBlurframebuffer;
  GLuint m_SSAOBlurframebufferTex;

  void passMatrices(GLuint shaderID);

  glm::mat4 M = glm::mat4(1.0f);
  glm::mat4 MVP, MV;
  glm::mat3 N;

  // Ambient Occlusion Kernel
  std::vector<glm::vec3> SSAOKernel;
  int SSAOKernelSize = 64;
  std::vector<glm::vec3> SSAONoise;
  GLuint m_gBufferRenderBuffer, m_gBuffer;
  GLfloat lerp(GLfloat a, GLfloat b, GLfloat f);

  GLuint gPosition, gNormal, gBuffer;




  // Television Variables
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
  void switchChannels(ngl::ShaderLib *shader, int otherframe);

  bool m_noiseon = true;
  bool m_vignette = true;
  bool m_showtangeants = false;
  bool m_showssao = false;
  bool m_ssaoonly = false;

  // Meshes
  std::unique_ptr<ngl::Obj> m_anistropicMesh, m_matteMesh, m_screenMesh, m_screenQuad, m_wood;

  // Frame
  int step=0;

  /// Initialise a texture
  void initTexture(const GLuint& /*texUnit*/, GLuint &/*texId*/, const char */*filename*/);
};

#endif // TVScene_H
