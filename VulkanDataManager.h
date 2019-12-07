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
	VD::ModelData loadModel(const Model& model);

	void lazyCleanup();
private:
	using OffsetedSharedByteVertices = VD::OffsetBuffer<VD::SharedByteVertices>;
	using OSBV = OffsetedSharedByteVertices;

	OffsetedSharedByteVertices loadVerticesFromMesh(const Mesh& mesh);
	VD::VertexFlags deduceMeshVerticesType(const Mesh& mesh) const;
	size_t deduceSizeFromVertexFlags(VD::VertexFlags flags) const;
	VD::ByteVertices transformMeshVerticesToByteVertices(const Mesh& mesh, VD::VertexFlags flags) const;

	using OffsetedSharedIndices = VD::OffsetBuffer<VD::SharedIndices>;
	using OSI = OffsetedSharedIndices;
	OffsetedSharedIndices loadIndicesFromMesh(const Mesh& mesh);

	std::map<VD::TextureType, VD::SharedTexture>  loadTexturesFromModel(const Mesh& mesh);
	VD::SharedTexture loadTexture(std::string path);

	
	std::map<VD::VertexFlags, std::vector<OffsetedSharedByteVertices>> vertices;
	std::vector<OffsetedSharedIndices> indices;

	std::vector<VD::SharedTexture> textures;

	struct RecordedBuffer
	{
		vkh::structs::Buffer buffer{};
		size_t lastWrittenByte = 0;
	};

	std::map<VD::VertexFlags, RecordedBuffer> vertexBuffers;
	void addToVertexBuffer(OffsetedSharedByteVertices& offsetByteVertices);
	void recreateWholeVertexBuffer(VD::VertexFlags vertexType);

	//std::vector<VD::SharedModelData> modelDatas;

	RecordedBuffer indexBuffer;
	void addToIndexBuffer(OffsetedSharedIndices& sharedOffsetIndices);
	void recreateWholeIndexBuffer();



	VulkanBase* vkBase;
	vkh::structs::VulkanDevice *device;
};

