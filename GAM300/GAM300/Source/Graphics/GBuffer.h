#ifndef GBUFFER_H
#define GBUFFER_H

#include "glslshader.h"

class gBuffer
{
public:

    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        //GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        //GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };/**/

    /*GBuffer();

    ~GBuffer();*/

    void Init(unsigned int WindowWidth, unsigned int WindowHeight);

    void BindForWriting();

    void BindForReading();
    
    void SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType);

    unsigned int gFBO;

    unsigned int gPosition, gNormal, gAlbedoSpec;

    unsigned int rbo;


private:

    /*GLuint m_fbo;
    GLuint m_textures[GBUFFER_NUM_TEXTURES];
    GLuint m_depthTexture;*/
};

#endif