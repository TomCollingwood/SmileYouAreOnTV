#include "TVScene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Image.h>
#include <string>
#include <sstream>


#define SCREEN_DEF
TVScene::TVScene() : Scene() {
}

void TVScene::handleKey(int _key)
{
    switch(_key) {
    case GLFW_KEY_A: //exit the application
        channel--;
        if(channel<0) channel=0;
        else changechannel=true;
        break;
    case GLFW_KEY_D:
        channel++;
        if(channel>maxChannel) channel=maxChannel;
        else changechannel=true;
        break;
    case GLFW_KEY_S:
        if(tvon) tvstate = 1;
        else
        {
          tvstate = 2;
          tvsteps=70;
        }
        break;
    }
}

void TVScene::initGL() noexcept {
    // Fire up the NGL machinary (not doing this will make it crash)
    ngl::NGLInit::instance();

    m_height=720;
    m_width=960;

    // Set background colour
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);

    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);

    // Create and compile the vertex and fragment shader
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->loadShader("Anisotropic",
                       "shaders/AnisotropicVertex.glsl",
                       "shaders/AnisotropicFragment.glsl");
    shader->loadShader("Matte",
                       "shaders/MatteVertex.glsl",
                       "shaders/MatteFragment.glsl");
    shader->loadShader("ScreenQuad",
                       "shaders/ScreenQuadVertex.glsl",
                       "shaders/ScreenQuadFragment.glsl");
    shader->loadShader("TVScreen",
                       "shaders/TVScreenVertex.glsl",
                       "shaders/TVScreenFragment.glsl");
    shader->loadShader("Wood",
                       "shaders/WoodVertex.glsl",
                       "shaders/WoodFragment.glsl");


    (*shader)["Anisotropic"]->use();

    m_anistropicMesh.reset(new ngl::Obj("models/anistest.obj"));
    m_anistropicMesh->createVAO();

    //(*shader)["Matte"]->use();

    m_matteMesh.reset(new ngl::Obj("models/matte.obj"));
    m_matteMesh->createVAO();

    m_screenMesh.reset(new ngl::Obj("models/screen.obj"));
    m_screenMesh->createVAO();

    m_screenQuad.reset(new ngl::Obj("models/screenQUAD.obj"));
    m_screenQuad->createVAO();

    m_wood.reset(new ngl::Obj("models/wood.obj"));
    m_wood->createVAO();



    glEnable(GL_TEXTURE_2D);

    initTexture(0, m_controls, "images/controlstexture.jpg");
    initTexture(1, m_title, "images/titletexture.jpg");
    initTexture(2, m_testscreen, "images/freaky.jpg");

    // Generate FrameBuffers and Textures
    glGenFramebuffers(2, &m_framebuffer[0]);
    glGenFramebuffers(2, &m_prealiasedframebuffer[0]);
    glGenTextures(2, &m_framebufferTex[0]);
    glGenRenderbuffers(2, &m_rboDepthStencil[0]);

    // FIRST FRAMEBUFFER TEXTURE AND BUFFER
    glActiveTexture(GL_TEXTURE3);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer[0]);
    glBindTexture(GL_TEXTURE_2D, m_framebufferTex[0]);
    glTexImage2D(
           GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
       );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebufferTex[0], 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER,m_rboDepthStencil[0]);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[0]
    );

    // SECOND FRAMEBUFFER TEXTURE AND BUFFER
    glActiveTexture(GL_TEXTURE4);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer[1]);
    glTexImage2D(
           GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
       );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebufferTex[1], 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepthStencil[1]);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8,GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[1]
    );


    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    (*shader)["ScreenQuad"]->use();
    GLuint pid = shader->getProgramID("ScreenQuad");
    glUniform1i(glGetUniformLocation(pid, "texFramebuffer"), 3);
    glUniform1i(glGetUniformLocation(pid, "height"), m_height);
    glUniform1i(glGetUniformLocation(pid, "width"), m_width);

    (*shader)["TVScreen"]->use();
    pid = shader->getProgramID("TVScreen");
    glUniform1i(glGetUniformLocation(pid, "screenTexture"), 3);
    glUniform1f(glGetUniformLocation(pid, "_xscale"), xscale);
    glUniform1f(glGetUniformLocation(pid, "_yscale"), yscale);
    glUniform1f(glGetUniformLocation(pid, "_brightness"), brightness);
    glUniform1i(glGetUniformLocation(pid, "_tvon"), tvon);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup timer
    //t0 = std::chrono::high_resolution_clock::now();
}

GLvoid TVScene::resizeGL(GLint width, GLint height) noexcept {

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

    GLuint pid = shader->getProgramID("TVScreen");

    m_width = width; m_height = height;

    glUniform1i(glGetUniformLocation(pid, "height"), m_height);
    glUniform1i(glGetUniformLocation(pid, "width"), m_width);

    m_ratio = m_width / (float) m_height;
    glDeleteFramebuffers(1, &m_framebuffer[0]);
    glDeleteRenderbuffers(1, &m_rboDepthStencil[0]);
    glDeleteTextures(2,&m_framebufferTex[0]);

    // Generate FrameBuffers and Textures
    glGenFramebuffers(2, &m_framebuffer[0]);
    glGenTextures(2, &m_framebufferTex[0]);
    glGenRenderbuffers(2, &m_rboDepthStencil[0]);

    // FIRST FRAMEBUFFER TEXTURE AND BUFFER
    glActiveTexture(GL_TEXTURE3);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer[0]);
    glBindTexture(GL_TEXTURE_2D, m_framebufferTex[0]);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebufferTex[0], 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER,m_rboDepthStencil[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[0]
    );

    // SECOND FRAMEBUFFER TEXTURE AND BUFFER
    glActiveTexture(GL_TEXTURE4);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer[1]);
    glBindTexture(GL_TEXTURE_2D, m_framebufferTex[1]);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebufferTex[1], 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepthStencil[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[1]
    );


    pid = shader->getProgramID("ScreenQuad");
    glUniform1i(glGetUniformLocation(pid, "height"), m_height);
    glUniform1i(glGetUniformLocation(pid, "height"), m_width);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void TVScene::initTexture(const GLuint& texUnit, GLuint &texId, const char *filename) {
    // Set our active texture unit
    glActiveTexture(GL_TEXTURE0 + texUnit);

    // Load up the image using NGL routine
    ngl::Image img(filename);

    // Create storage for our new texture
    glGenTextures(1, &texId);

    // Bind the current texture
    glBindTexture(GL_TEXTURE_2D, texId);

    // Transfer image data onto the GPU using the teximage2D call
    glTexImage2D (
                GL_TEXTURE_2D,    // The target (in this case, which side of the cube)
                0,                // Level of mipmap to load
                img.format(),     // Internal format (number of colour components)
                img.width(),      // Width in pixels
                img.height(),     // Height in pixels
                0,                // Border
                GL_RGB,          // Format of the pixel data
                GL_UNSIGNED_BYTE, // Data type of pixel data
                img.getPixels()); // Pointer to image data in memory

    // Set up parameters for our texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void TVScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["Anisotropic"]->use();
    GLint pid = shader->getProgramID("Anisotropic");


    // Our MVP matrices
    glm::mat4 M = glm::mat4(1.0f);
    glm::mat4 MVP, MV;
    glm::mat3 N;

    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));
    MVP = m_P * MV;

    // Set this MVP on the GPU
    // ANISOTROPIC
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data

    (*shader)["Matte"]->use();
    // MATTTE
    pid = shader->getProgramID("Matte");
    glUniform1i(glGetUniformLocation(pid, "iGlobalTime"), step);
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data


    (*shader)["TVScreen"]->use();
    // MATTTE
    pid = shader->getProgramID("TVScreen");
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data

    // HANLDE THE TV OFF / ON / CHANNELS

    if(tvstate==1) // turning on
    {
      tvsteps++;
      //60 frames to squeeze y
      //30 frames to squeeze x
      float ouryscale=yscale + 300.0f*((float)glm::clamp(tvsteps,0,60))/60.0f;
      float ourxscale=xscale + 20.0f*(41-glm::clamp(tvsteps,41,70));

      brightness= 1.0f + 50.0f*((float)glm::clamp(tvsteps,0,60))/60.0f;

      glUniform1f(glGetUniformLocation(pid, "_xscale"), ourxscale);
      glUniform1f(glGetUniformLocation(pid, "_yscale"), ouryscale);
      glUniform1f(glGetUniformLocation(pid, "_brightness"), brightness);


      if(tvsteps==70)
      {
        tvsteps=0;
        tvstate=0;

        tvon=0;
        glUniform1i(glGetUniformLocation(pid, "_tvon"), tvon);
      }
    }
    else if(tvstate==2)
    {
      tvsteps--;
      tvon=1;
      glUniform1i(glGetUniformLocation(pid, "_tvon"), tvon);
      //60 frames to squeeze y
      //30 frames to squeeze x
      float ouryscale=yscale + 300.0f*((float)glm::clamp(tvsteps,0,60))/60.0f;
      float ourxscale=xscale + 20.0f*(41-glm::clamp(tvsteps,41,70));

      brightness= 1.0f + 50.0f*((float)glm::clamp(tvsteps,0,60))/60.0f;

      glUniform1f(glGetUniformLocation(pid, "_xscale"), ourxscale);
      glUniform1f(glGetUniformLocation(pid, "_yscale"), ouryscale);
      glUniform1f(glGetUniformLocation(pid, "_brightness"), brightness);


      if(tvsteps==0)
      {
        tvsteps=0;
        tvstate=0;
      }
    }



    step++;
    glUniform1i(glGetUniformLocation(pid, "iGlobalTime"), step);

    (*shader)["ScreenQuad"]->use();
    // MATTTE
    pid = shader->getProgramID("ScreenQuad");
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data


    (*shader)["Wood"]->use();
    // MATTTE
    pid = shader->getProgramID("Wood");
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data


    // TIMER

    //
    int whichFrame = 0;
    int otherFrame = 1;
    if(frame)
    {
      whichFrame=1;
      otherFrame=0;
      frame=false;
    }
    else
    {
      whichFrame=0;
      otherFrame=1;
      frame=true;
    }



    //DRAW TO FRAMEBUFFER
#ifdef SCREEN_DEF
  glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer[whichFrame]);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
  glEnable(GL_DEPTH_TEST);
  (*shader)["Anisotropic"]->use();
  m_anistropicMesh->draw();

  (*shader)["Matte"]->use();
  m_matteMesh->draw();

  (*shader)["Wood"]->use();
  m_wood->draw();

#ifdef SCREEN_DEF
  (*shader)["TVScreen"]->use();
  if(channel==2)
  {
  pid = shader->getProgramID("TVScreen");
  glUniform1i(glGetUniformLocation(pid, "screenTexture"), 3+otherFrame);
  m_screenMesh->draw();
  }

  // DRAW TO SCREEN
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


   (*shader)["Anisotropic"]->use();
   m_anistropicMesh->draw();

   (*shader)["Matte"]->use();
   m_matteMesh->draw();

   (*shader)["Wood"]->use();
   m_wood->draw();

   (*shader)["TVScreen"]->use();

   //-------------------------------------------TV CHANNELS------------------------------
   pid = shader->getProgramID("TVScreen");
   if(channel==2)
   {
      glUniform1i(glGetUniformLocation(pid, "_camera"), 1);
      glUniform1i(glGetUniformLocation(pid, "screenTexture"), 3+otherFrame);
   }
   else if(channel==0)
   {
      glUniform1i(glGetUniformLocation(pid, "_camera"), 0);
       glUniform1i(glGetUniformLocation(pid, "screenTexture"), 0);
   }
   else if(channel==1)
   {
     glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

       glUniform1i(glGetUniformLocation(pid, "screenTexture"), 1);
   }
   else if(channel==3)
   {
     glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

     static int spongesteps =0;
     static int spongeframe =1;
     animate(&spongesteps,&spongeframe,2,"images/spongebob/sponge",".jpg",5,5);
     glUniform1i(glGetUniformLocation(pid, "screenTexture"), 2);
   }
   else if(channel==4)
   {
     glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

     static int spongesteps2 =0;
     static int spongeframe2 =1;
     animate(&spongesteps2,&spongeframe2,2,"images/bogart/frame_","_delay-0.1s.jpg",66,4);
     glUniform1i(glGetUniformLocation(pid, "screenTexture"), 2);
   }
   else if(channel==5)
   {
     glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

     static int spongesteps3 =0;
     static int spongeframe3 =1;
     animate(&spongesteps3,&spongeframe3,2,"images/homealone/homealone",".jpg",2414,2);
     glUniform1i(glGetUniformLocation(pid, "screenTexture"), 2);
   }
   else if(channel==6)
   {
     glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

     initTexture(2, m_testscreen, "images/testscreen.jpg");
     glUniform1i(glGetUniformLocation(pid, "screenTexture"), 2);
   }
   m_screenMesh->draw();

#endif

}

void TVScene::animate(int *steps, int *frame, int textureID, std::string pathbegin, std::string pathend,int numberOfFrames,int speed)
{
  (*steps)++;
  if((*steps)%speed==0)
  {
    (*frame)++;
    if((*frame)>numberOfFrames) (*frame)=0;
    std::ostringstream sstream;
    sstream << pathbegin.c_str() << (*frame)<<pathend.c_str();
    std::string query = sstream.str();
    initTexture(textureID, m_testscreen, query.c_str());
  }

}
