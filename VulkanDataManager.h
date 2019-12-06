#pragma once
#include <utility>
#include <memory>
#include <map>

#include "Model.h"
#include "vulkanHelper/VulkanStructs.h"
#include "PipelinesManager.h"
#include "DescriptorManager.h"

namespace
{
	struct Texture
	{
		std::string path;
		vkh::structs::Image image;
	};

	enum VertexType
	{
		POSITION = 0,
		COLOR = 1 << 0,
		NORMAL = 1 << 1,
		TEXTURE = 1 << 2,
	};
	using VertexFlags = uint32_t;
	using Bytes = std::vector<std::byte>;

	struct ByteVertices
	{
		VertexFlags type;
		Bytes data;
	};
	struct MeshData
	{
		std::shared_ptr<ByteVertices> vertices;
		std::shared_ptr<Indices> indices;

		std::map<TextureType, std::shared_ptr<Texture>> textures;

		using ID = uint32_t;
		ID m_descriptorSetReference = {};
		ID m_pipelineReference = {};

		size_t dynamicBufferOffset;
	};
	
	
	struct ModelData
	{
		std::vector<MeshData> meshDatas;
	};
}

class VulkanBase;
class VulkanDataManager
{
public:
	void initialize(VulkanBase* vkBase);
	void loadModel(const Model& model);

private:
	using SharedByteVertices = std::shared_ptr<ByteVertices>;
	SharedByteVertices loadVerticesFromMesh(const Mesh& mesh);
	VertexFlags deduceMeshVerticesType(const Mesh& mesh) const;
	size_t deduceSizeFromVertexFlags(VertexFlags flags) const;
	ByteVertices transformMeshVerticesToByteVertices(const Mesh& mesh, VertexFlags flags) const;

	using SharedIndices = std::shared_ptr<Indices>;
	SharedIndices loadIndicesFromMesh(const Mesh& mesh);

	using SharedTexture = std::shared_ptr<Texture>;
	std::map<TextureType, SharedTexture>  loadTexturesFromModel(const Mesh& mesh);
	SharedTexture loadTexture(std::string path);


	std::map<VertexFlags, std::vector<SharedByteVertices>> vertices;
	std::vector<SharedIndices> indices;
	std::vector<SharedTexture> textures;

	struct RecordedBuffer
	{
		vkh::structs::Buffer buffer{};
		size_t lastWrittenByte = 0;
	};

	std::map<VertexFlags, RecordedBuffer> vertexBuffers;
	void addToVertexBuffer(const SharedByteVertices& byteVertices);
	void recreateWholeVertexBuffer(VertexFlags vertexType);

	RecordedBuffer indexBuffer;
	void addToIndexBuffer(const SharedIndices& indices);
	void recreateWholeIndexBuffer();

	ModelData* uploadModelData(const ModelData& modelData);

	VulkanBase* vkBase;
	vkh::structs::VulkanDevice *device;

	PipelinesManager pipelineManager;
	DescriptorManager descriptorManager;
};

