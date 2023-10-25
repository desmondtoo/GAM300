#pragma once

#include "AssetTypes.h"
#include "Core/EventsManager.h"
#include <vector>
#include <unordered_set>
#include <Utilities/YAMLUtils.h>
#include <Utilities/Serializer.h>

//Hash table that maps an ID to a pointer
template <typename T>
using AssetsTable = std::unordered_map<Engine::GUID, T>;

namespace chron = std::chrono;

enum AssetState
{
	ASSET_LOADED,
	ASSET_UPDATED,
	ASSET_UNLOADED
};

template <typename T>
using AssetsBuffer = std::vector<std::pair<AssetState,T*>>;

//Hash table of single component types
template<typename... Ts>
struct AllAssetsGroup
{
	constexpr AllAssetsGroup(TemplatePack<Ts...>) {}
	AllAssetsGroup() = default;
	template<typename T>
	auto& GetAssets()
	{
		return std::get<AssetsTable<T>>(assets);
	}

	void AddAsset(const std::filesystem::path& filePath, FileData* pData = nullptr)
	{
		size_t assetType = AssetExtensionTypes[filePath.extension().string()];
		if (([&](auto type)
			{
				using T = decltype(type);
				if (GetAssetType::E<T>() == assetType)
				{
					Engine::GUID guid = GetGUID(filePath);
					if constexpr (std::is_base_of<Asset, T>())
					{
						if (fs::is_directory(filePath))
							return true;
						T& asset{ std::get<AssetsTable<T>>(assets)[guid]};
						asset.mFilePath = filePath;
						//Pre-existing data
						if (pData)
						{
							asset.mData = std::move(*pData);
						}
						else
						{
							//load data
							std::ifstream inputFile(filePath.c_str());
							E_ASSERT(inputFile, "Error opening file to update asset in memory!");
							asset.mData.assign(
								std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
							PRINT("Done adding ", filePath, " into memory!", '\n');
							inputFile.close();
						}
						std::get<AssetsBuffer<T>>(assetsBuffer).emplace_back(std::make_pair(ASSET_LOADED,&asset));
					}
					else
					{
						std::get<AssetsTable<T>>(assets)[guid];
					}
					return true;
				}
				return false;
			}
			(Ts{}) || ...))
		{
			return;
		}
	}

	bool RemoveAsset(const std::filesystem::path& filePath)
	{
		size_t assetType = AssetExtensionTypes[filePath.extension().string()];
		if (([&](auto type)
			{
				using T = decltype(type);
				if (GetAssetType::E<T>() == assetType)
				{
					Engine::GUID guid = GetGUID(filePath);
					T& asset{ std::get<AssetsTable<T>>(assets)[guid] };
					T* tempAsset = new T;
					tempAsset->mFilePath = filePath;
					std::get<AssetsTable<T>>(assets).erase(guid);
					std::get<AssetsBuffer<T>>(assetsBuffer).emplace_back(std::make_pair(ASSET_UNLOADED, tempAsset));
					return true;
				}
				return false;
			}
			(Ts{}) || ...))
		{
			return true;
		}
		return false;
	}

	void UpdateAsset(const std::filesystem::path& filePath)
	{
		size_t assetType = AssetExtensionTypes[filePath.extension().string()];
		if (([&](auto type)
			{
				using T = decltype(type);
				if (GetAssetType::E<T>() == assetType)
				{
					Engine::GUID guid = GetGUID(filePath);
					if constexpr (std::is_base_of<Asset, T>())
					{
						std::ifstream inputFile(filePath.c_str());
						E_ASSERT(inputFile, "Error opening file to update asset in memory!");
						T& asset{ std::get<AssetsTable<T>>(assets)[guid] };
						asset.mFilePath = filePath;
						asset.mData.assign(
							std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
						PRINT("Done updating ", filePath, " in memory!", '\n');
						inputFile.close();
						std::get<AssetsBuffer<T>>(assetsBuffer).emplace_back(std::make_pair(ASSET_UPDATED, &asset));
					}
					else
					{
						std::get<AssetsTable<T>>(assets)[guid] = {};
					}
					return true;
				}
				return false;
			}
			(Ts{}) || ...))
		{
			return;
		}
	}

	void RenameAsset(const std::filesystem::path& oldPath, const std::filesystem::path& newPath)
	{
		FileData* fileData = GetFileData(oldPath);

		//Deal with meta file conversion here
		if (AssetExtensionTypes[oldPath.extension()] != AssetExtensionTypes[newPath.extension()])
		{

		}
		else
		{
			fs::path oldMeta{ oldPath };
			oldMeta += ".meta";
			fs::path newMeta{ newPath };
			newMeta += ".meta";
			fs::rename(oldMeta, newMeta);
		}

		AddAsset(newPath);
		RemoveAsset(oldPath);
	}

	void ProcessBuffer()
	{
		(([&](auto type) 
		{
			using T = decltype(type);
			auto& buffer{std::get<AssetsBuffer<T>>(assetsBuffer)};
			for (auto& pair : buffer)
			{
				fs::path path{pair.second->mFilePath };
				switch (pair.first)
				{
					case ASSET_LOADED:
					{
						AssetLoadedEvent<T> e{ path,GetGUID(path),*pair.second };
						EVENTS.Publish(&e);
						break;
					}
					case ASSET_UPDATED:
					{
						AssetUpdatedEvent<T> e{ path,GetGUID(path),*pair.second };
						EVENTS.Publish(&e);
						break;
					}
					case ASSET_UNLOADED:
					{
						
						AssetUnloadedEvent<T> e{ path,GetGUID(path)};
						EVENTS.Publish(&e);
						fs::path metaPath = path;
						metaPath += ".meta";
						//Actual file does not exist
						if (std::filesystem::exists(metaPath))
						{
							std::filesystem::remove(metaPath);
							delete pair.second;
						}
						break;
					}
					default:
					{
						E_ASSERT(false, "Invalid asset state");
						break;
					}
				}
			}
			buffer.clear();
		})(Ts{}), ...);
	}



	Engine::GUID GetGUID(const std::filesystem::path& filePath)
	{
		std::filesystem::path metaPath = filePath;
		metaPath += ".meta";
		if (!std::filesystem::exists(metaPath))
			CreateMeta(filePath);
		MetaFile mFile = Deserialize<FolderMeta>(metaPath);
		Engine::GUID guid;
		AddToMeta(metaPath, "guid", guid);
		return guid;
	}

	fs::path GetFilePath(const Engine::GUID& guid)
	{
		fs::path path;
		if (([&](auto type)
			{
				using T = decltype(type);
				auto& table = std::get<AssetsTable<T>>(assets);
				if (table.find(guid) != table.end())
				{
					path = table[guid].mFilePath;
					return true;
				}
				return false;
			}
		(Ts{}) || ...));
		return path;
	}

	void GetMeta(const std::filesystem::path& filePath)
	{
		std::filesystem::path metaPath = filePath;
		metaPath += ".meta";
		Engine::GUID guid;
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "guid" << YAML::Value << guid.ToHexString();
		auto duration = fs::last_write_time(filePath).time_since_epoch();
		size_t seconds = chron::duration_cast<chron::seconds>(duration).count();
		out << YAML::Key << "lastModified" << YAML::Value << seconds;
		std::ofstream fs(metaPath);
		fs << out.c_str();
		fs.close();
		//// Mark meta files as hidden files
		const wchar_t* fileLPCWSTR = metaPath.wstring().c_str();
		int attribute = GetFileAttributes(fileLPCWSTR);
		if ((attribute & FILE_ATTRIBUTE_HIDDEN) == 0)
		{
			SetFileAttributes(fileLPCWSTR, attribute | FILE_ATTRIBUTE_HIDDEN);
		}
	}

	//Compare to meta file to see if file was modified while engine was closed
	bool IsModified(const std::filesystem::path& filePath)
	{
		std::filesystem::path metaPath = filePath;
		metaPath += ".meta";
		//If no meta, assume modified as it is just added
		if (!std::filesystem::exists(metaPath))
		{
			CreateMeta(filePath);
			return true;
		}
		std::vector<YAML::Node> data = YAML::LoadAllFromFile(metaPath.string());
		auto duration = fs::last_write_time(filePath).time_since_epoch();
		size_t seconds = chron::duration_cast<chron::seconds>(duration).count();
		for (YAML::Node& node : data)
		{
			if (node["lastModified"]) // Deserialize guid
			{
				if (node["lastModified"].as<size_t>() == seconds)
				{
					return false;
				}
				UpdateModifiedTime(metaPath,seconds);
				return true;
			}
		}
		//Failed to find node
		AddToMeta(metaPath,"lastModified", seconds);
		return true;
	}

	FileData* GetFileData(const std::filesystem::path& filePath)
	{
		size_t assetType = AssetExtensionTypes[filePath.extension().string()];
		FileData* pData;
		if (([&](auto type)
			{
				using T = decltype(type);
				if (GetAssetType::E<T>() == assetType)
				{
					Engine::GUID guid = GetGUID(filePath);
					if constexpr (std::is_base_of<Asset, T>())
					{
						T& asset{ std::get<AssetsTable<T>>(assets)[guid] };
						pData = &asset.mData;
					}
					return true;
				}
				return false;
			}
			(Ts{}) || ...))
		{
			return pData;
		}
		return pData;
	}
private:
	std::tuple<AssetsTable<Ts>...> assets;
	std::tuple<AssetsBuffer<Ts>...> assetsBuffer;
};

using AllAssets = decltype(AllAssetsGroup(AssetTypes()));