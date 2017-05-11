#include "TVScene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Image.h>
#include <string>
#include <sstream>
#include <random>

#define SCREEN_DEF
TVScene::TVScene() : Scene() {
}

GLfloat TVScene::lerp(GLfloat a, GLfloat b, GLfloat f)
{
    return a + f * (b - a);
}

void TVScene::passMatrices(GLuint shaderID)
{
  glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(MVP)); // a raw pointer to the data
  glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(MV)); // a raw pointer to the data
  glUniformMatrix3fv(glGetUniformLocation(shaderID, "N"), //location of uniform
                     1, // how many matrices to transfer
                     true, // whether to transpose matrix
                     glm::value_ptr(N)); // a raw pointer to the data
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

    int stuff;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &stuff);

    std::cout<<"HEYY YOOUU"<<stuff<<std::endl;

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
    shader->loadShader("GBuffer",
                       "shaders/GBufferVertex.glsl",
                       "shaders/GBufferFragment.glsl");
    shader->loadShader("SSAO",
                       "shaders/SSAOVertex.glsl",
                       "shaders/SSAOFragment.glsl");
    shader->loadShader("SSAOBlur",
                       "shaders/SSAOBlurVertex.glsl",
                       "shaders/SSAOBlurFragment.glsl");

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

    initTexture(5, m_controls, "images/controlstexture.jpg");
    initTexture(6, m_title, "images/titletexture.jpg");
    initTexture(7, m_testscreen, "images/freaky.jpg");

    // Generate FrameBuffers and Textures
    glGenFramebuffers(1, &m_SSAOframebuffer);
    glGenTextures(1, &m_SSAOframebufferTex);

    glGenFramebuffers(1, &m_SSAOBlurframebuffer);
    glGenTextures(1, &m_SSAOBlurframebufferTex);

    glGenFramebuffers(2, &m_FXAAframebuffer[0]);
    glGenTextures(2, &m_FXAAframebufferTex[0]);
    glGenRenderbuffers(4, &m_rboDepthStencil[0]);

    // FIRST FRAMEBUFFER TEXTURE AND BUFFER
    glActiveTexture(GL_TEXTURE3);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FXAAframebuffer[0]);
    glBindTexture(GL_TEXTURE_2D, m_FXAAframebufferTex[0]);
    glTexImage2D(
           GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
       );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FXAAframebufferTex[0], 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER,m_rboDepthStencil[0]);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[0]
    );

    // SECOND FRAMEBUFFER TEXTURE AND BUFFER
    glActiveTexture(GL_TEXTURE4);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FXAAframebuffer[1]);
    glTexImage2D(
           GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
       );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FXAAframebufferTex[1], 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepthStencil[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[1]
    );

    // SSAO

    glActiveTexture(GL_TEXTURE8);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOframebuffer);
    glBindTexture(GL_TEXTURE_2D, m_SSAOframebufferTex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOframebufferTex, 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepthStencil[2]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[2]
    );

    //SSAO BLUR
    glActiveTexture(GL_TEXTURE9);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurframebuffer);
    glBindTexture(GL_TEXTURE_2D, m_SSAOBlurframebufferTex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOBlurframebufferTex, 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepthStencil[3]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[3]
    );


     GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
            printf("FB error, status: 0x%x\n", Status);
    }



    // Generate Ambient Occlusion Kernel
    // Adapted from : https://learnopengl.com/#!Advanced-Lighting/SSAO
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
    std::default_random_engine generator;

    for (GLuint i = 0; i < SSAOKernelSize; ++i)
    {
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator)
        );
        sample  = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = GLfloat(i) / (float)SSAOKernelSize;
        scale   = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        SSAOKernel.push_back(sample);
    }

    for (GLuint i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f);
        SSAONoise.push_back(noise);
    }

    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    glGenRenderbuffers(1, &m_gBufferRenderBuffer);

    glActiveTexture(GL_TEXTURE0);
    GLuint noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &SSAONoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // - Position color buffer
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(
           GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
       );    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // - Normal color buffer
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(
           GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
       );    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);


    // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    glBindRenderbuffer(GL_RENDERBUFFER,m_gBufferRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_gBufferRenderBuffer
    );




    (*shader)["SSAO"]->use();
    GLuint pid = shader->getProgramID("SSAO");
    glUniform1i(glGetUniformLocation(pid, "texNoise"), 0);
    glUniform1i(glGetUniformLocation(pid, "gNormal"), 2);
    glUniform1i(glGetUniformLocation(pid, "gPosition"), 1);
    glUniform3fv(glGetUniformLocation(pid, "samples"), SSAOKernelSize, (float *)&SSAOKernel[0]);
    glUniform1i(glGetUniformLocation(pid, "height"), m_height);
    glUniform1i(glGetUniformLocation(pid, "width"), m_width);

    (*shader)["SSAOBlur"]->use();
    pid = shader->getProgramID("SSAOBlur");
    glUniform1i(glGetUniformLocation(pid, "ssaoInput"), 8);

    (*shader)["ScreenQuad"]->use();
    pid = shader->getProgramID("ScreenQuad");
    glUniform1i(glGetUniformLocation(pid, "texFramebuffer"), 3);
    glUniform1i(glGetUniformLocation(pid, "texSSAO"), 9);
    glUniform1i(glGetUniformLocation(pid, "noiseTexture"), 5);
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

}

GLvoid TVScene::resizeGL(GLint width, GLint height) noexcept {

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

    GLuint pid = shader->getProgramID("TVScreen");

    m_width = width; m_height = height;

    glUniform1i(glGetUniformLocation(pid, "height"), m_height);
    glUniform1i(glGetUniformLocation(pid, "width"), m_width);

    m_ratio = m_width / (float) m_height;
    glDeleteFramebuffers(2, &m_FXAAframebuffer[0]);
    glDeleteFramebuffers(1, &m_SSAOframebuffer);
    glDeleteFramebuffers(1, &m_SSAOBlurframebuffer);
    glDeleteFramebuffers(1, &m_gBuffer);
    glDeleteRenderbuffers(4, &m_rboDepthStencil[0]);
    glDeleteRenderbuffers(1, &m_gBufferRenderBuffer);
    glDeleteTextures(2,&m_FXAAframebufferTex[0]);
    glDeleteTextures(1,&gNormal);
    glDeleteTextures(1,&gPosition);
    glDeleteTextures(2,&m_SSAOBlurframebufferTex);

    // Generate FrameBuffers and Textures
    glGenFramebuffers(2, &m_FXAAframebuffer[0]);
    glGenTextures(2, &m_FXAAframebufferTex[0]);
    glGenRenderbuffers(4, &m_rboDepthStencil[0]);
    glGenFramebuffers(1, &m_SSAOframebuffer);
    glGenTextures(1, &m_SSAOframebufferTex);
    glGenFramebuffers(1, &m_SSAOBlurframebuffer);
    glGenTextures(1, &m_SSAOBlurframebufferTex);

    // FIRST FRAMEBUFFER TEXTURE AND BUFFER
    glActiveTexture(GL_TEXTURE3);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FXAAframebuffer[0]);
    glBindTexture(GL_TEXTURE_2D, m_FXAAframebufferTex[0]);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FXAAframebufferTex[0], 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER,m_rboDepthStencil[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[0]
    );

    // SECOND FRAMEBUFFER TEXTURE AND BUFFER
    glActiveTexture(GL_TEXTURE4);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FXAAframebuffer[1]);
    glBindTexture(GL_TEXTURE_2D, m_FXAAframebufferTex[1]);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FXAAframebufferTex[1], 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepthStencil[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[1]
    );


    // SSAO


    glActiveTexture(GL_TEXTURE8);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOframebuffer);
    glBindTexture(GL_TEXTURE_2D, m_SSAOframebufferTex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOframebufferTex, 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepthStencil[2]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[2]
    );

    //SSAO BLUR
    glActiveTexture(GL_TEXTURE9);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurframebuffer);
    glBindTexture(GL_TEXTURE_2D, m_SSAOBlurframebufferTex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOBlurframebufferTex, 0
    );

    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepthStencil[3]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthStencil[3]
    );

    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    glGenRenderbuffers(1, &m_gBufferRenderBuffer);


    // - Position color buffer
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(
           GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
       );    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // - Normal color buffer
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(
           GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
       );    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);


    // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    glBindRenderbuffer(GL_RENDERBUFFER,m_gBufferRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_gBufferRenderBuffer
    );

    pid = shader->getProgramID("ScreenQuad");
    glUniform1i(glGetUniformLocation(pid, "height"), m_height);
    glUniform1i(glGetUniformLocation(pid, "width"), m_width);

    pid = shader->getProgramID("SSAO");
    glUniform1i(glGetUniformLocation(pid, "height"), m_height);
    glUniform1i(glGetUniformLocation(pid, "width"), m_width);

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


    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));
    MVP = m_P * MV;

    //--------------------------------PASS MATRICES---------------------------------

    (*shader)["Anisotropic"]->use();
    passMatrices(shader->getProgramID("Anisotropic"));

    (*shader)["Matte"]->use();
    passMatrices(shader->getProgramID("Matte"));

    (*shader)["ScreenQuad"]->use();
    passMatrices(shader->getProgramID("ScreenQuad"));

    (*shader)["Wood"]->use();
    passMatrices(shader->getProgramID("Wood"));

    (*shader)["GBuffer"]->use();
    passMatrices(shader->getProgramID("Matte"));

    (*shader)["SSAO"]->use();
    glUniformMatrix4fv(glGetUniformLocation(shader->getProgramID("SSAO"), "projection"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(m_P)); // a raw pointer to the data

    (*shader)["TVScreen"]->use();
    GLuint pid = shader->getProgramID("TVScreen");
    glUniform1i(glGetUniformLocation(pid, "iGlobalTime"), step);
    passMatrices(shader->getProgramID("TVScreen"));

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

    // TIMER
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



  // gBuffer (depth + normals)
  glBindFramebuffer(GL_FRAMEBUFFER,m_gBuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, drawBuffers);
  (*shader)["GBuffer"]->use();
  m_anistropicMesh->draw();
  m_matteMesh->draw();
  m_wood->draw();
  m_screenMesh->draw();

  // SSAO
  glBindFramebuffer(GL_FRAMEBUFFER,m_SSAOframebuffer);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  (*shader)["SSAO"]->use();
  m_screenQuad->draw();

  // SSAO Blur
  glBindFramebuffer(GL_FRAMEBUFFER,0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glActiveTexture(GL_TEXTURE8);
  glBindTexture(GL_TEXTURE_2D, m_SSAOframebufferTex);

  (*shader)["SSAOBlur"]->use();
  //glUniform1i(glGetUniformLocation(shader->getProgramID("SSAOBlur"), "texFramebuffer"), 8);
  m_screenQuad->draw();
/*


  glBindFramebuffer(GL_FRAMEBUFFER, m_FXAAframebuffer[whichFrame]);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  (*shader)["Anisotropic"]->use();
  m_anistropicMesh->draw();

  (*shader)["Matte"]->use();
  m_matteMesh->draw();

  (*shader)["Wood"]->use();
  m_wood->draw();

  (*shader)["TVScreen"]->use();
  switchChannels(shader,otherFrame);
  m_screenMesh->draw();


  // DRAW TO SCREEN
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  (*shader)["ScreenQuad"]->use();
  pid = shader->getProgramID("ScreenQuad");
  glUniform1i(glGetUniformLocation(pid, "texFramebuffer"), 3+whichFrame);
  m_screenQuad->draw();



  // */



}

void TVScene::switchChannels(ngl::ShaderLib *shader, int otherFrame)
{
  GLuint pid = shader->getProgramID("TVScreen");
  if(channel==2)
  {
     glUniform1i(glGetUniformLocation(pid, "_camera"), 1);
     glUniform1i(glGetUniformLocation(pid, "screenTexture"), 3+otherFrame);
  }
  else if(channel==0)
  {
     glUniform1i(glGetUniformLocation(pid, "_camera"), 0);
      glUniform1i(glGetUniformLocation(pid, "screenTexture"), 5);
  }
  else if(channel==1)
  {
    glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

      glUniform1i(glGetUniformLocation(pid, "screenTexture"), 6);
  }
  else if(channel==3)
  {
    glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

    static int spongesteps =0;
    static int spongeframe =1;
    animate(&spongesteps,&spongeframe,7,"images/spongebob/sponge",".jpg",5,5);
    glUniform1i(glGetUniformLocation(pid, "screenTexture"), 7);
  }
  else if(channel==4)
  {
    glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

    static int spongesteps2 =0;
    static int spongeframe2 =1;
    animate(&spongesteps2,&spongeframe2,7,"images/bogart/frame_","_delay-0.1s.jpg",66,4);
    glUniform1i(glGetUniformLocation(pid, "screenTexture"), 7);
  }
  else if(channel==5)
  {
    glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

    static int spongesteps3 =0;
    static int spongeframe3 =1;
    animate(&spongesteps3,&spongeframe3,7,"images/homealone/homealone",".jpg",2414,2);
    glUniform1i(glGetUniformLocation(pid, "screenTexture"), 7);
  }
  else if(channel==6)
  {
    glUniform1i(glGetUniformLocation(pid, "_camera"), 0);

    initTexture(7, m_testscreen, "images/testscreen.jpg");
    glUniform1i(glGetUniformLocation(pid, "screenTexture"), 7);
  }
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
