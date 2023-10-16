/*!***************************************************************************************
\file			AnimationManager.h
\project
\author         Euphrasia Theophelia Tan Ee Mun

\par			Course: GAM300
\date           10/10/2023

\brief
	This file contains the Animation Manager and the declarations of its related functions.

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#pragma once

#include "../Core/SystemInterface.h"
#include "../gli-master/gli/gli.hpp"
#include "glslshader.h"

#include <glm/gtx/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include "Precompiled.h"
#include "IOManager/Handler_GLFW.h"
#include "glslshader.h"
#include "../../glfw-3.3.8.bin.WIN64/include/GLFW/glfw3.h"
#include "TextureManager.h"
#include "AssetManager/AssetManager.h"

#include "../../Compiler/Mesh.h"

#include "Model3d.h"


#define MAX_BONE_INFLUENCE 4

struct AnimationVertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct AnimationTexture {
    unsigned int id;
    std::string type;
    std::string path;
};

class AnimationMesh {
public:
    // mesh Data
    std::vector<AnimationVertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<AnimationTexture>textures;
    GLuint _VAO;

    // constructor
    AnimationMesh(std::vector<AnimationVertex> vertices, std::vector<unsigned int> indices, std::vector<AnimationTexture> textures);

    // render the mesh
    void Draw(GLSLShader& shader);

private:

    // initializes all the buffer objects/arrays
    void setupMesh();
};



class AssimpGLMHelpers
{
public:

    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
    {
        return glm::vec3(vec.x, vec.y, vec.z);
    }

    static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
    {
        return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
    }
};


struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

class Bone
{
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);

    void Update(float animationTime);
    glm::mat4 GetLocalTransform() { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() { return m_ID; }



    int GetPositionIndex(float animationTime);

    int GetRotationIndex(float animationTime);

    int GetScaleIndex(float animationTime);


private:

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    glm::mat4 InterpolatePosition(float animationTime);

    glm::mat4 InterpolateRotation(float animationTime);

    glm::mat4 InterpolateScaling(float animationTime);

    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;
};


struct BoneInfo
{
    /*id is index in finalBoneMatrices*/
    int id;

    /*offset matrix transforms vertex from model space to bone space*/
    glm::mat4 offset;

};




struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};


class AnimationModel
{
public:
    // model data 
    std::vector<AnimationTexture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<AnimationMesh> meshes;
    std::string directory;
    bool gammaCorrection;



    // constructor, expects a filepath to a 3D model.
    AnimationModel(std::string const& path, bool gamma);

    // draws the model, and thus all its meshes
    void Draw(GLSLShader& shader);

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }


private:

    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const& path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene);

    void SetVertexBoneDataToDefault(AnimationVertex& vertex);


    AnimationMesh processMesh(aiMesh* mesh, const aiScene* scene);

    void SetVertexBoneData(AnimationVertex& vertex, int boneID, float weight);


    void ExtractBoneWeightForVertices(std::vector<AnimationVertex>& vertices, aiMesh* mesh, const aiScene* scene);

    //unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false)
    //{
    //    string filename = string(path);
    //    filename = directory + '/' + filename;

    //    unsigned int textureID;
    //    glGenTextures(1, &textureID);

    //    int width, height, nrComponents;
    //    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    //    if (data)
    //    {
    //        GLenum format;
    //        if (nrComponents == 1)
    //            format = GL_RED;
    //        else if (nrComponents == 3)
    //            format = GL_RGB;
    //        else if (nrComponents == 4)
    //            format = GL_RGBA;

    //        glBindTexture(GL_TEXTURE_2D, textureID);
    //        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    //        glGenerateMipmap(GL_TEXTURE_2D);

    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //        stbi_image_free(data);
    //    }
    //    else
    //    {
    //        std::cout << "Texture failed to load at path: " << path << std::endl;
    //        stbi_image_free(data);
    //    }

    //    return textureID;
    //}

    //// checks all material textures of a given type and loads the textures if they're not loaded yet.
    //// the required info is returned as a Texture struct.
    //vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    //{
    //    vector<Texture> textures;
    //    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    //    {
    //        aiString str;
    //        mat->GetTexture(type, i, &str);
    //        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
    //        bool skip = false;
    //        for (unsigned int j = 0; j < textures_loaded.size(); j++)
    //        {
    //            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
    //            {
    //                textures.push_back(textures_loaded[j]);
    //                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
    //                break;
    //            }
    //        }
    //        if (!skip)
    //        {   // if texture hasn't been loaded already, load it
    //            Texture texture;
    //            texture.id = TextureFromFile(str.C_Str(), this->directory);
    //            texture.type = typeName;
    //            texture.path = str.C_Str();
    //            textures.push_back(texture);
    //            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
    //        }
    //    }
    //    return textures;
    //}
};


class Animation
{
public:
    Animation() = default;

    Animation(const std::string& animationPath, AnimationModel* model);


    ~Animation();

    Bone* FindBone(const std::string& name);


    inline float GetTicksPerSecond() { return m_TicksPerSecond; }
    inline float GetDuration() { return m_Duration; }
    inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
    inline const std::map<std::string, BoneInfo>& GetBoneIDMap()
    {
        return m_BoneInfoMap;
    }

private:
    void ReadMissingBones(const aiAnimation* animation, AnimationModel& model);

    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);

    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;
};


class AnimationAnimator
{
public:
    AnimationAnimator(Animation* animation);

    void UpdateAnimation(float dt);

    void PlayAnimation(Animation* pAnimation);

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

    std::vector<glm::mat4> GetFinalBoneMatrices();

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;

};



#define AnimationManager Animation_Manager::Instance()

SINGLETON(Animation_Manager)
{
public:
	void Init();
	void Update(float dt);
	void Exit();

	//// used in asset manager to add dds textures to the texture container
	//void AddTexture(char const* Filename, std::string GUID);

	//// uses GUID to retrieve a texture from the texture container
	//GLuint GetTexture(std::string GUID);

	//// creates a texture and returns it to be stored in the texture container
	//GLuint CreateTexture(char const* Filename);

	//// creates a skybox texture and returns it to be stored in the texture container
	//GLuint CreateSkyboxTexture(char const* Filename);

private:

	//std::unordered_map<std::string, std::pair<char const*, GLuint>> mAnimationContainer; // GUID, <file name, GLuint>

};

