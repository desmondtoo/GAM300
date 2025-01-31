/*!***************************************************************************************
\file			TextureManager.cpp
\project
\author         Euphrasia Theophelia Tan Ee Mun, Euan Lim Yiren

\par			Course: GAM300
\date           28/09/2023

\brief
    This file contains the Texture Manager and the definitions of its related functions. 

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#include "Precompiled.h"
#include "TextureManager.h"
#include "Core/EventsManager.h"


void Texture_Manager::Init()
{
    EVENTS.Subscribe(this, &Texture_Manager::CallbackTextureAssetLoaded);
    EVENTS.Subscribe(this, &Texture_Manager::CallbackTextureAssetUnloaded);
}

void Texture_Manager::Update(float)
{
    // Empty by design
}

void Texture_Manager::Exit()
{

}

void Texture_Manager::AddTexture(char const* Filename, const Engine::GUID<TextureAsset>& GUID)
{
    BaseTexture tempTexture;
    tempTexture.path = Filename;

    // check if from skybox
    std::string searchWord = "skybox";
    const char* found = strstr(Filename, searchWord.c_str());

    if (found != nullptr) {
        // if skybox
        
        // if filename_top, we create skybox texture as the other .dds should have been loaded in
        searchWord = "_top";
        found = strstr(Filename, searchWord.c_str());

        if (found != nullptr) {
            size_t length = found - Filename;
            char* subString = new char[length + 1];

            //strncpy(subString, Filename, length);
            //subString[length] = '\0';
            strncpy_s(subString, length + 1, Filename, length);
            subString[length] = '\0';

            tempTexture.textureID = CreateSkyboxTexture(subString);

            E_ASSERT(tempTexture.textureID, "Skybox texture creation failed. Check if all textures necessary for skybox creation are named correctly.");

            delete[] subString;
            mTextureContainer.emplace(GUID, tempTexture);
        }
    }
    else {
        // if not skybox
        CreateTexture(tempTexture);
        E_ASSERT(tempTexture.textureID, "Texture creation failed.");
        mTextureContainer.emplace(GUID, tempTexture);
    }

}

GLuint Texture_Manager::CreateTexture(char const* Filename)
{
    BaseTexture tempTexture;
    tempTexture.path = Filename;
    tempTexture.textureID = 0;
    CreateTexture(tempTexture);

    return tempTexture.textureID;
}

/// Filename can be KTX or DDS files
void Texture_Manager::CreateTexture(BaseTexture& _texture)
{
    gli::texture Texture = gli::load(_texture.path);
    if (Texture.empty())
        return;

    gli::gl GL(gli::gl::PROFILE_GL33);
    gli::gl::format Format = GL.translate(Texture.format(), Texture.swizzles());
    GLenum Target = GL.translate(Texture.target());

    //// New: Use BC3 (DXT3) format for alpha support
    //Format.Internal = gli::gl::INTERNAL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    //Format.External = gli::gl::EXTERNAL_RGBA;
    //Format.Type = gli::gl::TYPE_UNSIGNED_BYTE;

    //Texture = gli::flip<gli::texture>(Texture);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint TextureName = 0; 
    glGenTextures(1, &TextureName);
    glBindTexture(Target, TextureName);
    glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
    glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
    glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
    glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
    glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

    glm::tvec3<GLsizei> /*const*/ Extent(Texture.extent());
    GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

    switch (Texture.target())
    {
    case gli::TARGET_1D:
        glTexStorage1D(
            Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
        break;
    case gli::TARGET_1D_ARRAY:
    case gli::TARGET_2D:
    case gli::TARGET_CUBE:
        glTexStorage2D(
            Target, static_cast<GLint>(Texture.levels()), Format.Internal,
            Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
        break;
    case gli::TARGET_2D_ARRAY:
    case gli::TARGET_3D:
    case gli::TARGET_CUBE_ARRAY:
        glTexStorage3D(
            Target, static_cast<GLint>(Texture.levels()), Format.Internal,
            Extent.x, Extent.y,
            Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
        break;
    default:
        assert(0);
        break;
    }

    for (std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
        for (std::size_t Face = 0; Face < Texture.faces(); ++Face)
            for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
            {
                GLsizei const LayerGL = static_cast<GLsizei>(Layer);
                /*glm::tvec3<GLsizei>*/ Extent = Texture.extent(Level);
                Target = gli::is_target_cube(Texture.target())
                    ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
                    : Target;

                switch (Texture.target())
                {
                case gli::TARGET_1D:
                    if (gli::is_compressed(Texture.format()))
                        glCompressedTexSubImage1D(
                            Target, static_cast<GLint>(Level), 0, Extent.x,
                            Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                            Texture.data(Layer, Face, Level));
                    else
                        glTexSubImage1D(
                            Target, static_cast<GLint>(Level), 0, Extent.x,
                            Format.External, Format.Type,
                            Texture.data(Layer, Face, Level));
                    break;
                case gli::TARGET_1D_ARRAY:
                case gli::TARGET_2D:
                case gli::TARGET_CUBE:
                    if (gli::is_compressed(Texture.format()))
                        glCompressedTexSubImage2D(
                            Target, static_cast<GLint>(Level),
                            0, 0,
                            Extent.x,
                            Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
                            Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                            Texture.data(Layer, Face, Level));
                    else
                        glTexSubImage2D(
                            Target, static_cast<GLint>(Level),
                            0, 0,
                            Extent.x,
                            Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
                            Format.External, Format.Type,
                            Texture.data(Layer, Face, Level));
                    break;
                case gli::TARGET_2D_ARRAY:
                case gli::TARGET_3D:
                case gli::TARGET_CUBE_ARRAY:

                    if (gli::is_compressed(Texture.format()))
                        glCompressedTexSubImage3D(
                            Target, static_cast<GLint>(Level),
                            0, 0, 0,
                            Extent.x, Extent.y,
                            Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
                            Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                            Texture.data(Layer, Face, Level));
                    else
                        glTexSubImage3D(
                            Target, static_cast<GLint>(Level),
                            0, 0, 0,
                            Extent.x, Extent.y,
                            Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
                            Format.External, Format.Type,
                            Texture.data(Layer, Face, Level));
                    break;
                default: assert(0); break;
                }
            }

    _texture.textureID = TextureName;
    _texture.pixelDimension = glm::vec2(Extent);
}

GLuint Texture_Manager::CreateSkyboxTexture(char const* Filename)
{
    std::string FilenameStr(Filename);

    std::string left = FilenameStr + "_top.dds";
    std::string top = FilenameStr + "_bottom.dds";

    std::vector<std::string> faces
    {
        left,left,top,top,left,left
    };

    GLuint Skybox_Tex = 0;
    glGenTextures(1, &Skybox_Tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox_Tex);

    //int width{}, height{}, nrChannels{};
    //unsigned int err = 0;

    for (size_t i = 0; i < faces.size(); i++)
    {
        gli::texture Texture = gli::load(faces[i]);
        gli::gl GL(gli::gl::PROFILE_GL33);
        gli::gl::format Format = GL.translate(Texture.format(), Texture.swizzles());
        auto& form = Format.Internal;

        glCompressedTexImage2D(
            (GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i),
            0,
            GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ,
            Texture.extent().x,
            Texture.extent().y,
            0,
            GLsizei(Texture.size()),
            Texture.data());

        GLenum code = glGetError();
    }
    //GL_COMPRESSED_SRGB_S3TC_DXT1_EXT

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return Skybox_Tex;
}

GLuint Texture_Manager::GetTexture(const Engine::GUID<TextureAsset>& GUID)
{
    if ((mTextureContainer.find(GUID) != mTextureContainer.end())) {
        return mTextureContainer.find(GUID)->second.textureID;
    }

    return 0;
}

BaseTexture* Texture_Manager::GetBaseTexture(const Engine::GUID<TextureAsset>& GUID)
{
    if ((mTextureContainer.find(GUID) != mTextureContainer.end())) {
        return &mTextureContainer.find(GUID)->second;
    }

    return nullptr;
}

GLuint Texture_Manager::GetTexture(const fs::path& filePath)
{
    GetAssetEvent<TextureAsset> e{filePath};
    EVENTS.Publish(&e);
    return GetTexture(e.guid);
}

void Texture_Manager::CallbackTextureAssetLoaded(AssetLoadedEvent<TextureAsset>* pEvent)
{
     AddTexture(pEvent->asset.mFilePath.string().c_str(), pEvent->asset.importer->guid);
}

void Texture_Manager::CallbackTextureAssetUnloaded(AssetUnloadedEvent<TextureAsset>* pEvent)
{

}