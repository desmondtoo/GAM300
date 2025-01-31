#include <Precompiled.h>
#include "ParticleRenderer.h"
#include "Graphics/MeshManager.h"
//#include "ParticleSystem/ParticleSystem.h"
#include "Scene/SceneManager.h"
#include "GraphicsHeaders.h"
#include "Texture/TextureManager.h"


void ParticleRenderer::Init() {
    SetupInstancedQuad(); // for 2D particles
    SetupInstancedCylinder(); // for cylinder trails
}

void ParticleRenderer::Update(float dt) {
    particleSRT.clear(); // @kk not all entity should use the same container
    trailSRT.clear();
    particleLifetimes.clear();
    Scene& currentScene = SceneManager::Instance().GetCurrentScene();
    glm::mat4 _2dmtx(1.f);
    glm::vec3 camTranslate(1.f);
    glm::vec3 camUp(0.f, 1.f, 0.f);
    int particleCounter{ 0 };

    for (Camera& cam : currentScene.GetArray<Camera>()) {
        Transform& camTransform = currentScene.Get<Transform>(cam);
        //_2dmtx = glm::inverse(camTransform.GetWorldMatrix());
        camTranslate = camTransform.GetGlobalTranslation();
        camUp = cam.GetUpVec();
    }/**/

    for (ParticleComponent& particleComponent : currentScene.GetArray<ParticleComponent>()) {
        if (!currentScene.IsActive(particleComponent))
            continue;
        Entity& entity = currentScene.Get<Entity>(particleComponent);
        Transform& transform = currentScene.Get<Transform>(particleComponent);
        if (!currentScene.IsActive(entity))
            continue;
        particleComponent.trailCounter = 0;
        for (int i = 0; i < particleComponent.numParticles_; ++i) {
            //particleTransform.GetTranslation() += particleComponent.particles_[i].position;
            if (i >= particleComponent.particles_.size())
            {
                break;
            }
            glm::mat4 scale = glm::mat4(1.f) * particleComponent.particles_[i].scale;
            scale[3] = glm::vec4(0, 0, 0, 1);

            glm::mat4 rotate = glm::mat4(1.f);
            //calculate rotation if 2D
            if (particleComponent.is2D) {
                // look vector
                glm::vec3 forward = glm::normalize(camTranslate - particleComponent.particles_[i].position);
                glm::vec3 newz = glm::normalize(glm::cross(forward, camUp));

                rotate = glm::mat4(
                glm::vec4(newz, 0.f),
                glm::vec4(camUp, 0.f),
                glm::vec4(forward, 0.f),
                glm::vec4(0.f, 0.f, 0.f, 1.f)
                );
            }
            glm::mat4 translate;
            if (particleComponent.isLocalSpace) {
                translate = glm::mat4(
                    glm::vec4(1, 0, 0, 0),
                    glm::vec4(0, 1, 0, 0),
                    glm::vec4(0, 0, 1, 0),
                    glm::vec4(particleComponent.particles_[i].position + transform.GetGlobalTranslation(), 1));/**/
            }
            else {
                translate = glm::mat4(
                    glm::vec4(1, 0, 0, 0),
                    glm::vec4(0, 1, 0, 0),
                    glm::vec4(0, 0, 1, 0),
                    glm::vec4(particleComponent.particles_[i].position, 1));/**/
            }

            

            //particleSRT.emplace_back(scale * rotate * translate);
            glm::mat4 srt = translate * rotate * scale;

            // Color Fading
            if (particleComponent.fadeToColor)
            {
                particleLifetimes.emplace_back(particleComponent.particles_[i].lifetime, particleComponent.particles_[i].lifetimeInitial);
            }
            else
            {
                particleLifetimes.emplace_back(0.f,0.f);
            }

            particleSRT.emplace_back(srt);

            float thicc = particleComponent.trailThiccness / 10.f;

            // update trail
            if (!particleComponent.is2D) {

                for (unsigned int j = 1; j < particleComponent.particles_[i].trails.count; ++j) {
                    glm::vec3 trailVector = particleComponent.particles_[i].trails.pos[j] - particleComponent.particles_[i].trails.pos[j - 1u];
                    float trailScale = glm::length(trailVector);
                    glm::mat4 trailScaleMatrix = glm::mat4(
                        glm::vec4(thicc, 0, 0, 0),
                        //glm::vec4(0, 1, 0, 0),
                        glm::vec4(0, trailScale, 0, 0),
                        glm::vec4(0, 0, thicc, 0),
                        glm::vec4(0, 0, 0, 1));
                    trailVector = glm::normalize(trailVector);
                    glm::vec3 newX = glm::cross(trailVector, glm::vec3(0.0f, 1.0f, 0.0f));
                    if (glm::length(newX) == 0) {
                        newX = vec3(.5f, 0.f, 0.f);
                    }
                    glm::vec3 newZ = glm::cross(newX, trailVector);
                    glm::mat4 trailRotationMatrix = glm::mat4(
                        glm::vec4(newX, 0),
                        glm::vec4(trailVector, 0),
                        glm::vec4(newZ, 0),
                        glm::vec4(0, 0, 0, 1));
                    glm::mat4 trailTranslateMatrix = glm::mat4(
                        glm::vec4(1, 0, 0, 0),
                        glm::vec4(0, 1, 0, 0),
                        glm::vec4(0, 0, 1, 0),
                        glm::vec4(particleComponent.particles_[i].trails.pos[j - 1], 1));
                    //trailSRT.emplace_back(trailScaleMatrix * trailRotationMatrix * trailTranslateMatrix);
                    trailSRT.emplace_back(trailTranslateMatrix * trailRotationMatrix * trailScaleMatrix);
                    particleComponent.trailCounter++;
                }
            }
        }

        //sort particle
        if (particleComponent.is2D && !particleSRT.size() > 1) {
            std::sort(particleSRT.begin() + particleCounter, particleSRT.end(),
                [&](const glm::mat4& particle1, const glm::mat4& particle2) {
                    return compareParticles(particle1, particle2, camTranslate);
                });
        }
        particleCounter += particleComponent.particles_.size();
    }
    
}

// Define a custom comparator function for sorting particles based on distance to camera
bool ParticleRenderer::compareParticles(const glm::mat4& particle1, const glm::mat4& particle2, const glm::vec3& cameraPosition) {
    // Extract translation from the transformation matrices
    glm::vec3 position1 = glm::vec3(particle1[3]);
    glm::vec3 position2 = glm::vec3(particle2[3]);

    // Calculate squared distances from particles to camera
    float distance1 = glm::length2(position1 - cameraPosition);
    float distance2 = glm::length2(position2 - cameraPosition);

    // Sort particles based on distance
    return distance1 > distance2; // Sort in descending order (furthest first)
}

void ParticleRenderer::Draw(BaseCamera& _camera) {
    Scene& currentScene = SceneManager::Instance().GetCurrentScene();
    int counter = 0;
    int generalTrailCounter = 0;
    for (ParticleComponent& particleComponent : currentScene.GetArray<ParticleComponent>()) {
        if (!currentScene.IsActive(particleComponent))
            continue;
        Entity& entity = currentScene.Get<Entity>(particleComponent);
        if (!currentScene.IsActive(entity))
            continue;
        if (particleComponent.is2D) {
            counter += particleComponent.particles_.size();
            generalTrailCounter += particleComponent.trailCounter;
            continue;
        }
        if (particleComponent.numParticles_ == 0)
            continue;
        if (particleSRT.size() == 0)
            continue;

        GLuint vao = MESHMANAGER.DereferencingMesh(particleComponent.meshID)->vaoID;
        GLenum prim = MESHMANAGER.DereferencingMesh(particleComponent.meshID)->prim; // for now particles are all cubes
        InstanceProperties& prop = MESHMANAGER.instanceProperties->find(vao)->second;
        Material_instance currMatInstance = MaterialSystem::Instance().getMaterialInstance(particleComponent.materialGUID);

        hasTexture = currMatInstance.albedoTexture.longInt[0] ? true : false;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureManager.GetTexture(currMatInstance.albedoTexture));

        glBindBuffer(GL_ARRAY_BUFFER, prop.entitySRTbuffer);

        glBufferSubData(GL_ARRAY_BUFFER, 0, (particleComponent.particles_.size()) * sizeof(glm::mat4), particleSRT.data() + counter);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        GLSLShader shader = SHADER.GetShader(SHADERTYPE::PARTICLES);
        shader.Use();

        glBindBuffer(GL_ARRAY_BUFFER, prop.ShininessBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (particleComponent.particles_.size()) * sizeof(glm::vec2), particleLifetimes.data() + counter);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        GLint fadeToColor =
            glGetUniformLocation(shader.GetHandle(), "fadeToColor");
        GLint colorToFadeTo =
            glGetUniformLocation(shader.GetHandle(), "colorToFadeTo");

        GLint maxLifetime =
            glGetUniformLocation(shader.GetHandle(), "maxLifetime");

        GLint perspective =
            glGetUniformLocation(shader.GetHandle(), "persp_projection");
        GLint view =
            glGetUniformLocation(shader.GetHandle(), "View");
        GLint campos =
            glGetUniformLocation(shader.GetHandle(), "camPos");
        GLint AoConstant =
            glGetUniformLocation(shader.GetHandle(), "AoConstant");
        GLint EmissionConstant =
            glGetUniformLocation(shader.GetHandle(), "EmissionConstant");
        GLint Colour =
            glGetUniformLocation(shader.GetHandle(), "frag_Albedo");

        glUniform1f(fadeToColor, particleComponent.fadeToColor);
        
        glUniform3f(colorToFadeTo, particleComponent.colorToFadeTowards.x, 
            particleComponent.colorToFadeTowards.y, particleComponent.colorToFadeTowards.z);

        glUniform1f(maxLifetime, particleComponent.particleLifetime_);

        glUniform1f(glGetUniformLocation(shader.GetHandle(), "gammaCorrection"), RENDERER.getGamma());

        glUniformMatrix4fv(perspective, 1, GL_FALSE,
            glm::value_ptr(_camera.GetProjMatrix()));
        glUniformMatrix4fv(view, 1, GL_FALSE,
            glm::value_ptr(_camera.GetViewMatrix()));
        glUniform4fv(Colour, 1, glm::value_ptr(glm::vec4(currMatInstance.albedoColour)));
        glUniform1f(AoConstant, currMatInstance.aoConstant);
        glUniform1f(EmissionConstant, currMatInstance.emissionConstant);
        glUniform1f(glGetUniformLocation(shader.GetHandle(), "ambience_multiplier"), RENDERER.getAmbient());
        glUniform1f(glGetUniformLocation(shader.GetHandle(), "bloomThreshold"), RENDERER.GetBloomThreshold());
        GLint boolean1 = glGetUniformLocation(shader.GetHandle(), "hasTexture");
        glUniform1i(boolean1, hasTexture);
        
        glBindVertexArray(vao);
        glDrawElementsInstanced(prim, prop.drawCount, GL_UNSIGNED_INT, 0, particleComponent.particles_.size());
        
        glBindVertexArray(0);

        shader.UnUse();

        if (particleComponent.trailCounter != 0) {
            GLSLShader trailshader = SHADER.GetShader(SHADERTYPE::TRAILS);
            glBindBuffer(GL_ARRAY_BUFFER, cylSRTBuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, particleComponent.trailCounter * sizeof(glm::mat4), trailSRT.data() + generalTrailCounter);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            trailshader.Use();
            perspective = glGetUniformLocation(trailshader.GetHandle(), "persp_projection");
            view = glGetUniformLocation(trailshader.GetHandle(), "View");
            GLuint trailColor = glGetUniformLocation(trailshader.GetHandle(), "trailColor");
            glUniform3fv(trailColor, 1, glm::value_ptr(glm::vec3(particleComponent.trailColor)));
            glUniformMatrix4fv(perspective, 1, GL_FALSE,
                glm::value_ptr(_camera.GetProjMatrix()));
            glUniformMatrix4fv(view, 1, GL_FALSE,
                glm::value_ptr(_camera.GetViewMatrix()));
            glBindVertexArray(cylVAO);
            glDrawElementsInstanced(GL_TRIANGLE_STRIP, (GLsizei)cylsize, GL_UNSIGNED_INT, 0, (GLsizei)particleComponent.trailCounter);
            glBindVertexArray(0);

            trailshader.UnUse();
            generalTrailCounter += particleComponent.trailCounter;

        }
        
        counter += particleComponent.particles_.size();
    }
}

void ParticleRenderer::Draw2D(BaseCamera& _camera) {
    Scene& currentScene = SceneManager::Instance().GetCurrentScene();
    int counter = 0;
    for (ParticleComponent& particleComponent : currentScene.GetArray<ParticleComponent>()) {
        if (!currentScene.IsActive(particleComponent))
            continue;
        Entity& entity = currentScene.Get<Entity>(particleComponent);
        if (!currentScene.IsActive(entity))
            continue;
        if (!particleComponent.is2D) {
            counter += particleComponent.particles_.size();
            continue;
        }
        if (particleComponent.numParticles_ == 0)
            continue;
        if (particleSRT.size() == 0)
            continue;
        

        GLuint vao = MESHMANAGER.DereferencingMesh(particleComponent.meshID)->vaoID;
        GLenum prim = MESHMANAGER.DereferencingMesh(particleComponent.meshID)->prim; // for now particles are all cubes
        InstanceProperties& prop = MESHMANAGER.instanceProperties->find(vao)->second;
        Material_instance currMatInstance = MaterialSystem::Instance().getMaterialInstance(particleComponent.materialGUID);

        hasTexture = currMatInstance.albedoTexture.longInt[0] ? true : false;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureManager.GetTexture(currMatInstance.albedoTexture));

        glBindBuffer(GL_ARRAY_BUFFER, quadSRTBuffer);
        
        glBufferSubData(GL_ARRAY_BUFFER, 0, (particleComponent.particles_.size()) * sizeof(glm::mat4), particleSRT.data() + counter);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, lifetimeFor2D);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (particleComponent.particles_.size()) * sizeof(glm::vec2), particleLifetimes.data() + counter);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        GLSLShader shader = SHADER.GetShader(SHADERTYPE::PARTICLES2D);
        shader.Use();

        GLint fadeToColor =
            glGetUniformLocation(shader.GetHandle(), "fadeToColor");
        GLint colorToFadeTo =
            glGetUniformLocation(shader.GetHandle(), "colorToFadeTo");

        GLint maxLifetime =
            glGetUniformLocation(shader.GetHandle(), "maxLifetime");

        GLint AoConstant =
            glGetUniformLocation(shader.GetHandle(), "AoConstant");
        GLint EmissionConstant =
            glGetUniformLocation(shader.GetHandle(), "EmissionConstant");

        GLint perspective =
            glGetUniformLocation(shader.GetHandle(), "persp_projection");
        GLint view =
            glGetUniformLocation(shader.GetHandle(), "View");
        GLint Colour =
            glGetUniformLocation(shader.GetHandle(), "frag_Albedo");

        glUniform1f(glGetUniformLocation(shader.GetHandle(), "gammaCorrection"), RENDERER.getGamma());

        glUniform1f(fadeToColor, particleComponent.fadeToColor);

        glUniform3f(colorToFadeTo, particleComponent.colorToFadeTowards.x,
            particleComponent.colorToFadeTowards.y, particleComponent.colorToFadeTowards.z);

        glUniform1f(maxLifetime, particleComponent.particleLifetime_);

        glUniform1f(AoConstant, currMatInstance.aoConstant);
        glUniform1f(EmissionConstant, currMatInstance.emissionConstant);

        glUniformMatrix4fv(perspective, 1, GL_FALSE,
            glm::value_ptr(_camera.GetProjMatrix()));
        glUniformMatrix4fv(view, 1, GL_FALSE,
            glm::value_ptr(_camera.GetViewMatrix()));
        glUniform4fv(Colour, 1, glm::value_ptr(glm::vec4(currMatInstance.albedoColour)));
        GLint boolean1 = glGetUniformLocation(shader.GetHandle(), "hasTexture");
        glUniform1f(glGetUniformLocation(shader.GetHandle(), "ambience_multiplier"), RENDERER.getAmbient());
        glUniform1f(glGetUniformLocation(shader.GetHandle(), "bloomThreshold"), RENDERER.GetBloomThreshold());
        glUniform1i(boolean1, hasTexture);

        glBindVertexArray(quadVAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particleComponent.particles_.size());
        
        
        glBindVertexArray(0);

        counter += particleComponent.particles_.size();

    }
}
void ParticleRenderer::SetupInstancedCylinder() {
    float radius = 0.5f;  // Adjust as needed
    float height = 1.0f;  // Adjust as needed
    int slices = 6;      // Number of segments around the cylinder

    std::vector<glm::vec3> vertices;

    // Bottom base center vertex
    vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

    // Vertices around the bottom base
    for (int i = 0; i < slices; i++) {
        float angle = 2 * 3.1415926f * i / slices;
        vertices.push_back(glm::vec3(radius * cos(angle), 0.0f, radius * sin(angle)));
    }

    // Top base center vertex
    vertices.push_back(glm::vec3(0.0f, height, 0.0f));

    // Vertices around the top base (same as bottom, but at height)
    for (int i = 0; i < slices; i++) {
        vertices.push_back(vertices[i + 1] + glm::vec3(0.0f, height, 0.0f));
    }
    // Indices for drawing the cylinder using triangle strips
    std::vector<unsigned int> indices;

    // Bottom base
    for (int i = 0; i < slices; i++) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }
    indices.push_back(0);
    indices.push_back(slices + 1);
    indices.push_back(1);

    // Top base
    for (int i = 0; i < slices; i++) {
        indices.push_back(slices + 2);
        indices.push_back(slices + i + 3);
        indices.push_back(slices + i + 2);
    }
    indices.push_back(slices + 2);
    indices.push_back(slices * 2 + 2);
    indices.push_back(slices + 3);

    // Side
    for (int i = 0; i < slices; i++) {
        indices.push_back(i + 1);
        indices.push_back(slices + i + 2);
        indices.push_back(slices + i + 3);
        indices.push_back(i + 1);
        indices.push_back(slices + i + 3);
        indices.push_back(i + 2);
    }
    cylsize = (GLuint)indices.size();

    GLuint VBO, ebo;
    glGenVertexArrays(1, &cylVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &ebo);

    glBindVertexArray(cylVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylsize * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0); // unbind vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo
    //GL_TRIANGLE_STRIP


    // instance rendering setup
    glGenBuffers(1, &cylSRTBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cylSRTBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::mat4), trailSRT.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(cylVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cylSRTBuffer);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);
    glVertexAttribDivisor(9, 1);
    glBindVertexArray(0);




}

void ParticleRenderer::SetupInstancedQuad() {
    float quadVertices[] = {
        // positions            // texture Coords
        -.5f, -.5f,	0.f,    	0.f, 1.f,
         .5f, -.5f,	0.f,    	1.f, 1.f,
         .5f,  .5f,	0.f,    	1.f, 0.f,
        -.5f,  .5f,	0.f,    	0.f, 0.f
    };
    unsigned int indices[] = {
        0,1,2,
        2,3,0 
    };
    GLuint quadVBO, ebo;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &ebo);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0); // unbind vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo
    //GL_TRIANGLE_STRIP


    // instance rendering setup
    glGenBuffers(1, &quadSRTBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadSRTBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::mat4), particleSRT.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadSRTBuffer);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);
    glVertexAttribDivisor(9, 1);
    glBindVertexArray(0);

    
    glGenBuffers(1, &lifetimeFor2D);
    glBindBuffer(GL_ARRAY_BUFFER, lifetimeFor2D);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec2), particleLifetimes.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(14);
    glVertexAttribPointer(14, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glVertexAttribDivisor(14, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleRenderer::Exit() {

}