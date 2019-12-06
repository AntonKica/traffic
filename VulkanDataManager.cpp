#include "VulkanDataManager.h"
#include "VulkanBase.h"

#include <stb/stb_image.h>

void VulkanDataManager::initialize(VulkanBase* vkBase)
{
	this->vkBase = vkBase;
	device = vkBase->getDevice();
}
void VulkanDataManager::loadModel(const Model& model)
{
	for (int i = 0; i < 5; ++i)
	{
		ModelData modelData;
		for (const auto& mesh : model.meshes)
		{
			MeshData meshData;
			meshData.vertices = loadVerticesFromMesh(mesh);

			if (mesh.indices.size())
				meshData.indices = loadIndicesFromMesh(mesh);

			if (mesh.textures.size())
				meshData.textures = loadTexturesFromModel(mesh);

			modelData.meshDatas.push_back(meshData);
		}
	}

}

VulkanDataManager::SharedByteVertices VulkanDataManager::loadVerticesFromMesh(const Mesh& mesh)
{
	// laod to RAM
	auto flags = deduceMeshVerticesType(mesh);
	// copy to byte
	ByteVertices byteVertices = transformMeshVerticesToByteVertices(mesh, flags);
	vertices[flags].push_back(std::make_shared<ByteVertices>(byteVertices));

	auto& sharedVerts = vertices[flags].back();

	addToVertexBuffer(sharedVerts);
	return sharedVerts;
}

VertexFlags VulkanDataManager::deduceMeshVerticesType(const Mesh& mesh) const
{
	constexpr bool ignoreNormals = true;
	VertexFlags flags = 0;

	if (mesh.colorVertices.size())		flags |= COLOR;
	if (mesh.textureVertices.size())	flags |= TEXTURE;

	// i dont want normals for now
	if (!ignoreNormals && mesh.normalVertices.size())		
		flags |= NORMAL;

	return flags;
}

size_t VulkanDataManager::deduceSizeFromVertexFlags(VertexFlags flags) const
{
	size_t size = 0;
	switch (flags)
	{
	case POSITION:
		size += sizeof(Vertex);
	case COLOR:
		size += sizeof(ColorVertex);
	// skip normal
	//case NORMAL:
	//	break;
	case TEXTURE:
		size += sizeof(TextureVertex);
	}
	return size;
}

ByteVertices VulkanDataManager::transformMeshVerticesToByteVertices(const Mesh& mesh, VertexFlags flags) const
{
	// 
	ByteVertices byteVertices;
	auto& [type, data] = byteVertices;

	type = flags;

	//deduce size
	// skiping normal
	size_t size = 0;
	size += sizeof(Vertex) * mesh.vertices.size();
	size += sizeof(ColorVertex) * mesh.colorVertices.size();
	size += sizeof(TextureVertex) * mesh.textureVertices.size();

	// copy
	{
		data.resize(size);
		std::byte* byte = data.data();
		for (int index = 0; index < mesh.vertices.size(); ++index)
		{
			if ((flags & POSITION) == POSITION)
			{
				std::memcpy(byte, &mesh.vertices[index], sizeof(Vertex));
				byte += sizeof(Vertex);
			}
			if ((flags & COLOR) == COLOR)
			{
				std::memcpy(byte, &mesh.colorVertices[index], sizeof(ColorVertex));
				byte += sizeof(ColorVertex);
			}
			if ((flags & TEXTURE) == TEXTURE)
			{
				std::memcpy(byte, &mesh.textureVertices[index], sizeof(TextureVertex));
				byte += sizeof(TextureVertex);
			}
		}
	}

	return byteVertices;
}

VulkanDataManager::SharedIndices VulkanDataManager::loadIndicesFromMesh(const Mesh& mesh)
{
	// laod to RAM

	// no existence checking
	indices.push_back(std::make_shared<Indices>(mesh.indices));
	
	auto& sharedInds = indices.back();
	addToIndexBuffer(sharedInds);

	return sharedInds;
}

std::map<TextureType, VulkanDataManager::SharedTexture> VulkanDataManager::loadTexturesFromModel(const Mesh& mesh)
{
	std::map<TextureType, SharedTexture> textures;
	for (const auto[type, path] : mesh.textures)
		textures[type] = loadTexture(path);

	return textures;
}

VulkanDataManager::SharedTexture VulkanDataManager::loadTexture(std::string path)
{
	// find existing
	for (const auto& sharedTexture : textures)
	{
		if (sharedTexture->path == path)
			return sharedTexture;
	}

	int width, height, channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	if (!data)
		throw std::runtime_error("Failed to load texture " + path);

	vkh::structs::Buffer& stagingBuffer = device->getStagingBuffer(width * height * 4, data);
	stbi_image_free(data);

	vkh::structs::Image newImage;
	device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newImage, false);

	auto cmdBuffer = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	newImage.transitionLayout(*device, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdBuffer);
	device->flushCommandBuffer(cmdBuffer, device->queues.graphics, false);

	device->copyBufferToImage(stagingBuffer, newImage, width, height);

	device->beginCommandBuffer(cmdBuffer);
	newImage.transitionLayout(*device, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdBuffer);
	device->flushCommandBuffer(cmdBuffer, device->queues.graphics);

	device->createImageView(newImage);
	newImage.setupDescriptor(vkBase->getSampler());
	device->freeStagingBuffer(stagingBuffer);

	//std::string file = std::filesystem::path(loadPath).filename().string();
	Texture texture;
	texture.path = path;
	texture.image = newImage;

	textures.push_back(std::make_shared<Texture>(texture));
	
	return textures.back();
}

void VulkanDataManager::addToVertexBuffer(const SharedByteVertices& byteVertices)
{
	auto& [vertexBuffer, lastWrittenByte] = vertexBuffers[byteVertices->type];
	//find and or create
	if (!vertexBuffer.initialized())
	{
		constexpr const size_t defaultBufferSize = 1'000'000;
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, defaultBufferSize, vertexBuffer);
	}

	auto& verticesData = byteVertices->data;

	const size_t requiredSize = lastWrittenByte + verticesData.size();
	const size_t copySize = verticesData.size();
	// move whole collection
	if (requiredSize > vertexBuffer.size)
	{
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requiredSize * 1.25, vertexBuffer);
		recreateWholeVertexBuffer(byteVertices->type);
	}
	else
	{
		auto stagingBuffer = device->getStagingBuffer(copySize, verticesData.data());

		VkBufferCopy bufferCopy;
		bufferCopy.size = copySize;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = lastWrittenByte;

		device->copyBuffer(stagingBuffer, vertexBuffer, &bufferCopy);
		lastWrittenByte += copySize;

		device->freeStagingBuffer(stagingBuffer);
	}
}

void VulkanDataManager::recreateWholeVertexBuffer(VertexFlags vertexType)
{
	auto& currentVertices = vertices[vertexType];
	auto& [vertexBuffer, lastWrittenByte] = vertexBuffers[vertexType];

	size_t copySize = 0;
	for (const auto& vts : currentVertices)
		copySize += vts->data.size();

	auto stagingBuffer = device->getStagingBuffer(copySize);
	std::byte* data = reinterpret_cast<std::byte*>(stagingBuffer.map());
	for (const auto& vts : currentVertices)
	{
		auto& vData = vts->data;

		std::memcpy(data, vData.data(), vData.size());
		data += vData.size();
	}
	lastWrittenByte = data - stagingBuffer.mapped;
	stagingBuffer.unmap();

	device->copyBuffer(stagingBuffer, vertexBuffer);

	device->freeStagingBuffer(stagingBuffer);
}

void VulkanDataManager::addToIndexBuffer(const SharedIndices& indices)
{
	auto& [indexBuf, lastWrittenByte] = indexBuffer;
	//find and or create
	if(!indexBuf.initialized())
	{
		constexpr const size_t defaultBufferSize = 1'000'000;
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, defaultBufferSize, indexBuf);
	}

	const size_t requiredSize = lastWrittenByte + indices->size();
	const size_t copySize = indices->size() * sizeof((*indices)[0]);
	// move whole collection
	if (requiredSize > indexBuf.size)
	{
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requiredSize * 1.25, indexBuf);
		recreateWholeIndexBuffer();
	}
	else
	{
		auto stagingBuffer = device->getStagingBuffer(copySize, indices->data());

		VkBufferCopy bufferCopy;
		bufferCopy.size = copySize;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = lastWrittenByte;

		device->copyBuffer(stagingBuffer, indexBuf, &bufferCopy);
		lastWrittenByte += copySize;

		device->freeStagingBuffer(stagingBuffer);
	}
}

void VulkanDataManager::recreateWholeIndexBuffer()
{
	auto& [idxBuffer, lastWrittenByte] = indexBuffer;

	size_t copySize = 0;
	for (const auto& ids : indices)
		copySize += ids->size() * sizeof((*ids)[0]);

	auto stagingBuffer = device->getStagingBuffer(copySize);
	std::byte* data = reinterpret_cast<std::byte*>(stagingBuffer.map());
	for (const auto& ids : indices)
	{
		size_t currentCopySize = ids->size() * sizeof((*ids)[0]);

		std::memcpy(data, ids->data(), currentCopySize);
		data += currentCopySize;
	}
	lastWrittenByte = data - stagingBuffer.mapped;
	stagingBuffer.unmap();

	device->copyBuffer(stagingBuffer, idxBuffer);

	device->freeStagingBuffer(stagingBuffer);
}

ModelData* VulkanDataManager::uploadModelData(const ModelData& modelData)
{
	return nullptr;
}
