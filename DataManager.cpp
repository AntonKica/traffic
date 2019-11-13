#include "DataManager.h"
#include "ModelLoader.h"
#include "Utilities.h"

#include <filesystem>

using namespace GO;
using namespace Utility;

static std::string narrowFileName(std::string_view fileName)
{
	return std::filesystem::path(fileName).filename().string();
}

static bool identicalVertex(const VariantVertex& v1, const VariantVertex v2)
{
	return std::memcmp(&v1, &v2, sizeof(VariantVertex)) == 0;
};

template<typename M_IDV>
inline size_t getLoadedTotalSize(M_IDV c)
{
	size_t totalSize = 0;
	for (const auto& [id, vector] : c)
	{
		totalSize += vector.size();
	}
	return totalSize;
}


bool DataManager::identicalTypedVerticles(const GO::TypedVertices& lhs, const GO::TypedVertices& rhs) const
{
	const auto& [lhsType, lhsVertices] = lhs;
	const auto& [rhsType, rhsVertices] = rhs;
	if (lhsType == rhsType)
	{
		if (sizeContentCompare(lhsVertices, rhsVertices, identicalVertex))
			return true;
	}

	return false;
}

DataManager::IDList DataManager::findListOfIdenticalVertices(const GO::TypedVertices& verticesToFind) const
{
	IDList verticesIDs;
	// go trough all vertices
	for (const auto& [id, typedVertices] : loaded.typedVertices)
	{
		if (identicalTypedVerticles(verticesToFind, typedVertices))
		{
			verticesIDs.push_back(id);
		}
	}

	return verticesIDs;
}

DataManager::IDList DataManager::findListOfIdenticalIndices(const GO::Indices& indicesToFind) const
{
	DataManager::IDList indicesIDs;
	// go trough all indices
	for (const auto& [id, indices] : loaded.indices)
	{
		if (sizeContentCompare(indicesToFind, indices))
		{
			indicesIDs.push_back(id);
		}
	}

	return indicesIDs;
}

DataManager::IDList DataManager::findListOfIdenticalTexture(const std::string& imageFileToFind) const
{
	DataManager::IDList texturesIDs;
	// only file
	for (const auto& [id, file] : loaded.textures)
	{
		if (imageFileToFind == file)
		{
			texturesIDs.push_back(id);
		}
	}

	return texturesIDs;
}

std::optional<GO::ID> DataManager::findIdenticalVertices(const GO::TypedVertices& verticesToFind) const
{
	std::optional<GO::ID> verticesID;
	// go trough all vertices
	for (const auto& [id, typedVertices] : loaded.typedVertices)
	{
		if (identicalTypedVerticles(verticesToFind, typedVertices))
		{
			verticesID = id;
			break;
		}
	}

	return verticesID;
}

std::optional<GO::ID> DataManager::findIdenticalIndices(const GO::Indices& indicesToFind) const
{
	std::optional<GO::ID> indicesID;
	// go trough all indices
	for (const auto& [id, indices] : loaded.indices)
	{
		if (sizeContentCompare(indicesToFind, indices))
		{
			indicesID = id;
		}
	}

	return indicesID;
}

std::optional<GO::ID> DataManager::findIdenticalTexture(const std::string& imageFileToFind) const
{
	std::optional<GO::ID> texturesID;
	// only file
	for (const auto& [id, file] : loaded.textures)
	{
		if (imageFileToFind == file)
		{
			texturesID = id;
		}
	}

	return texturesID;
}

std::optional<GO::ID> DataManager::findIdenticalModelReference(IDList vertices, IDList indices, IDList textures) const
{
	std::optional<GO::ID> modelRefId;

	// models IDS with suitable
	IDList suitableVerts;
	IDList suitableInds;
	IDList suitableTexs;
	for (const auto& [id, modelRef] : loaded.models)
	{
		{
			auto suitVert = std::find(vertices.begin(), vertices.end(), modelRef.vertices);
			if (suitVert != vertices.end())
				suitableVerts.push_back(*suitVert);
			else
				continue; // no point in findListOfing other attributes
		}
		{
			if (!modelRef.indices && indices.empty())
			{
				suitableInds.push_back(id);
			}
			else if (modelRef.indices && indices.size() > 0)
			{
				auto suitInd = std::find(indices.begin(), indices.end(), modelRef.indices);
				if (suitInd != indices.end())
					suitableInds.push_back(*suitInd);
			}
		}
		{
			// no texture
			if (!modelRef.textureRefID && textures.empty())
			{
				suitableTexs.push_back(id);
			}
			// texture
			else if (modelRef.textureRefID && textures.size() > 0)
			{
				auto suitTex = std::find(textures.begin(), textures.end(), modelRef.textureRefID);
				if (suitTex != textures.end())
					suitableTexs.push_back(*suitTex);
			}
		}
	}

	IDList commonVerInd;
	std::set_intersection(suitableVerts.begin(), suitableVerts.end(),
		suitableInds.begin(), suitableInds.end(), std::back_inserter(commonVerInd));

	IDList commonVertIndTex;
	std::set_intersection(commonVerInd.begin(), commonVerInd.end(),
		suitableTexs.begin(), suitableTexs.end(), std::back_inserter(commonVertIndTex));

	if (commonVertIndTex.size() > 0)
	{
		modelRefId = commonVertIndTex[0];
	}

	return modelRefId;
}

GO::ID DataManager::processModelInfo(const Info::ModelInfo& info)
{
	GO::Model newModel;
	if (info.modelPath)
	{
		newModel = ModelLoader::loadModel(info.modelPath.value());
	}
	else
	{
		if (!info.vertices)
		{
			throw std::runtime_error("No vertices supplied!\n");
		}

		newModel.typedVertices = *info.vertices;
		if (info.indices)
			newModel.indices = *info.indices;
		if (!info.texturePath.empty())
			newModel.texturePath = info.texturePath;
	}
	return loadModel(newModel);
}

ID DataManager::loadModel(const Model& model)
{
	std::optional<ID> loadedModel = modelLoaded(model);
	if (loadedModel)
	{
		return loadedModel.value();
	}

	// create reference
	ModelReference ref;
	ref.file = model.file;
	ref.vertices = loadVertices(model.typedVertices, false);
	ref.verticesType = model.typedVertices.first;

	// has indices?
	if (model.indices)
		ref.indices = loadIndices(model.indices.value(), false);
	// has texture?
	if (model.texturePath)
		ref.textureRefID = loadTexture(model.texturePath.value(), false);

	// register
	ID refId = generateNextContainerID(loaded.models);
	loaded.models[refId] = ref;

	setState(MODEL_LOADED, true);
	return refId;
}

std::optional<ID> DataManager::modelLoaded(const GO::Model& model) const
{
	std::optional<ID> modelIndex;
	bool foundSomething = false;
	// compare via file or 
	if (!model.file.empty())
	{
		for (const auto& [index, loadedModel] : loaded.models)
		{
			if (model.file == loadedModel.file)
			{
				modelIndex = index;
				return modelIndex;
			}
		}
	}

	//collect data
	DataManager::IDList suitableVertices;
	DataManager::IDList suitableIndices;
	DataManager::IDList suitableTextures;
	{
		suitableVertices = findListOfIdenticalVertices(model.typedVertices);
		if (suitableVertices.size() > 0)
		{
			if (model.indices)
				suitableIndices = findListOfIdenticalIndices(model.indices.value());

			if (model.texturePath)
				suitableTextures = findListOfIdenticalTexture(model.texturePath.value());
			foundSomething = true;
		}
	}

	if (foundSomething)
		modelIndex = findIdenticalModelReference(suitableVertices, suitableIndices, suitableTextures);

	return modelIndex;
}

ID DataManager::loadIndices(const Indices& indices, bool checkLoaded)
{
	if (checkLoaded)
	{
		auto identicalIndices = findIdenticalIndices(indices);
		if (identicalIndices)
			return identicalIndices.value();
	}

	ID newId = generateNextContainerID(loaded.indices);
	loaded.indices[newId] = indices;

	setState(INDICES_LOADED, true);
	return newId;
}

ID DataManager::loadVertices(const TypedVertices& typedVertices, bool checkLoaded)
{
	const auto& [vertType, vertices] = typedVertices;
	if (checkLoaded)
	{
		auto identicalVertices = findIdenticalVertices(typedVertices);
		if (identicalVertices)
			return identicalVertices.value();
	}

	ID newId = generateNextContainerID(loaded.typedVertices);
	loaded.typedVertices[newId] = typedVertices;

	setState(VERTICES_LOADED, true);
	return newId;
}

GO::ID DataManager::loadTexture(const std::string& textureFile, bool checkLoaded)
{
	if (checkLoaded)
	{
		auto identicalTexture = findIdenticalTexture(textureFile);
		if (identicalTexture)
			return identicalTexture.value();
	}

	ID newId = generateNextContainerID(loaded.textures);
	loaded.textures[newId] = textureFile;

	setState(TEXTURE_LOADED, true);
	return newId;
}

size_t DataManager::getLoadedVerticesSize(GO::VertexType type) const
{
	size_t loadedCount = 0;
	for (const auto& [id, typedVertex] : loaded.typedVertices)
	{
		const auto& [_type, vertices] = typedVertex;

		if (_type == type)
			loadedCount += vertices.size();
	}
	return loadedCount;
}

size_t DataManager::getLoadedIndicesSize() const
{
	size_t loadedCount = 0;
	for (const auto& [id, indices] : loaded.indices)
	{
		loadedCount += indices.size();
	}

	return loadedCount;
}

const GO::ModelReference& DataManager::getModelReference(GO::ID id) const
{
	auto modelIT = loaded.models.find(id);
	if (modelIT == std::end(loaded.models))
		throw std::runtime_error("Unknowns model" + std::to_string(id));

	return modelIT->second;
}

std::string DataManager::getTexturePath(GO::ID id) const
{
	auto textIT = loaded.textures.find(id);
	if (textIT == std::end(loaded.textures))
		throw std::runtime_error("Unknowns model" + std::to_string(id));

	return textIT->second;
}

std::pair<size_t, size_t> DataManager::getIndicesCountAndOffsetFromModelReference(GO::ID modelID) const
{
	auto modelRef = getModelReference(modelID);
	size_t count = loaded.indices.find(modelRef.indices.value())->second.size();
	size_t offset = offsets.indices.find(modelRef.indices.value())->second;

	return std::make_pair(count, offset);
}

std::pair<size_t, size_t> DataManager::getVerticesCountAndOffsetFromModelReference(GO::ID modelID) const
{
	auto& modelRef = getModelReference(modelID);

	// lol, syntax
	size_t count = loaded.typedVertices.find(modelRef.vertices)->second.second.size();
	const auto& [id, vertOffPair] = *offsets.typedVertices.find(modelRef.vertices);
	size_t offset = vertOffPair.second;

	return std::make_pair(count, offset);
}

void DataManager::setIndicesOffset(GO::ID id, uint64_t offset)
{
	offsets.indices[id] = offset;
}

void DataManager::setVerticesOffset(GO::ID id, GO::VertexType type ,uint64_t offset)
{
	// ugly
	offsets.typedVertices[id].first = type;
	offsets.typedVertices[id].second = offset;
}

bool DataManager::getState(Flags flag) const
{
	return stateFlags[flag];
}

void DataManager::setState(Flags flag, bool value)
{
	stateFlags.set(flag, value);
}
