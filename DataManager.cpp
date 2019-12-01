#include "DataManager.h"
#include "ModelLoader.h"
#include "Utilities.h"

#include <string>
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


GO::Model DataManager::processModelInfo(const Info::ModelInfo& info) const
{
	GO::Model newModel;
	if (info.modelPath)
	{
		newModel = ModelLoader::loadModel(info.modelPath.value());
	}
	else
	{
		if (!info.vertices)
			throw std::runtime_error("No vertices supplied!\n");

		newModel.vertices = GO::transformTypedVerticesToBytes(*info.vertices);
		if (info.indices && info.indices->size() > 0)
			newModel.indices = *info.indices;
		if (!info.texturePath.empty())
			newModel.texturePath = info.texturePath;
	}

	return newModel;
}

/*void DataManager::addUsage(const ModelReference* model)
{
	addUsage(model, usageCounts.models);

	addUsage(model->pVertices, usageCounts.vertices);
	if(model->pIndices)
		addUsage(model->pIndices.value(), usageCounts.indices);
	if (model->texture)
		addUsage(model->texture.value(), usageCounts.textures);
}*/
/*
void DataManager::removeUsage(const ModelReference* model)
{
	removeUsage(model, usageCounts.models);

	removeVertices(model->pVertices);
	if (model->pIndices)
		removeIndices(model->pIndices.value());
	if (model->texture)
		removeTexture(model->texture.value());
}*/

std::shared_ptr<ModelReference> DataManager::getModelReference(const Info::ModelInfo& info)
{
	std::shared_ptr<ModelReference> retModel;
	GO::Model model = processModelInfo(info);

	/*auto loadedModel = modelLoaded(model);
	if (loadedModel)
	{
		retModel = loadedModel.value();
	}
	else*/
	{
		// create reference
		ModelReference ref;
		ref.file = model.getFile(model);
		ref.pVertices = loadVertices(model.vertices);

		// has indices?
		if (model.indices)
			ref.pIndices = loadIndices(model.indices.value());
		// has texture?
		if (model.texturePath)
			ref.texture = loadTexture(model.texturePath.value());
		retModel = std::make_shared<ModelReference>(ref);
		// register
		//const auto& [modelRef, inserted] = loaded.models.insert(ref);
		//retModel = &(*modelRef);
	}
	//addUsage(retModel);

	setState(MODEL_LOADED, true);
	return retModel;
}

std::shared_ptr<ModelReference> DataManager::copyModelReference(std::shared_ptr<ModelReference> reference)
{
	//addUsage(copyReference);

	return reference;
}
/*
void DataManager::removeModelReference(const ModelReference* reference)
{
	// find berore nulling
	auto mIter = loaded.models.find(*reference);

	removeUsage(reference);
	if (canRemove(reference, usageCounts.models))
	{
		loaded.models.erase(mIter);
	}
}*/
/*
std::optional<const ModelReference *> DataManager::modelLoaded(const GO::Model& model) const
{
	std::optional<const ModelReference*> modelRef;
	// compare via file or 
	//std::string file = std::filesystem::path(model.file).filename().string();
	if (model.directory)
	{
		std::string file = model.getFile(model).value();
		for (const auto& loadedModel : loaded.models)
		{
			if (loadedModel.file == file)
			{
				modelRef = &loadedModel;
				break;
			}
		}
	}

	if (!(modelRef && model.directory))
	{
		const GO::ByteVertices* verts = findVertices(model.vertices);

		const GO::Indices* inds = nullptr;
		if(model.indices)
			inds = findIndices(model.indices.value());

		const std::string* texts = nullptr;
		if (model.texturePath)
			texts = findTexture(model.texturePath.value());
		
		modelRef = findSuitableModelReference(verts, inds, texts);
	}

	return modelRef;
}*/

std::shared_ptr<GO::ByteVertices> DataManager::loadVertices(const GO::ByteVertices& byteVertices)
{
	auto ptr = std::make_shared<GO::ByteVertices>(byteVertices);

	loaded.vertices.push_back(ptr);

	/*const GO::ByteVertices* pVertices = &(*vts);

	setState(VERTICES_LOADED, true);
	//return const_cast<ByteVertices*>(&(*vts));
	return pVertices;*/
	setState(VERTICES_LOADED, true);
	return ptr;
}

std::shared_ptr<GO::Indices> DataManager::loadIndices(const Indices& indices)
{
	auto ptr = std::make_shared<GO::Indices>(indices);

	loaded.indices.push_back(ptr);
	/*const auto&[ids, inserted] = loaded.indices.insert(indices);

	const GO::Indices* pIndices = &(*ids);

	return pIndices;
	*/
	setState(INDICES_LOADED, true);
	return ptr;
}


std::string DataManager::loadTexture(const std::string& textureFile)
{
	//std::string file = std::filesystem::path(textureFile).filename().string();
	/*const auto& [pTexture, inserted] = loaded.textures.insert(textureFile);


	const std::string text = (*pTexture);
	*/
	setState(TEXTURE_LOADED, true);
	return textureFile;
}

void DataManager::removeVertices(const GO::ByteVertices* vertices)
{
	removeUsage(vertices, usageCounts.vertices);

	if (canRemove(vertices, usageCounts.vertices))
	{
		/*auto vIter = loaded.vertices.find(*vertices);
		loaded.vertices.erase(vIter);*/
	}
}

void DataManager::removeIndices(const GO::Indices* indices)
{
	removeUsage(indices, usageCounts.indices);

	if (canRemove(indices, usageCounts.indices))
	{
		/*auto iIter = loaded.indices.find(*indices);
		loaded.indices.erase(iIter);*/
	}
}

void DataManager::removeTexture(const std::string textureFile)
{
	removeUsage(textureFile, usageCounts.textures);

	if (canRemove(textureFile, usageCounts.textures))
	{
		/*auto tIter = loaded.textures.find(textureFile);
		loaded.textures.erase(tIter);*/
	}
}
/*
inline const GO::ByteVertices* DataManager::findVertices(const GO::ByteVertices& toFind) const
{
	/*auto vIter = loaded.vertices.find(toFind);

	return vIter != std::end(loaded.vertices) ? &(*vIter) : nullptr;
}

inline const GO::Indices* DataManager::findIndices(const GO::Indices& toFind) const
{
	auto iIter = loaded.indices.find(toFind);

	return iIter != std::end(loaded.indices) ? &(*iIter) : nullptr;
}

inline const std::string* DataManager::findTexture(const std::string& file) const
{
	auto tIter = loaded.textures.find(file);

	return tIter != std::end(loaded.textures) ? &(*tIter) : nullptr;
}
*/
/*
std::optional<const ModelReference*> DataManager::findSuitableModelReference(
	const GO::ByteVertices* verts,
	const GO::Indices* inds,
	const std::string* text) const
{
	std::optional<const ModelReference*> suitableModels;

	// vertices
	if (verts)
	{
		for (const auto& model : loaded.models)
		{
			if (Comparators::ByteVerticesCompEqual()(*verts, *model.pVertices))
			{
				bool identicalIndices = false;
				if (model.pIndices && inds)
					identicalIndices = Comparators::IndicesCompEqual()(*inds, *(model.pIndices.value()));
				else
					identicalIndices = (model.pIndices && inds);


				bool identicalTexture = false;
				if (model.texture && text)
					identicalTexture = model.texture == *text;
				else
					identicalTexture = (model.texture && text);

				if (identicalIndices && identicalTexture)
				{
					suitableModels = &model;
					break;
				}
			}
		}
	}
	return suitableModels;
}*/

size_t DataManager::getLoadedVerticesByteSize(GO::VertexType type) const
{
	size_t loadedCount = 0;
	for (const auto& loadedVertice : loaded.vertices)
	{
		const auto& [_type, verticesList] = *loadedVertice;
		if (_type == type)
			loadedCount += verticesList.size();
	}
	return loadedCount;
}

size_t DataManager::getLoadedIndicesSize() const
{
	size_t loadedCount = 0;
	for (const auto& indicesList : loaded.indices)
	{
		loadedCount += indicesList->size();
	}

	return loadedCount;
}

void DataManager::setIndicesOffset(const GO::Indices* inds, uint64_t offset)
{
	offsets.indices[inds] = offset;
}

void DataManager::setVerticesOffset(const GO::ByteVertices* verts, uint64_t byteOffset)
{
	offsets.vertices[verts] = byteOffset / GO::getVertexSize(verts->type);
}

uint64_t DataManager::getVerticesOffset(const GO::ByteVertices* verts) const
{
	auto vtOff = offsets.vertices.find(verts);
	if (vtOff == std::end(offsets.vertices))
		throw std::runtime_error("Unknown byteVertices!");

	return vtOff->second;
}

uint64_t DataManager::getIndicesOffset(const GO::Indices* inds) const
{
	auto iOff = offsets.indices.find(inds);
	if (iOff == std::end(offsets.indices))
		throw std::runtime_error("Unknown indices!");

	return iOff->second;
}

bool DataManager::getState(Flags flag) const
{
	return stateFlags[flag];
}

void DataManager::setState(Flags flag, bool value)
{
	stateFlags.set(flag, value);
}
