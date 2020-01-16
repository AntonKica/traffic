#include "VulkanDataManager.h"
#include "VulkanBase.h"

#include <stb/stb_image.h>

void VulkanDataManager::initialize(VulkanBase* vkBase)
{
	this->vkBase = vkBase;
	device = vkBase->getDevice();
}

void VulkanDataManager::cleanUp(const VkAllocationCallbacks* allocator)
{
	for (auto& [vertexFlag, rb] : vertexBuffers)
		rb.buffer.cleanUp(*device, allocator);
	indexBuffer.buffer.cleanUp(*device, allocator);

	for (auto& texture : textures)
		texture->image.cleanUp(*device, allocator);

	for (auto& [vertexFlag, rb] : modelVertexBuffers)
		rb.buffer.cleanUp(*device, allocator);
	modelIndexBuffer.buffer.cleanUp(*device, allocator);
}

VD::ModelData VulkanDataManager::loadModel(const Model& model)
{
	lazyCleanup();

	VD::ModelData modelData;
	for (const auto& mesh : model.meshes)
	{
		VD::MeshData meshData;
		meshData.drawData.vertices = loadVerticesFromMesh(mesh);
		meshData.vertexBuffer = &vertexBuffers[meshData.drawData.vertices->buffer.type].buffer;

		if (mesh.indices.size())
		{
			meshData.drawData.indices = loadIndicesFromMesh(mesh);
			meshData.indexBuffer = &indexBuffer.buffer;
		}

		if (mesh.textures.size())
			meshData.drawData.textures = loadTexturesFromModel(mesh);

		modelData.meshDatas.push_back(meshData);
	}
	
	return modelData;
}

VD::ModelData VulkanDataManager::loadModelFromPath(std::string modelPath)
{
	lazyCleanup();

	auto possibleModel = findExistingModel(modelPath);
	if (possibleModel)
		return possibleModel.value();

	Model newModel(modelPath);
	VD::ModelData modelData;
	for (const auto& mesh : newModel.meshes)
	{
		VD::MeshData meshData;
		meshData.drawData.vertices = loadModelVerticesFromMesh(mesh);
		meshData.vertexBuffer = &modelVertexBuffers[meshData.drawData.vertices->buffer.type].buffer;

		if (mesh.indices.size())
		{
			meshData.drawData.indices = loadModelIndicesFromMesh(mesh);
			meshData.indexBuffer = &modelIndexBuffer.buffer;
		}

		if (mesh.textures.size())
			meshData.drawData.textures = loadTexturesFromModel(mesh);

		modelData.meshDatas.push_back(meshData);
	}

	// add to loaded
	LoadedModel loadedModel;
	loadedModel.data = modelData;
	loadedModel.modelPath = modelPath;

	loadedModels.push_back(loadedModel);

	return modelData;
}

VD::SharedOffsetedByteVertices VulkanDataManager::loadVerticesFromMesh(const Mesh& mesh)
{
	// laod to RAM
	auto flags = deduceMeshVerticesType(mesh);
	// copy to byte
	VD::ByteVertices byteVertices = transformMeshVerticesToByteVertices(mesh, flags);

	VD::OffsetedByteVertices obv;
	obv.buffer = byteVertices;
	obv.byteOffset = 0;

	vertices[flags].push_back(std::make_shared<VD::OffsetedByteVertices>(obv));

	auto& sharedVerts = vertices[flags].back();

	addToVertexBuffer(sharedVerts);
	return sharedVerts;
}

VD::VertexFlags VulkanDataManager::deduceMeshVerticesType(const Mesh& mesh) const
{
	constexpr bool ignoreNormals = true;
	VD::VertexFlags flags = 0;

	if (mesh.vertices.positions.size())	flags |= VD::POSITION;
	if (mesh.vertices.colors.size())	flags |= VD::COLOR;
	if (mesh.vertices.textures.size())	flags |= VD::TEXTURE;

	// i dont want normals for now
	if (!ignoreNormals && mesh.vertices.normals.size())		
		flags |= VD::NORMAL;

	return flags;
}

size_t VulkanDataManager::deduceSizeFromVertexFlags(VD::VertexFlags flags) const
{
	size_t size = 0;
	switch (flags)
	{
	case VD::POSITION:
		size += sizeof(VD::PositionVertex);
	case VD::COLOR:
		size += sizeof(VD::ColorVertex);
		// skip normal
		//case NORMAL:
		//	break;
	case VD::TEXTURE:
		size += sizeof(VD::TextureVertex);
	}
	return size;
}

VD::ByteVertices VulkanDataManager::transformMeshVerticesToByteVertices(const Mesh& mesh, VD::VertexFlags flags) const
{
	// 
	VD::ByteVertices byteVertices;
	auto& [type, data] = byteVertices;

	type = flags;

	//deduce size
	// skiping normal
	size_t size = 0;
	size += sizeof(VD::PositionVertex) * mesh.vertices.positions.size();
	size += sizeof(VD::ColorVertex) * mesh.vertices.colors.size();
	size += sizeof(VD::TextureVertex) * mesh.vertices.textures.size();

	// copy
	{
		data.resize(size);
		std::byte* byte = data.data();

		const auto& vertices = mesh.vertices;
		for (int index = 0; index < mesh.vertices.positions.size(); ++index)
		{
			if ((flags & VD::POSITION) == VD::POSITION)
			{
				std::memcpy(byte, &vertices.positions[index], sizeof(VD::PositionVertex));
				byte += sizeof(VD::PositionVertex);
			}
			if ((flags & VD::COLOR) == VD::COLOR)
			{
				std::memcpy(byte, &vertices.colors[index], sizeof(VD::ColorVertex));
				byte += sizeof(VD::ColorVertex);
			}
			if ((flags & VD::TEXTURE) == VD::TEXTURE)
			{
				std::memcpy(byte, &vertices.textures[index], sizeof(VD::TextureVertex));
				byte += sizeof(VD::TextureVertex);
			}
		}
	}

	return byteVertices;
}


VD::SharedOffsetedIndices VulkanDataManager::loadIndicesFromMesh(const Mesh& mesh)
{
	// no existence checking
	VD::OffsetedIndices oi;
	oi.buffer = mesh.indices;

	indices.push_back(std::make_shared<VD::OffsetedIndices>(oi));

	auto& sharedInds = indices.back();
	addToIndexBuffer(sharedInds);

	return sharedInds;
}

std::map<VD::TextureType, VD::SharedTexture> VulkanDataManager::loadTexturesFromModel(const Mesh& mesh)
{
	std::map<VD::TextureType, VD::SharedTexture> textures;
	for (const auto [type, path] : mesh.textures)
		textures[type] = loadTexture(path);

	return textures;
}

VD::SharedTexture VulkanDataManager::loadTexture(std::string path)
{
	// find existing
	for (const auto& sharedTexture : textures)
	{
		if (sharedTexture->path == path)
			return sharedTexture;
	}

	int width, height, channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
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
	VD::Texture texture;
	texture.path = path;
	texture.image = newImage;

	textures.push_back(std::make_shared<VD::Texture>(texture));

	return textures.back();
}

void VulkanDataManager::addDataToVertexBuffer(
	VD::SharedOffsetedByteVertices& data, 
	RecordedBuffer& vertexBuffer)
{
	auto& [byteVertices, byteOffset] = *data;
	auto& [buffer, lastWrittenByte] = vertexBuffer;

	auto& verticesData = byteVertices.data;

	const size_t copySize = verticesData.size();
	// move whole collection
	auto stagingBuffer = device->getStagingBuffer(copySize, verticesData.data());

	VkBufferCopy bufferCopy;
	bufferCopy.size = copySize;
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = lastWrittenByte;

	device->copyBuffer(stagingBuffer, buffer, &bufferCopy);

	byteOffset = lastWrittenByte;
	lastWrittenByte += copySize;

	device->freeStagingBuffer(stagingBuffer);
}

void VulkanDataManager::recreateWholeVertexBufferFromSource(
	RecordedBuffer& vertexBuffer, 
	const std::vector<VD::SharedOffsetedByteVertices>& source)
{
	auto& [buffer, lastWrittenByte] = vertexBuffer;

	size_t copySize = 0;
	for (const auto& vts : source)
		copySize += vts->buffer.data.size();

	auto stagingBuffer = device->getStagingBuffer(copySize);
	std::byte* data = reinterpret_cast<std::byte*>(stagingBuffer.map());
	for (auto& vts : source)
	{
		auto& vData = vts->buffer.data;

		std::memcpy(data, vData.data(), vData.size());

		vts->byteOffset = data - stagingBuffer.mapped;
		data += vData.size();
	}
	lastWrittenByte = data - stagingBuffer.mapped;
	stagingBuffer.unmap();

	device->copyBuffer(stagingBuffer, buffer, lastWrittenByte);

	device->freeStagingBuffer(stagingBuffer);
}

void VulkanDataManager::addDataToIndexBuffer(
	VD::SharedOffsetedIndices& data,
	RecordedBuffer& indexBuffer)
{
	auto& [idxs, byteOffset] = *data;
	auto& [buffer, lastWrittenByte] = indexBuffer;

	const size_t copySize = sizeof(data->buffer[0]) * data->buffer.size();

	auto stagingBuffer = device->getStagingBuffer(copySize, data->buffer.data());

	VkBufferCopy bufferCopy;
	bufferCopy.size = copySize;
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = lastWrittenByte;

	device->copyBuffer(stagingBuffer, buffer, &bufferCopy);

	data->byteOffset = lastWrittenByte;
	lastWrittenByte += copySize;

	device->freeStagingBuffer(stagingBuffer);
}

void VulkanDataManager::recreateWholeIndexBufferFromSource(
	RecordedBuffer& indexBuffer, 
	const std::vector<VD::SharedOffsetedIndices>& source)
{
	auto& [idxBuffer, lastWrittenByte] = indexBuffer;

	size_t copySize = 0;
	for (const auto& ids : source)
		copySize += ids->buffer.size() * sizeof(ids->buffer[0]);

	auto stagingBuffer = device->getStagingBuffer(copySize);
	std::byte* data = reinterpret_cast<std::byte*>(stagingBuffer.map());
	for (auto& ids : source)
	{
		size_t currentCopySize = ids->buffer.size() * sizeof(ids->buffer[0]);

		std::memcpy(data, ids->buffer.data(), currentCopySize);

		ids->byteOffset = data - stagingBuffer.mapped;
		data += currentCopySize;
	}
	lastWrittenByte = data - stagingBuffer.mapped;
	stagingBuffer.unmap();

	device->copyBuffer(stagingBuffer, idxBuffer, lastWrittenByte);

	device->freeStagingBuffer(stagingBuffer);
}

void VulkanDataManager::addToVertexBuffer(VD::SharedOffsetedByteVertices& offsetByteVertices)
{
	auto& vertexBuffer= vertexBuffers[offsetByteVertices->buffer.type];

	//find and or create
	if (!vertexBuffer.buffer.initialized())
	{
		constexpr const size_t defaultBufferSize = 1'000'000;
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, defaultBufferSize, vertexBuffer.buffer);
	}

	const size_t copySize = sizeof(offsetByteVertices->buffer.data[0]) * offsetByteVertices->buffer.data.size();
	const size_t requiredSize = copySize + indexBuffer.lastWrittenByte;

	if (requiredSize > vertexBuffer.buffer.size)
	{
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requiredSize * 1.25, vertexBuffer.buffer);

		recreateWholeVertexBufferFromSource(vertexBuffer, vertices[offsetByteVertices->buffer.type]);
	}
	else
	{
		addDataToVertexBuffer(offsetByteVertices, vertexBuffer);
	}
}

void VulkanDataManager::addToIndexBuffer(VD::SharedOffsetedIndices& sharedOffsetIndices)
{
	if (!indexBuffer.buffer.initialized())
	{
		constexpr const size_t defaultBufferSize = 1'000'000;
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, defaultBufferSize, indexBuffer.buffer);
	}

	const size_t copySize = sizeof(sharedOffsetIndices->buffer[0]) * sharedOffsetIndices->buffer.size();
	const size_t requiredSize = copySize + indexBuffer.lastWrittenByte;

	if (requiredSize > indexBuffer.buffer.size)
	{
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requiredSize * 1.25, indexBuffer.buffer);

		recreateWholeIndexBufferFromSource(indexBuffer, indices);
	}
	else
	{
		addDataToIndexBuffer(sharedOffsetIndices, indexBuffer);
	}
}

std::optional<VD::ModelData> VulkanDataManager::findExistingModel(const std::string& modelPath) const
{
	std::optional<VD::ModelData> optModel;
	for (const auto& loaded : loadedModels)
	{
		if (modelPath == loaded.modelPath)
		{
			optModel.emplace(loaded.data);
			break;
		}
	}

	return optModel;
}

VD::SharedOffsetedByteVertices VulkanDataManager::loadModelVerticesFromMesh(const Mesh& mesh)
{
	auto flags = deduceMeshVerticesType(mesh);
	// copy to byte
	VD::ByteVertices byteVertices = transformMeshVerticesToByteVertices(mesh, flags);

	VD::OffsetedByteVertices obv;
	obv.buffer = byteVertices;
	obv.byteOffset = 0;

	modelVertices[flags].push_back(std::make_shared<VD::OffsetedByteVertices>(obv));

	auto& sharedVerts = modelVertices[flags].back();

	addToModelVertexBuffer(sharedVerts);
	return sharedVerts;
}

VD::SharedOffsetedIndices VulkanDataManager::loadModelIndicesFromMesh(const Mesh& mesh)
{
	VD::OffsetedIndices oi;
	oi.buffer = mesh.indices;

	modelIndices.push_back(std::make_shared<VD::OffsetedIndices>(oi));

	auto& sharedInds = modelIndices.back();
	addToModelIndexBuffer(sharedInds);

	return sharedInds;
}

void VulkanDataManager::addToModelVertexBuffer(VD::SharedOffsetedByteVertices& offsetByteVertices)
{
	auto& vertexBuffer = modelVertexBuffers[offsetByteVertices->buffer.type];

	//find and or create
	if (!vertexBuffer.buffer.initialized())
	{
		constexpr const size_t defaultBufferSize = 1'000'000;
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, defaultBufferSize, vertexBuffer.buffer);
	}

	const size_t copySize = sizeof(offsetByteVertices->buffer.data[0]) * offsetByteVertices->buffer.data.size();
	const size_t requiredSize = copySize + vertexBuffer.lastWrittenByte;

	if (requiredSize > vertexBuffer.buffer.size)
	{
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requiredSize * 1.25, vertexBuffer.buffer);

		recreateWholeVertexBufferFromSource(vertexBuffer, modelVertices[offsetByteVertices->buffer.type]);
	}
	else
	{
		addDataToVertexBuffer(offsetByteVertices, vertexBuffer);
	}
}

void VulkanDataManager::addToModelIndexBuffer(VD::SharedOffsetedIndices& sharedOffsetIndices)
{
	if (!modelIndexBuffer.buffer.initialized())
	{
		constexpr const size_t defaultBufferSize = 1'000'000;
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, defaultBufferSize, modelIndexBuffer.buffer);
	}

	const size_t copySize = sizeof(sharedOffsetIndices->buffer[0]) * sharedOffsetIndices->buffer.size();
	const size_t requiredSize = copySize + modelIndexBuffer.lastWrittenByte;

	if (requiredSize > modelIndexBuffer.buffer.size)
	{
		device->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requiredSize * 1.25, modelIndexBuffer.buffer);

		recreateWholeIndexBufferFromSource(modelIndexBuffer, modelIndices);
	}
	else
	{
		addDataToIndexBuffer(sharedOffsetIndices, modelIndexBuffer);
	}
}


void VulkanDataManager::lazyCleanup()
{
	// no need for model cleanup
	// nonModel
	{
		for (auto& [vertexFlag, verticesVector] : vertices)
		{
			auto eraseIt = std::remove_if(std::begin(verticesVector), std::end(verticesVector), [](SOBV& sobv)
				{
					return sobv.use_count() == 1;
				});

			if (eraseIt != verticesVector.end())
			{
				verticesVector.erase(eraseIt, verticesVector.end());
				recreateWholeVertexBufferFromSource(vertexBuffers[vertexFlag], vertices[vertexFlag]);
			}
		}

		// indices
		{
			auto eraseIt = std::remove_if(std::begin(indices), std::end(indices), [](SOI& osi)
				{
					return osi.use_count() == 1;
				});
			if (eraseIt != indices.end())
			{
				indices.erase(eraseIt, indices.end());
				recreateWholeIndexBufferFromSource(indexBuffer, indices);
			}
		}
	}
}
