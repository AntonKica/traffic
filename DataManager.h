#pragma once
#include <map>
#include <optional>
#include <bitset>
#include "GraphicsObjects.h"
#include "GraphicsComponent.h"

class DataManager
{
private:
	template <typename c1, typename c2> 
	bool sizeContentCompare(const c1& c1, const c2& c2) const;
	template <typename c1, typename c2, typename pr> 
	bool sizeContentCompare(const c1& c1, const c2& c2, pr pred) const;

	bool identicalTypedVerticles(const GO::TypedVertices& lhs, const GO::TypedVertices& rhs) const;

	using IDList = std::vector<GO::ID>;
	IDList findListOfIdenticalVertices(const GO::TypedVertices& verticesToFind) const;
	IDList findListOfIdenticalIndices(const GO::Indices& indicesToFind) const;
	IDList findListOfIdenticalTexture(const std::string& imageFileToFind) const;
	std::optional<GO::ID> findIdenticalVertices(const GO::TypedVertices& verticesToFind) const;
	std::optional<GO::ID> findIdenticalIndices(const GO::Indices& indicesToFind) const;
	std::optional<GO::ID> findIdenticalTexture(const std::string& imageFileToFind) const;
	std::optional<GO::ID> findIdenticalModelReference(IDList vertices, IDList indices, IDList textures) const;
public:
	GO::ID processModelInfo(const Info::ModelInfo& info);
	GO::ID loadModel(const GO::Model& model);;
	std::optional<GO::ID> modelLoaded(const GO::Model& model) const;
	GO::ID loadVertices(const GO::TypedVertices& vertices, bool checkLoaded = true);
	GO::ID loadIndices(const GO::Indices& indices, bool checkLoaded = true);
	GO::ID loadTexture(const std::string& textureFile, bool checkLoaded = true);

	size_t getLoadedVerticesSize(GO::VertexType type) const;
	size_t getLoadedIndicesSize() const;
	const GO::ModelReference& getModelReference(GO::ID id) const;
	std::string getTexturePath(GO::ID id) const;
	std::pair<size_t, size_t> getIndicesCountAndOffsetFromModelReference(GO::ID modelID) const;
	std::pair<size_t, size_t> getVerticesCountAndOffsetFromModelReference(GO::ID modelID) const;
	//private:
	struct
	{
		std::map<GO::ID, GO::ModelReference> models;
		std::map<GO::ID, GO::TypedVertices> typedVertices;
		std::map<GO::ID, GO::Indices> indices;
		std::map<GO::ID, std::string> textures;
	} loaded;

	// for drawing purposes
	void setIndicesOffset(GO::ID id, uint64_t offset);
	void setVerticesOffset(GO::ID id, GO::VertexType type, uint64_t offset);
	struct
	{
		std::map<GO::ID, std::pair<GO::VertexType, uint64_t>> typedVertices;
		std::map<GO::ID, uint64_t> indices;
	} offsets;

	enum Flags
	{
		MODEL_LOADED = 0,
		VERTICES_LOADED,
		INDICES_LOADED,
		TEXTURE_LOADED,
		MAX_FLAGS
	};
	std::bitset<MAX_FLAGS> stateFlags;
	bool getState(Flags flag) const;
	//bool getState(Flags flag, bool switchValue);
	void setState(Flags flag, bool value);
};

template<typename c1, typename c2>
bool DataManager::sizeContentCompare(const c1& c1, const c2& c2) const
{
	auto lmbd = [&](auto lhs, auto rhs) { return lhs == rhs; };
	return sizeContentCompare(c1, c2, lmbd);
}

template<typename c1, typename c2, typename pr>
bool DataManager::sizeContentCompare(const c1& c1, const c2& c2, pr pred) const
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
