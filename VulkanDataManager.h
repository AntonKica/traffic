#pragma once
#include <map>
#include <optional>
#include <bitset>
#include "GraphicsObjects.h"

class DataManager
{
private:
	template <typename c1, typename c2> bool sizeContentCompare(c1 c1, c2 c2);
	template <typename c1, typename c2, typename pr> bool sizeContentCompare(c1 c1, c2 c2, pr pred);

	template <typename V> size_t generateNextContainerID(V c);

public:
	template <typename M_IDV> size_t getLoadedTotalSize(M_IDV c);

	GO::ID loadModel(const std::string& modelPath);;
	std::optional<GO::ID> modelLoaded(std::string_view modelPath) const;
	GO::ID loadIndices(const GO::Indices& indices, bool checkLoaded = true);
	GO::ID loadTexturedVertices(const GO::TexturedVertices& vertices, bool checkLoaded = true);
	GO::ID loadTexture(const std::string& textureFile, bool checkLoaded = true);
	//private:
	struct
	{
		std::map<GO::ID, GO::ModelReference> models;
		std::map<GO::ID, GO::TexturedVertices> texturedVertices;
		std::map<GO::ID, GO::Indices> indices;
		std::map<GO::ID, std::string> textures;
	} m_loaded;

	// for drawing purposes
	void setIndicesOffset(GO::ID id, uint64_t offset);
	void setTexturedVerticesOffset(GO::ID id, uint64_t offset);
	struct
	{
		std::map<GO::ID, uint64_t> texturedVertices;
		std::map<GO::ID, uint64_t> indices;
	} m_offset;

	enum Flags
	{
		MODEL_LOADED = 0,
		TEXTURED_VERT_LOADED,
		INDICES_LOADED,
		TEXTURE_LOADED,
		MAX_FLAGS
	};
	std::bitset<MAX_FLAGS> m_stateFlags;
	bool getState(Flags flag) const;
	//bool getState(Flags flag, bool switchValue);
	void setState(Flags flag, bool value);
};

template<typename c1, typename c2>
bool DataManager::sizeContentCompare(c1 c1, c2 c2)
{
	auto lmbd = [&](auto lhs, auto rhs) { return lhs == rhs; };
	return sizeContentCompare(c1, c2, lmbd);
}

template<typename c1, typename c2, typename pr>
bool DataManager::sizeContentCompare(c1 c1, c2 c2, pr pred)
{
	if (c1.size() == c2.size())
	{
		if (std::equal(c1.begin(), c1.end(), c2.begin(), pred))
		{
			return true;
		}
	}

	return false;
}

template<typename V>
size_t DataManager::generateNextContainerID(V c)
{
	return c.size();
}

template<typename M_IDV>
inline size_t DataManager::getLoadedTotalSize(M_IDV c)
{
	size_t totalSize = 0;
	for(const auto&[id, vector] : c)
	{
		totalSize += vector.size();
	}
	return totalSize;
}
