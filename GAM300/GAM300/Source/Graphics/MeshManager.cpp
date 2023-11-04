/*!***************************************************************************************
\file			MeshManager.cpp
\project
\author         Euan Lim, Davin Tan,Jake Lian, Theophelia Tan

\par			Course: GAM300
\date           28/09/2023

\brief
    This file contains the Mesh Manager and it's related Functionalities's definitions

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#include "Precompiled.h"
#include "MeshManager.h"
#include "GraphicsSystem.h"
#include "GraphicsHeaders.h"
#include "Core/EventsManager.h"
#include "AssetManager/GeomDecompiler.h"

void MESH_Manager::Init()
{
    EVENTS.Subscribe(this, &MESH_Manager::CallbackMeshAssetLoaded);
    EVENTS.Subscribe(this, &MESH_Manager::CallbackMeshAssetUnloaded);
    instanceProperties = &RENDERER.GetInstanceProperties();

    // Create all the hardcoded meshes here : Cube , (Maybe circle)?
	CreateInstanceCube();
    CreateInstanceSphere();
    
    CreateInstanceLine();
    CreateInstanceSegment3D();
}

MeshAsset& MESH_Manager::GetMeshAsset(const Engine::GUID& meshID)
{
    E_ASSERT(mMeshesAsset.find(meshID) != mMeshesAsset.end(),"Trying to access invalid mesh");
    return mMeshesAsset[meshID];
}

void MESH_Manager::GetGeomFromFiles(const std::string& filePath, const Engine::GUID& guid)
{
    ModelComponents newGeom(std::move(GEOMDECOMPILER.DeserializeGeoms(filePath, guid)));

    /*std::cout << "I have Materials : " << newGeom._materials.size() << 
        "from " << filePath << "\n";*/

    /*for (int i = 0; i < newGeom._materials.size(); ++i)
    {
        std::cout << "Ambience : " << newGeom._materials[i].Ambient.r << "\n";
        std::cout << "Ambience : " << newGeom._materials[i].Ambient.g << "\n";
        std::cout << "Ambience : " << newGeom._materials[i].Ambient.b << "\n";
        std::cout << "Ambience : " << newGeom._materials[i].Ambient.a << "\n";
        std::cout << "\n\n";
        std::cout << "Diffuse : " << newGeom._materials[i].Diffuse.r << "\n";
        std::cout << "Diffuse : " << newGeom._materials[i].Diffuse.g << "\n";
        std::cout << "Diffuse : " << newGeom._materials[i].Diffuse.b << "\n";
        std::cout << "Diffuse : " << newGeom._materials[i].Diffuse.a << "\n";
        std::cout << "\n\n";

        std::cout << "Specular : " << newGeom._materials[i].Specular.r << "\n";
        std::cout << "Specular : " << newGeom._materials[i].Specular.g << "\n";
        std::cout << "Specular : " << newGeom._materials[i].Specular.b << "\n";
        std::cout << "Specular : " << newGeom._materials[i].Specular.a << "\n";
        std::cout << "\n\n";*/


        // Pushing into the buffers
        /*temp_AlbedoContainer.push_back(glm::vec4(1.f, 1.f, 1.f, 1.f));
        temp_DiffuseContainer.push_back(glm::vec4(newGeom._materials[i].Diffuse.r, newGeom._materials[i].Diffuse.g,
            newGeom._materials[i].Diffuse.b, newGeom._materials[i].Diffuse.a));
        temp_SpecularContainer.push_back(glm::vec4(newGeom._materials[i].Specular.r, newGeom._materials[i].Specular.g,
            newGeom._materials[i].Specular.b, newGeom._materials[i].Specular.a));
        temp_AmbientContainer.push_back(glm::vec4(newGeom._materials[i].Ambient.r, newGeom._materials[i].Ambient.g,
            newGeom._materials[i].Ambient.b, newGeom._materials[i].Ambient.a));
        temp_ShininessContainer.push_back(0.f);
    }*/

    Mesh newMesh;
    newMesh.index = (unsigned int)mContainer.size();
    glm::vec3 min(FLT_MAX);
    glm::vec3 max(FLT_MIN);
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    for (int i = 0; i < newGeom.meshes.size(); ++i)
    {

        for (int k = 0; k < newGeom.meshes[i].vertices.size(); ++k)
        {
            glm::vec3& pos = newGeom.meshes[i].vertices[k].position;
            pos = pos * 0.01f; // Bean: 0.01f here converts the vertices position from centimeters to meters

            min.x = std::min(pos.x, min.x);
            min.y = std::min(pos.y, min.y);
            min.z = std::min(pos.z, min.z);

            max.x = std::max(pos.x, max.x);
            max.y = std::max(pos.y, max.y);
            max.z = std::max(pos.z, max.z);
        }

        // Bounds
        newMesh.vertices_min = newGeom.meshes[i].boundsMin = min;
        newMesh.vertices_max = newGeom.meshes[i].boundsMax = max;
        
        // Generate Vertex Array and Buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, newGeom.meshes[i].vertices.size() * sizeof(ModelVertex), &newGeom.meshes[i].vertices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)0);

        glEnableVertexAttribArray(1); // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, normal));

        glEnableVertexAttribArray(2); // Tangent
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, tangent));

        glEnableVertexAttribArray(3); // Texture Coordinates (uv coords)
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, textureCords));

        glEnableVertexAttribArray(4); // Color
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, color));

        glEnableVertexAttribArray(5); // Bone Indexes
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(ModelVertex), (void*)offsetof(ModelVertex, boneIDs));

        glEnableVertexAttribArray(6); // Bone Weights
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, weights));

        // bind indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, newGeom.meshes[i].indices.size() * sizeof(unsigned int), &newGeom.meshes[i].indices[0], GL_STATIC_DRAW);

        glBindVertexArray(0); // unbind vao
        glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo

        InstanceProperties tempProp;
        tempProp.VAO = VAO;
        tempProp.drawCount = (unsigned int)newGeom.meshes[i].indices.size();
        tempProp.drawType = GL_TRIANGLES;

        vaoMap.emplace(std::make_pair(guid, VAO)); // rmb change to guid after u ask someone @kk

        newMesh.prim = GL_TRIANGLES;
        newMesh.vaoID = VAO;
        newMesh.vboID = VBO;
        newMesh.drawCounts = (GLuint)(newGeom.meshes[i].indices.size());

        newMesh.SRTBufferIndex = InstanceSetup_PBR(tempProp);

        instanceProperties->emplace(std::make_pair(VAO, tempProp));

    }

    //debugAABB_setup(newMesh.vertices_min, newMesh.vertices_max, instanceProperties[0]);

    mContainer.emplace(guid, newMesh);
}

void MESH_Manager::AddMesh(const MeshAsset& _meshAsset, const Engine::GUID& _guid)
{
    Mesh newMesh;
    newMesh.index = (unsigned int)mContainer.size();
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    // Bounds
    newMesh.vertices_min = _meshAsset.boundsMin;
    newMesh.vertices_max = _meshAsset.boundsMax;

    // Generate Vertex Array and Buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, _meshAsset.vertices.size() * sizeof(ModelVertex), &_meshAsset.vertices[0], GL_STATIC_DRAW);
    
    // Bean:: This might be affecting the current crash
    //glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo

    // bind indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _meshAsset.indices.size() * sizeof(unsigned int), &_meshAsset.indices[0], GL_STATIC_DRAW);
    
    // Bean:: This might be affecting the current crash
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)0);

    glEnableVertexAttribArray(1); // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, normal));

    glEnableVertexAttribArray(2); // Tangent
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, tangent));

    glEnableVertexAttribArray(3); // Texture Coordinates (uv coords)
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, textureCords));

    glEnableVertexAttribArray(4); // Color
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, color));

    glEnableVertexAttribArray(5); // Bone Indexes
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(ModelVertex), (void*)offsetof(ModelVertex, boneIDs));

    glEnableVertexAttribArray(6); // Bone Weights
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof(ModelVertex, weights));
    
    glBindVertexArray(0); // unbind vao

    InstanceProperties tempProp;
    tempProp.VAO = VAO;
    tempProp.drawCount = (unsigned int)_meshAsset.indices.size();
    tempProp.drawType = GL_TRIANGLES;

    vaoMap.emplace(std::make_pair(_guid, VAO));

    newMesh.prim = GL_TRIANGLES;
    newMesh.vaoID = VAO;
    newMesh.vboID = VBO;
    newMesh.drawCounts = (GLuint)(_meshAsset.indices.size());
    newMesh.numBones = _meshAsset.numBones;

    newMesh.verticesBoneInfo.resize(_meshAsset.numVertices);
    for (size_t i = 0; i < _meshAsset.numVertices; i++)
    {
        for (size_t j = 0; j < MAX_BONE_INFLUENCE; j++)
        {
            newMesh.verticesBoneInfo[i].boneIDs[j] = _meshAsset.vertices[i].boneIDs[j];
            newMesh.verticesBoneInfo[i].weights[j] = _meshAsset.vertices[i].weights[j];
        }
    }

    newMesh.SRTBufferIndex = InstanceSetup_PBR(tempProp);
    //PRINT("Using guid: ", _guid.ToHexString(), " for ", _meshAsset.mFilePath.stem().string(), '\n');
    debugAABB_setup(newMesh.vertices_min, newMesh.vertices_max, tempProp);
    instanceProperties->emplace(std::make_pair(VAO, tempProp));

    mContainer.emplace(_guid, newMesh);
}

//// NEED TO CHANGE TO MAKE IT PROCESS STUFF FROM COMPILER
//AnimGeomImported MESH_Manager::DeserializeAnimGeoms(const std::string& filePath, const std::string& fileName)
//{
//    std::vector<AnimationMesh> tempmesh = AnimationManager.GetModel().meshes;
//    AnimGeomImported tempgeom;
//
//    for (size_t i = 0; i < tempmesh.size(); i++)
//    {
//        gAnimMesh newtempmesh;
//
//        for (size_t j = 0; j < tempmesh[i].vertices.size(); j++)
//        {
//            gAnimVertex newtempvert;
//            newtempvert.pos = tempmesh[i].vertices[j].Position;
//            newtempvert.normal = tempmesh[i].vertices[j].Normal;
//            newtempvert.tangent = tempmesh[i].vertices[j].Tangent;
//            newtempvert.tex = tempmesh[i].vertices[j].TexCoords;
//            newtempvert.color = tempmesh[i].vertices[j].Color;
//            
//            for (size_t k = 0; k < MAX_BONE_INFLUENCE; k++)
//            {
//                newtempvert.m_BoneIDs[k] = tempmesh[i].vertices[j].m_BoneIDs[k];
//                newtempvert.m_Weights[k] = tempmesh[i].vertices[j].m_Weights[k];
//            }
//            newtempmesh._vertices.push_back(newtempvert);
//        }
//        for (size_t j = 0; j < tempmesh[i].indices.size(); j++)
//        {
//            unsigned int newtempind;
//            newtempind = tempmesh[i].indices[j];
//            newtempmesh._indices.push_back(newtempind);
//        }
//
//        tempgeom.mMeshes.push_back(newtempmesh);
//    }
//
//    //tempgeom.oneAnimation = 
//
//    return tempgeom;
//}

void MESH_Manager::CreateInstanceCube()
{

    Mesh newMesh;
    newMesh.index = (unsigned int)mContainer.size();

    GLfloat vertices[] = {
        // Positions           Normals            Tangents          Texture Coords     Colors (RGB)
        // Front face
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f,      //1.0f, 0.0f, 0.0f, 1.f,
         0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, 0.0f,    1.0f, 0.0f,      //1.0f, 0.0f, 0.0f, 1.f,
         0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, 0.0f,    1.0f, 1.0f,      //1.0f, 0.0f, 0.0f, 1.f,
        -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f,      //1.0f, 0.0f, 0.0f, 1.f,
                                                                                          //
        // Back face                                                                      //
        -0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,      //0.0f, 1.0f, 0.0f, 1.f,
         0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,      //0.0f, 1.0f, 0.0f, 1.f,
         0.5f,  0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,      //0.0f, 1.0f, 0.0f, 1.f,
        -0.5f,  0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,      //0.0f, 1.0f, 0.0f, 1.f,
                                                                                          //
        // Left face                                                                      //
        -0.5f, 0.5f, 0.5f,     -1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,      //0.0f, 0.0f, 1.0f, 1.f,
        -0.5f, 0.5f, -0.5f,    -1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,   1.0f, 0.0f,      //0.0f, 0.0f, 1.0f, 1.f,
        -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,   1.0f, 1.0f,      //0.0f, 0.0f, 1.0f, 1.f,
        -0.5f, -0.5f, 0.5f,    -1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f,      //0.0f, 0.0f, 1.0f, 1.f,
                                                                                          //
        // Right face                                                                     //
         0.5f, 0.5f, 0.5f,     1.0f, 0.0f, 0.0f,     0.0f, -1.0f, 0.0f,  0.0f, 0.0f,      //1.0f, 1.0f, 0.0f, 1.f,
         0.5f, 0.5f, -0.5f,    1.0f, 0.0f, 0.0f,     0.0f, -1.0f, 0.0f,  1.0f, 0.0f,      //1.0f, 1.0f, 0.0f, 1.f,
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,     0.0f, -1.0f, 0.0f,  1.0f, 1.0f,      //1.0f, 1.0f, 0.0f, 1.f,
         0.5f, -0.5f, 0.5f,    1.0f, 0.0f, 0.0f,     0.0f, -1.0f, 0.0f,  0.0f, 1.0f,      //1.0f, 1.0f, 0.0f, 1.f,
                                                                                          //
         // Top face                                                                      //
         -0.5f, 0.5f, -0.5f,    0.0f, 1.0f, 0.0f,     1.0f, 0.0f, 0.0f,  0.0f, 0.0f,      //0.0f, 1.0f, 1.0f, 1.f,
          0.5f, 0.5f, -0.5f,    0.0f, 1.0f, 0.0f,     1.0f, 0.0f, 0.0f,  1.0f, 0.0f,      //0.0f, 1.0f, 1.0f, 1.f,
          0.5f, 0.5f, 0.5f,     0.0f, 1.0f, 0.0f,     1.0f, 0.0f, 0.0f,  1.0f, 1.0f,      //0.0f, 1.0f, 1.0f, 1.f,
         -0.5f, 0.5f, 0.5f,     0.0f, 1.0f, 0.0f,     1.0f, 0.0f, 0.0f,  0.0f, 1.0f,      //0.0f, 1.0f, 1.0f, 1.f,
                                                                                          //
         // Bottom face                                                                   //
         -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,    1.0f, 0.0f, 0.0f,  0.0f, 0.0f,      //1.0f, 1.0f, 0.0f, 1.f,
          0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,    1.0f, 0.0f, 0.0f,  1.0f, 0.0f,      //1.0f, 1.0f, 0.0f, 1.f,
          0.5f, -0.5f, 0.5f,    0.0f, -1.0f, 0.0f,    1.0f, 0.0f, 0.0f,  1.0f, 1.0f,      //1.0f, 1.0f, 0.0f, 1.f,
         -0.5f, -0.5f, 0.5f,    0.0f, -1.0f, 0.0f,    1.0f, 0.0f, 0.0f,  0.0f, 1.0f      //1.0f, 1.0f, 0.0f, 1.f
    };

    GLuint indices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Back face
        4, 5, 6,
        6, 7, 4,

        // Left face
        8, 9, 10,
        10, 11, 8,

        // Right face
        12, 13, 14,
        14, 15, 12,

        // Top face
        16, 17, 18,
        18, 19, 16,

        // Bottom face
        20, 21, 22,
        22, 23, 20
    };

    StoreMeshVertex(DEFAULT_ASSETS["Cube.geom"], { -0.5f, 0.5f, -0.5f });
    StoreMeshVertex(DEFAULT_ASSETS["Cube.geom"], { 0.5f, 0.5f, -0.5f });
    StoreMeshVertex(DEFAULT_ASSETS["Cube.geom"], { 0.5f, 0.5f, 0.5f });
    StoreMeshVertex(DEFAULT_ASSETS["Cube.geom"], { -0.5f, 0.5f, 0.5f });
    StoreMeshIndex (DEFAULT_ASSETS["Cube.geom"], 0);
    StoreMeshIndex (DEFAULT_ASSETS["Cube.geom"], 1);
    StoreMeshIndex (DEFAULT_ASSETS["Cube.geom"], 2);
    StoreMeshIndex (DEFAULT_ASSETS["Cube.geom"], 2);
    StoreMeshIndex (DEFAULT_ASSETS["Cube.geom"], 3);
    StoreMeshIndex (DEFAULT_ASSETS["Cube.geom"], 0);

    //// Top Face
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, 0.5f, -0.5f }, 0);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, 0.5f, -0.5f }, 1);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, 0.5f, 0.5f }, 2);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, 0.5f, 0.5f }, 2);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, 0.5f, 0.5f }, 3);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, 0.5f, -0.5f }, 0);

    //// Front Face
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, -0.5f }, 0);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, -0.5f, -0.5f }, 1);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f,  0.5f, -0.5f }, 2);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f,  0.5f, -0.5f }, 2);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f,  0.5f, -0.5f }, 3);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, -0.5f }, 0);

    //// Back Face
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, 0.5f }, 4);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, -0.5f, 0.5f }, 5);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f,  0.5f, 0.5f }, 6);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f,  0.5f, 0.5f }, 6);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f,  0.5f, 0.5f }, 7);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, 0.5f }, 4);

    //// Left Face
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, 0.5f, 0.5f }, 8);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, 0.5f, -0.5f }, 9);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, -0.5f }, 10);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, -0.5f }, 10);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, 0.5f }, 11);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, 0.5f, 0.5f }, 8);

    //// Right Face
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, 0.5f, 0.5f }, 12);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, 0.5f, -0.5f }, 13);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, -0.5f, -0.5f }, 14);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, -0.5f, -0.5f }, 14);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, -0.5f, 0.5f }, 15);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, 0.5f, 0.5f }, 12);

    //// Top Face
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, 0.5f, -0.5f }, 16);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, 0.5f, -0.5f }, 17);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, 0.5f, 0.5f }, 18);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, 0.5f, 0.5f }, 18);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, 0.5f, 0.5f }, 19);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, 0.5f, -0.5f }, 16);

    //// Bottom Face
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, -0.5f }, 20);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, -0.5f, -0.5f }, 21);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, -0.5f, 0.5f }, 22);
    //ASSETMANAGER.StoreMesh("Cube", { 0.5f, -0.5f, 0.5f }, 22);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, 0.5f }, 23);
    //ASSETMANAGER.StoreMesh("Cube", { -0.5f, -0.5f, -0.5f }, 20);

    newMesh.vertices_min = glm::vec3(-0.5f, -0.5f, -0.5f);
    newMesh.vertices_max = glm::vec3(0.5f, 0.5f, 0.5f);

    // first, configure the cube's VAO (and VBO)
    //unsigned int VBO, cubeVAO;

    GLuint vaoid;
    GLuint vboid;
    GLuint ebo;
    glGenVertexArrays(1, &vaoid);
    glGenBuffers(1, &vboid);
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vboid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(vaoid);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Tangent attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Texture coord
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    // color coord
    /*glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);*/

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0); // unbind vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo

    Engine::GUID& cubeGUID = DEFAULT_ASSETS["Cube.geom"];
    InstanceProperties tempProp; 
    tempProp.VAO = vaoid;
    tempProp.drawCount = 36;
    tempProp.drawType = GL_TRIANGLES;
    vaoMap.emplace(std::make_pair(cubeGUID, vaoid));

    newMesh.vaoID = vaoid;
    newMesh.vboID = vboid;
    newMesh.prim = GL_TRIANGLES;
    newMesh.drawCounts = 36;
    newMesh.SRTBufferIndex = InstanceSetup_PBR(tempProp);

    debugAABB_setup(newMesh.vertices_min, newMesh.vertices_max, tempProp);
    mContainer.emplace(cubeGUID, newMesh);
    instanceProperties->emplace(std::make_pair(vaoid, tempProp));
}


void MESH_Manager::CreateInstanceSphere()
{
    Mesh newMesh;
    newMesh.index = (unsigned int)mContainer.size();
    GLuint vaoid;
    GLuint vboid;
    GLuint ebo;


    glGenVertexArrays(1, &vaoid);
    glGenBuffers(1, &vboid);
    glGenBuffers(1, &ebo);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;

    glm::vec3 min(FLT_MAX);
    glm::vec3 max(FLT_MIN);

    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI) * 0.5f;
            float yPos = std::cos(ySegment * PI) * 0.5f;
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI) * 0.5f;

            positions.push_back(glm::vec3(xPos, yPos, zPos));
            uv.push_back(glm::vec2(xSegment, ySegment));
            normals.push_back(glm::vec3(xPos, yPos, zPos));
            min.x = std::min(xPos, min.x);
            min.y = std::min(yPos, min.y);
            min.z = std::min(zPos, min.z);

            max.x = std::max(xPos, max.x);
            max.y = std::max(yPos, max.y);
            max.z = std::max(zPos, max.z);

        }
    }
    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }

    std::vector<float> data;
    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
        if (uv.size() > 0)
        {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
    }
    glBindVertexArray(vaoid);
    glBindBuffer(GL_ARRAY_BUFFER, vboid);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    unsigned int stride = (3 + 2 + 3) * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    InstanceProperties tempProp;
    tempProp.drawType = GL_TRIANGLE_STRIP;
    tempProp.VAO = vaoid;
    tempProp.drawCount = (unsigned int)(indices.size()) ;

    Engine::GUID& sphereGUID = DEFAULT_ASSETS["Sphere.geom"];
    vaoMap.emplace(std::make_pair(sphereGUID,vaoid));

    //vaoMap.emplace(std::pair<std::string, GLuint>(AssetManager::Instance().GetAssetGUID("Sphere"), vaoid));
    //instanceProperties->emplace(std::pair<std::string, InstanceProperties>(std::string("Sphere"), tempProp));
    newMesh.vaoID = vaoid;
    newMesh.vboID = vboid;
    newMesh.prim = GL_TRIANGLE_STRIP;
    newMesh.drawCounts = (unsigned int)(indices.size());
    newMesh.SRTBufferIndex = InstanceSetup_PBR(tempProp);
    //newMesh.SRTBufferIndex.push_back(InstanceSetup_PBR((*instanceProperties)["Sphere"]));
    newMesh.vertices_min = min;
    newMesh.vertices_max = max;

    //Do something about AABB
    debugAABB_setup(newMesh.vertices_min, newMesh.vertices_max, tempProp);
    mContainer.emplace(sphereGUID, newMesh);
    instanceProperties->emplace(std::make_pair(vaoid, tempProp));


}


// THIS IS THE PREVIOUS MATERIAL STUFFS -> BLINN PHONG THINGS
unsigned int  MESH_Manager::InstanceSetup(InstanceProperties& prop) {


    // SRT Buffer set up
    prop.entitySRTbuffer;
    glGenBuffers(1, &prop.entitySRTbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.entitySRTbuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::mat4), &(prop.entitySRT[0]), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //entitySRTBuffer
    glBindVertexArray(prop.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, prop.entitySRTbuffer);
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


    // Albedo Buffer Setup
    prop.AlbedoBuffer;
    glGenBuffers(1, &prop.AlbedoBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.AlbedoBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec4), &(prop.Albedo[0]), GL_STATIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(10, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Specular Buffer Setup
    prop.SpecularBuffer;
    glGenBuffers(1, &prop.SpecularBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.SpecularBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec4), &(prop.Specular[0]), GL_STATIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(11);
    glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(11, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Diffuse Buffer Setup
    prop.DiffuseBuffer;
    glGenBuffers(1, &prop.DiffuseBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.DiffuseBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec4), &(prop.Diffuse[0]), GL_STATIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(12);
    glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(12, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Ambient Buffer SetupF
    prop.AmbientBuffer;
    glGenBuffers(1, &prop.AmbientBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.AmbientBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec4), &(prop.Ambient[0]), GL_STATIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(13);
    glVertexAttribPointer(13, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(13, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Shininess Buffer Setup
    prop.ShininessBuffer;
    glGenBuffers(1, &prop.ShininessBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.ShininessBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(float), &(prop.Shininess[0]), GL_STATIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(14);
    glVertexAttribPointer(14, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glVertexAttribDivisor(14, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    prop.textureIndexBuffer;
    glGenBuffers(1, &prop.textureIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.textureIndexBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec2), &(prop.textureIndex[0]), GL_STATIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(15);
    glVertexAttribPointer(15, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glVertexAttribDivisor(15, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //// Material Buffer Setup
    //prop.entityMATbuffer;
    //glBindVertexArray(prop.VAO);

    //glGenBuffers(1, &prop.entityMATbuffer);
    //glBindBuffer(GL_ARRAY_BUFFER, prop.entityMATbuffer);

    //glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit *sizeof(Materials), &(prop.entityMAT[0]), GL_STATIC_DRAW);
    //// Albedo
    //glEnableVertexAttribArray(10);

    //glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)0);
    //// Specular
    //glEnableVertexAttribArray(11);

    //glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)(sizeof(glm::vec4)));
    //// Diffuse
    //glEnableVertexAttribArray(12);

    //glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)(2 * sizeof(glm::vec4)));
    //// Ambient
    //glEnableVertexAttribArray(13);

    //glVertexAttribPointer(13, 4, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)(3 * sizeof(glm::vec4)));
    //// Shininess
    //glEnableVertexAttribArray(14);

    //glVertexAttribPointer(14, 1, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)(4 * sizeof(glm::vec4)));
    //glVertexAttribDivisor(10, 1);
    //glVertexAttribDivisor(11, 1);
    //glVertexAttribDivisor(12, 1);
    //glVertexAttribDivisor(13, 1);
    //glVertexAttribDivisor(14, 1);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);



    return prop.entitySRTbuffer;
}

// THIS IS THE PREVIOUS MATERIAL STUFFS -> PBR
unsigned int  MESH_Manager::InstanceSetup_PBR(InstanceProperties& prop) {

    size_t buffersize = prop.entitySRT.size();
    // SRT Buffer set up
    prop.entitySRTbuffer;
    glGenBuffers(1, &prop.entitySRTbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.entitySRTbuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::mat4), prop.entitySRT.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //entitySRTBuffer
    glBindVertexArray(prop.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, prop.entitySRTbuffer);
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


    // Albedo Buffer Setup
    prop.AlbedoBuffer;
    glGenBuffers(1, &prop.AlbedoBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.AlbedoBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec4), prop.Albedo.data(), GL_STATIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(10, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Metal_Rough_AO_Texture_Buffer Buffer Setup
    prop.Metal_Rough_AO_Texture_Buffer;
    glGenBuffers(1, &prop.Metal_Rough_AO_Texture_Buffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.Metal_Rough_AO_Texture_Buffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec4), prop.M_R_A_Texture.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(11);
    glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glVertexAttribDivisor(11, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Diffuse Buffer Setup
    prop.Metal_Rough_AO_Texture_Constant;
    glGenBuffers(1, &prop.Metal_Rough_AO_Texture_Constant);
    glBindBuffer(GL_ARRAY_BUFFER, prop.Metal_Rough_AO_Texture_Constant);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec4), prop.M_R_A_Constant.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(12);
    glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glVertexAttribDivisor(12, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    prop.textureIndexBuffer;
    glGenBuffers(1, &prop.textureIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, prop.textureIndexBuffer);
    glBufferData(GL_ARRAY_BUFFER, EntityRenderLimit * sizeof(glm::vec2), prop.textureIndex.data(), GL_STATIC_DRAW);

    glBindVertexArray(prop.VAO);
    glEnableVertexAttribArray(15);
    glVertexAttribPointer(15, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glVertexAttribDivisor(15, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return prop.entitySRTbuffer;
}

void MESH_Manager::CreateInstanceLine()
{
    Mesh newMesh;
    newMesh.index = (unsigned int)(mContainer.size());
    GLfloat vertices[] = {
        -1.f, 0.f, 0.f,   
        1.0f, 0.f, 0.f
    };

    GLuint indices[] = {
        0, 1
    };

    newMesh.vertices_min = glm::vec3(-1.f, 0.f, 0.f);
    newMesh.vertices_max = glm::vec3(-1.f, 0.f, 0.f);
    // first, configure the cube's VAO (and VBO)
    //unsigned int VBO, cubeVAO;

    GLuint vaoid;
    GLuint vboid;
    GLuint ebo;
    glGenVertexArrays(1, &vaoid);
    glGenBuffers(1, &vboid);
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vboid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(vaoid);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0); // unbind vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo

    InstanceProperties tempProp;
    tempProp.VAO = vaoid;
    tempProp.drawCount = 2;
    Engine::GUID& lineGUID = DEFAULT_ASSETS["Line.geom"];
    vaoMap.emplace(std::make_pair(lineGUID, vaoid));
    //instanceProperties->emplace(std::pair<std::string, InstanceProperties>(std::string("Line"), tempProp));
    newMesh.vaoID = vaoid;
    newMesh.vboID = vboid;
    newMesh.prim = GL_LINES;
    newMesh.drawCounts = 2;
    newMesh.SRTBufferIndex = InstanceSetup_PBR(tempProp);
    //InstanceSetup_PBR(tempProp);
    //newMesh.SRTBufferIndex.push_back(InstanceSetup_PBR((*instanceProperties)["Line"]));
    //debugAABB_setup(newMesh.vertices_min, newMesh.vertices_max, tempProp);

    mContainer.emplace(lineGUID, newMesh);
    instanceProperties->emplace(std::make_pair(vaoid, tempProp));

}

void MESH_Manager::CreateInstanceSegment3D()
{
    Mesh newMesh;
    newMesh.index = (unsigned int)(mContainer.size());

    GLfloat vertices[] = {
        0.f,0.f,0.f,
        1.f,1.f,1.f
    };

    GLuint indices[] = {
        0, 1
    };
    newMesh.vertices_min = glm::vec3(0.f, 0.f, 0.f);
    newMesh.vertices_max = glm::vec3(1.f, 1.f, 1.f);

    // first, configure the cube's VAO (and VBO)
    //unsigned int VBO, cubeVAO;

    GLuint vaoid;
    GLuint vboid;
    GLuint ebo;
    glGenVertexArrays(1, &vaoid);
    glGenBuffers(1, &vboid);
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vboid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(vaoid);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0); // unbind vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo

    InstanceProperties tempProp;
    tempProp.VAO = vaoid;
    tempProp.drawCount = 2;
    tempProp.drawType = GL_LINES;

    Engine::GUID& segGUID{DEFAULT_ASSETS["Segment3D.geom"]};

    vaoMap.emplace(std::make_pair(segGUID, vaoid));

    //properties->emplace(std::pair<std::string, InstanceProperties>(std::string("Segment3D"), tempProp));
    newMesh.vaoID = vaoid;
    newMesh.vboID = vboid;
    newMesh.prim = GL_LINES;
    newMesh.drawCounts = 2;
    newMesh.SRTBufferIndex = InstanceSetup_PBR(tempProp);

    //InstanceSetup_PBR(tempProp);

    mContainer.emplace(segGUID, newMesh);
    instanceProperties->emplace(std::make_pair(vaoid, tempProp));

}

void MESH_Manager::debugAABB_setup(glm::vec3 minpt, glm::vec3 maxpt, InstanceProperties& prop) // vao
{
    //// find min max points for each axis
    //glm::vec3 minpt = _geom->_vertices[0].pos, maxpt = _geom->_vertices[0].pos;

    //for (size_t i = 0; i < _geom->_vertices.size(); i++)
    //{
    //    minpt.x = std::min(minpt.x, _geom->_vertices[i].pos.x);
    //    minpt.y = std::min(minpt.y, _geom->_vertices[i].pos.y);
    //    minpt.z = std::min(minpt.z, _geom->_vertices[i].pos.z);

    //    maxpt.x = std::max(maxpt.x, _geom->_vertices[i].pos.x);
    //    maxpt.y = std::max(maxpt.y, _geom->_vertices[i].pos.y);
    //    maxpt.z = std::max(maxpt.z, _geom->_vertices[i].pos.z);
    //}
    glm::vec3 pntAABB[8];
    std::vector<glm::ivec2> idxAABB{};

    pntAABB[0] = minpt;
    pntAABB[1] = glm::vec3(minpt.x, minpt.y, maxpt.z);
    pntAABB[2] = glm::vec3(minpt.x, maxpt.y, maxpt.z);
    pntAABB[3] = glm::vec3(minpt.x, maxpt.y, minpt.z);

    pntAABB[4] = maxpt;
    pntAABB[5] = glm::vec3(maxpt.x, maxpt.y, minpt.z);
    pntAABB[6] = glm::vec3(maxpt.x, minpt.y, minpt.z);
    pntAABB[7] = glm::vec3(maxpt.x, minpt.y, maxpt.z);

    /*for (int i = 0; i < 8; ++i) {
        PRINT(pntAABB[i].x, ", ");
        PRINT(pntAABB[i].y, ", ");
        PRINT(pntAABB[i].z, "\n");
    }
    PRINT("\n");*/

    int indice = 0;

    idxAABB.push_back(glm::ivec2(indice, indice + 1));
    idxAABB.push_back(glm::ivec2(indice + 1, indice + 2));
    idxAABB.push_back(glm::ivec2(indice + 2, indice + 3));
    idxAABB.push_back(glm::ivec2(indice + 3, indice));

    idxAABB.push_back(glm::ivec2(indice + 4, indice + 5));
    idxAABB.push_back(glm::ivec2(indice + 5, indice + 6));
    idxAABB.push_back(glm::ivec2(indice + 6, indice + 7));
    idxAABB.push_back(glm::ivec2(indice + 7, indice + 4));

    idxAABB.push_back(glm::ivec2(indice + 7, indice + 1));
    idxAABB.push_back(glm::ivec2(indice + 4, indice + 2));
    idxAABB.push_back(glm::ivec2(indice + 5, indice + 3));
    idxAABB.push_back(glm::ivec2(indice + 6, indice));


    // setup vao
    //GLuint DebugVaoid; // point buffer
    GLuint Pbuff; // point buffer
    GLuint Ibuff; // indice buffer

    glGenVertexArrays(1, &prop.debugVAO);
    glGenBuffers(1, &Pbuff);
    glGenBuffers(1, &Ibuff);

    glBindVertexArray(prop.debugVAO);
    glBindBuffer(GL_ARRAY_BUFFER, Pbuff);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 8, &pntAABB[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ibuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 2 * idxAABB.size(),
        &idxAABB[0], GL_STATIC_DRAW);

    glBindVertexArray(0); // unbind vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind vbo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind ebo

    //entitySRTBuffer
    glBindVertexArray(prop.debugVAO);
    glBindBuffer(GL_ARRAY_BUFFER, prop.entitySRTbuffer);
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
    //return DebugVaoid;
}

void MESH_Manager::StoreMeshVertex(const Engine::GUID& mKey, const glm::vec3& mVertex)
{
    ModelVertex v;
    v.position = mVertex;
    mMeshesAsset[mKey].vertices.push_back(v);
}

void MESH_Manager::StoreMeshIndex(const Engine::GUID& mKey, const int& mIndex)
{
    mMeshesAsset[mKey].indices.push_back(mIndex);
}

void MESH_Manager::CallbackMeshAssetLoaded(AssetLoadedEvent<MeshAsset>* pEvent)
{
    AddMesh(pEvent->asset, pEvent->guid);
}

void MESH_Manager::CallbackMeshAssetUnloaded(AssetUnloadedEvent<MeshAsset>* pEvent)
{

}

