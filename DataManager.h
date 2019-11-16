#pragma once
#include <map>
#include <optional>
#include <bitset>
#include <set>
#include <string>
#include "GraphicsObjects.h"
#include "GraphicsComponent.h"

namespace Info
{
	struct ModelInfo
	{
		// model
		std::optional<std::string> modelPath;
		// or raw
		const GO::TypedVertices* vertices = nullptr;
		const GO::Indices* indices = nullptr;
		std::string texturePath;
	};

	struct VerticesInfo
	{
		GO::VertexType verticesType;
	};
}


struct ModelReference
{
	std::optional<std::string> file;

	const GO::ByteVertices* pVertices;
	std::optional<const GO::Indices*> pIndices;
	std::optional<const std::string> texture;
};
namespace Comparators
{
	struct ModelReferenceCompLess
	{
		bool operator()(const ModelReference& lhs, const ModelReference& rhs) const
		{
			bool returnVal = false;
			if (lhs.file && rhs.file)
			{
				returnVal =  lhs.file.value() < rhs.file.value();
			}
			else if (!lhs.file && rhs.file)
			{
				returnVal =  false;
			}
			else if (lhs.file && !rhs.file)
			{
				returnVal =  true;
			}
			else
			{
				if (ByteVerticesCompEqual()(*lhs.pVertices, *rhs.pVertices))
				{
					bool lessIndices = false;
					{
						if (lhs.pIndices.has_value() && rhs.pIndices.has_value())
							lessIndices = IndicesCompLess()(*lhs.pIndices.value(), *rhs.pIndices.value());
						else if (!lhs.pIndices && rhs.pIndices)
							lessIndices = false;
						else
							lessIndices = true;
					}
					bool lessTexture = false;
					{
						if (lhs.texture.has_value() && rhs.texture.has_value())
							lessTexture = lhs.texture.value() < rhs.texture.value();
						else if (!lhs.texture && rhs.texture)
							lessTexture = false;
						else
							lessTexture = true;
					}

					returnVal =  lessIndices || lessTexture;
				}
				else
				{
					returnVal =  ByteVerticesCompLess()(*lhs.pVertices, *rhs.pVertices);
				}
			}

			return returnVal;
		}
	};
	struct ModelReferenceCompEqual
	{
		bool operator()(const ModelReference& lhs, const ModelReference& rhs) const
		{
			if(lhs.file == rhs.file)
				return ByteVerticesCompEqual()(*lhs.pVertices, *rhs.pVertices);

			return false;
		}
	};

}
class DataManager
{
private:
	template <typename c1, typename c2>
	bool sizeContentCompare(const c1& c1, const c2& c2) const;
	template <typename c1, typename c2, typename pr>
	bool sizeContentCompare(const c1& c1, const c2& c2, pr pred) const;
	using ByteVerticesSet = std::set<GO::ByteVertices, Comparators::ByteVerticesCompLess>;
	using IndicesSet = std::set<GO::Indices, Comparators::IndicesCompLess>;
	using ModelReferenceSet = std::set<ModelReference, Comparators::ModelReferenceCompLess>;

	GO::Model processModelInfo(const Info::ModelInfo& info) const;
	std::optional<const ModelReference*> modelLoaded(const GO::Model& model) const;
	const GO::ByteVertices* loadVertices(const GO::ByteVertices& vertices);
	const GO::Indices* loadIndices(const GO::Indices& indices);
	std::string loadTexture(const std::string& textureFile);

	const GO::ByteVertices* findVertices(const GO::ByteVertices& toFind) const;
	const GO::Indices* findIndices(const GO::Indices& toFind) const;
	const std::string* findTexture(const std::string& file) const;
	std::optional<const ModelReference*> findSuitableModelReference(
		const GO::ByteVertices* verts, 
		const GO::Indices* inds, 
		const std::string* text) const;
public:
	const ModelReference* getModelReference(const Info::ModelInfo& info);

	size_t getLoadedVerticesByteSize(GO::VertexType type) const;
	size_t getLoadedIndicesSize() const;
	//std::pair<size_t, size_t> getIndicesCountAndOffsetFromModelReference(const ModelReference* modelRef) const;
	//std::pair<size_t, size_t> getVerticesCountAndOffsetFromModelReference(const ModelReference* modelRef) const;
	//private:
	struct
	{
		ModelReferenceSet models;
		ByteVerticesSet vertices;
		IndicesSet indices;
		std::set<std::string> textures;
	} loaded;

	// for drawing purposes
	void setIndicesOffset(const GO::Indices* inds, uint64_t offset);
	void setVerticesOffset(const GO::ByteVertices* verts, uint64_t byteOffset);
	uint64_t getVerticesOffset(const GO::ByteVertices* verts) const;
	uint64_t getIndicesOffset(const GO::Indices* inds) const;
	struct
	{
		std::map<const GO::ByteVertices*, uint64_t> vertices;
		std::map<const GO::Indices*, uint64_t> indices;
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
