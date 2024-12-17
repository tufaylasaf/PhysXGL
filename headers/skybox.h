#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "model.h"

class Skybox
{
public:
    unsigned int hdrTexture;
    unsigned int envCubemap;
    unsigned int irradianceMap;
    unsigned int prefilterMap;
    unsigned int brdfLUTTexture;

    Skybox(const std::string &hdrPath);

    void Init(Camera &camera);
    void Render(Camera &camera);

private:
    unsigned int captureFBO, captureRBO;

    Shader equirectangularToCubemapShader;
    Shader irradianceShader;
    Shader prefilterShader;
    Shader brdfShader;
    Shader backgroundShader;

    glm::mat4 captureProjection;
    glm::mat4 captureViews[6];

    Model cubeMap; // Model representing the skybox cube

    void LoadHDR(const std::string &hdrPath);
    void SetupCubemap(Camera &camera);
    void SetupIrradiance(Camera &camera);
    void SetupPrefilter(Camera &camera);
    void SetupBRDF(Camera &camera);
    void RenderQuad();
};

#endif
