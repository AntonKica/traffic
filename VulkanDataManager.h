#pragma once
#include <utility>
#include <memory>
#include <map>

#include "VulkanData.h"
#include "Model.h"
#include "vulkanHelper/VulkanStructs.h"

class VulkanBase;
class VulkanDataManager
{
public:
	void initialize(VulkanBase* vkBase);
	void cleanup(const VkAllocationCallbacks* allocator);
	VD::ModelData loadModel(const Model& model);
	VD::ModelData loadModelFromPath(std::string modelPath);

	void lazyCleanup();
private:

	using SOBV = VD::SharedOffsetedByteVertices;
	VD::SharedOffsetedByteVertices loadVerticesFromMesh(const Mesh& mesh);
	VD::VertexFlags deduceMeshVerticesType(const Mesh& mesh) const;
	size_t deduceSizeFromVertexFlags(VD::VertexFlags flags) const;
	VD::ByteVertices transformMeshVerticesToByteVertices(const Mesh& mesh, VD::VertexFlags flags) const;


	using SOI = VD::SharedOffsetedIndices;
	VD::SharedOffsetedIndices loadIndicesFromMesh(const Mesh& mesh);

	std::map<VD::TextureType, VD::SharedTexture> loadTexturesFromModel(const Mesh& mesh);
	VD::SharedTexture loadTexture(std::string path);


	std::map<VD::VertexFlags, std::vector<VD::SharedOffsetedByteVertices>> vertices;
	std::vector<VD::SharedOffsetedIndices> indices;

	std::vector<VD::SharedTexture> textures;

	struct RecordedBuffer
	{
		vkh::structs::Buffer buffer{};
		size_t lastWrittenByte = 0;
	};

	//template <class SharedOffsetType> void addToBuffer(SharedOffsetType& sharedOffsetData, )
	void addDataToVertexBuffer(VD::SharedOffsetedByteVertices& data,
		RecordedBuffer& vertexBuffer);
	void recreateWholeVertexBufferFromSource(
		RecordedBuffer& vertexBuffer,
		const std::vector<VD::SharedOffsetedByteVertices>& source);

	//void addDataToIndexBuffer
	void addDataToIndexBuffer(VD::SharedOffsetedIndices& data,
		RecordedBuffer& indexBuffer);
	void recreateWholeIndexBufferFromSource(
		RecordedBuffer& indexBuffer, 
		const std::vector<VD::SharedOffsetedIndices>& source);

	std::map<VD::VertexFlags, RecordedBuffer> vertexBuffers;
	void addToVertexBuffer(VD::SharedOffsetedByteVertices& offsetByteVertices);


	RecordedBuffer indexBuffer;
	void addToIndexBuffer(VD::SharedOffsetedIndices& sharedOffsetIndices);

	// Model part
	struct LoadedModel
	{
		VD::ModelData data;
		std::string modelPath;
	};

	std::vector<LoadedModel> loadedModels;
	std::optional<VD::ModelData> findExistingModel(const std::string& modelPath) const;

	VD::SharedOffsetedByteVertices loadModelVerticesFromMesh(const Mesh& mesh);

	VD::SharedOffsetedIndices loadModelIndicesFromMesh(const Mesh& mesh);

	std::map<VD::VertexFlags, std::vector<VD::SharedOffsetedByteVertices>> modelVertices;
	std::vector<VD::SharedOffsetedIndices> modelIndices;

	std::map<VD::VertexFlags, RecordedBuffer> modelVertexBuffers;
	void addToModelVertexBuffer(VD::SharedOffsetedByteVertices& offsetByteVertices);

	RecordedBuffer modelIndexBuffer;
	void addToModelIndexBuffer(VD::SharedOffsetedIndices& sharedOffsetIndices);

	VulkanBase* vkBase;
	vkh::structs::VulkanDevice *device;
};

