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

	void lazyCleanup();
private:
	using SOBV = VD::SharedOffsetedByteVertices;

	VD::SharedOffsetedByteVertices loadVerticesFromMesh(const Mesh& mesh);
	VD::VertexFlags deduceMeshVerticesType(const Mesh& mesh) const;
	size_t deduceSizeFromVertexFlags(VD::VertexFlags flags) const;
	VD::ByteVertices transformMeshVerticesToByteVertices(const Mesh& mesh, VD::VertexFlags flags) const;

	using SOI = VD::SharedOffsetedIndices;
	VD::SharedOffsetedIndices loadIndicesFromMesh(const Mesh& mesh);

	std::map<VD::TextureType, VD::SharedTexture>  loadTexturesFromModel(const Mesh& mesh);
	VD::SharedTexture loadTexture(std::string path);

	
	std::map<VD::VertexFlags, std::vector<VD::SharedOffsetedByteVertices>> vertices;
	std::vector<VD::SharedOffsetedIndices> indices;

	std::vector<VD::SharedTexture> textures;

	struct RecordedBuffer
	{
		vkh::structs::Buffer buffer{};
		size_t lastWrittenByte = 0;
	};

	std::map<VD::VertexFlags, RecordedBuffer> vertexBuffers;
	void addToVertexBuffer(VD::SharedOffsetedByteVertices& offsetByteVertices);
	void recreateWholeVertexBuffer(VD::VertexFlags vertexType);

	//std::vector<VD::SharedModelData> modelDatas;

	RecordedBuffer indexBuffer;
	void addToIndexBuffer(VD::SharedOffsetedIndices& sharedOffsetIndices);
	void recreateWholeIndexBuffer();



	VulkanBase* vkBase;
	vkh::structs::VulkanDevice *device;
};

