#pragma once
#include <map>
#include <optional>
#include <bitset>
#include <set>
#include <memory>
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

	std::shared_ptr<GO::ByteVertices> pVertices;
	std::shared_ptr<GO::Indices> pIndices;
	std::optional<std::string> texture;
};
namespace Comparators
{
	/*struct ModelReferenceCompLess
	{
		bool compare(const ModelReference* lhs, const ModelReference* rhs) const
		{
			bool returnVal = false;
			if (lhs->file && rhs->file)
			{
				returnVal = lhs->file.value() < rhs->file.value();
			}
			else if (!lhs->file && rhs->file)
			{
				returnVal = false;
			}
			else if (lhs->file && !rhs->file)
			{
				returnVal = true;
			}
			else
			{
				if (ByteVerticesCompEqual()(lhs->pVertices, rhs->pVertices))
				{
					bool lessIndices = false;
					{
						if (lhs->pIndices.has_value() && rhs->pIndices.has_value())
							lessIndices = IndicesCompLess()(*lhs->pIndices.value(), *rhs->pIndices.value());
						else if (!lhs->pIndices && rhs->pIndices)
							lessIndices = false;
						else if (lhs->pIndices && !rhs->pIndices)
							lessIndices = true;
						else
							lessIndices = false;
					}
					bool lessTexture = false;
					{
						if (lhs->texture.has_value() && rhs->texture.has_value())
							lessTexture = lhs->texture.value() < rhs->texture.value();
						else if (!lhs->texture && rhs->texture)
							lessTexture = false;
						else if (lhs->texture && !rhs->texture)
							lessTexture = true;
						else
							lessTexture = false;
					}

					returnVal = lessIndices || lessTexture;
				}
				else
				{
					returnVal = ByteVerticesCompLess()(*lhs->pVertices, *rhs->pVertices);
				}
			}

			return returnVal;
		}

		template<class model> bool operator()(const model& lhs, const model& rhs) const
		{
			if (!std::is_same<model, ModelReference>::value)
				static_assert("Uncompatible ByteVerticesContainers");

			return compare(ptr(lhs), ptr(rhs));
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
	};*/
}
class DataManager
{
private:
	template <typename c1, typename c2>
	bool sizeContentCompare(const c1& c1, const c2& c2) const;
	template <typename c1, typename c2, typename pr>
	bool sizeContentCompare(const c1& c1, const c2& c2, pr pred) const;
	//using ByteVerticesSet = std::multiset<GO::ByteVertices, Comparators::ByteVerticesCompLess>;
	//using IndicesSet = std::multiset<GO::Indices, Comparators::IndicesCompLess>;
	//using ModelReferenceSet = std::multiset<ModelReference, Comparators::ModelReferenceCompLess>;

	GO::Model processModelInfo(const Info::ModelInfo& info) const;
	//std::optional<const ModelReference*> modelLoaded(const GO::Model& model) const;
	//const GO::ByteVertices* loadVertices(const GO::ByteVertices& vertices);
	//const GO::Indices* loadIndices(const GO::Indices& indices);
	//std::string loadTexture(const std::string& textureFile);
	std::shared_ptr<GO::ByteVertices> loadVertices(const GO::ByteVertices& vertices);
	std::shared_ptr<GO::Indices> loadIndices(const GO::Indices& indices);
	std::string loadTexture(const std::string& textureFile);

	void lazyCleanup();

	void removeVertices(const GO::ByteVertices* vertices);
	void removeIndices(const GO::Indices* indices);
	void removeTexture(const std::string textureFile);

	const GO::ByteVertices* findVertices(const GO::ByteVertices& toFind) const;
	const GO::Indices* findIndices(const GO::Indices& toFind) const;
	const std::string* findTexture(const std::string& file) const;
	/*std::optional<const ModelReference*> findSuitableModelReference(
		const GO::ByteVertices* verts, 
		const GO::Indices* inds, 
		const std::string* text) const;
		*/
	struct
	{
		std::map<const GO::Indices*, size_t> indices;
		std::map<const GO::ByteVertices*, size_t> vertices;
		std::map<const ModelReference*, size_t> models;
		std::map<const std::string, size_t> textures;

	} usageCounts;
	//void addUsage(const ModelReference* model);
	template<class type, class container>
	//void addUsage(const type& t, container& c);
	void removeUsage(const ModelReference* model);
	template<class type, class container>
	void removeUsage(const type& t, container& c);
	template<class type, class container>
	bool canRemove(const type& t, container& c) const;

public:
	std::shared_ptr<ModelReference> getModelReference(const Info::ModelInfo& info);
	std::shared_ptr<ModelReference> copyModelReference(std::shared_ptr<ModelReference> reference);
	//void removeModelReference(const ModelReference* reference);

	size_t getLoadedVerticesByteSize(GO::VertexType type) const;
	size_t getLoadedIndicesSize() const;
	//std::pair<size_t, size_t> getIndicesCountAndOffsetFromModelReference(const ModelReference* modelRef) const;
	//std::pair<size_t, size_t> getVerticesCountAndOffsetFromModelReference(const ModelReference* modelRef) const;
	//private:
	/*struct
	{
		ModelReferenceSet models;
		ByteVerticesSet vertices;
		IndicesSet indices;
		std::set<std::string> textures;
	} loaded;*/
	struct
	{
		std::vector<std::shared_ptr<ModelReference>> models;
		std::vector<std::shared_ptr<GO::ByteVertices>> vertices;
		std::vector<std::shared_ptr<GO::Indices>>  indices;
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

/*ò/
template<class type, class container>
void DataManager::addUsage(const type& t, container& c)
{
	if (!std::is_same<container, std::map<type, size_t>>::value)
		static_assert("Container is not <idk, size_t>");
	if (!std::is_const<type>::value)
		static_assert("Non const");
	if (!std::is_pointer<type>::value)
		static_assert("Non Pointer");

	typename container::iterator iter = c.find(t);
	if (iter == std::end(c))
	{
		c[t] = 1;
	}
	else
	{
		++(iter->second);
	}
}*/

template<class type, class container>
void DataManager::removeUsage(const type& t, container& c)
{
	if (!std::is_same<container, std::map<type, size_t>>::value)
		static_assert("Container is not <idk, size_t>");
	if (!std::is_const<type>::value)
		static_assert("Non const");

	typename container::iterator iter = c.find(t);
	if (iter == std::end(c))
	{
		throw std::runtime_error("Removing usage from unknown type");
	}
	else
	{
		--(iter->second);
		// remove key as well
		if (iter->second == 0)
			c.erase(iter);
	}
}

template<class type, class container>
inline bool DataManager::canRemove(const type& t, container& c) const
{
	if (!std::is_same <container, std::map<type, size_t>>::value)
		static_assert("Container is not <idk, size_t>");
	if (!std::is_const<type>::value)
		static_assert("Non const");

	typename container::iterator iter = c.find(t);
	return iter == std::end(c);
}
