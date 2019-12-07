#include "DescriptorManager.h"
#include "VulkanBase.h"
#include "Utilities.h"
using namespace Utility;

VD::SharedDescriptorSet DescriptorManager::getDescriptorSet(const Info::DescriptorSetInfo& setInfo)
{
	auto possibleSet = findDescriptorSet(setInfo);
	if (possibleSet)
		return possibleSet.value();
	else
		return createDescriptorSet(setInfo);
}

VD::SharedDescriptorSet DescriptorManager::createDescriptorSet(const Info::DescriptorSetInfo& setInfo)
{
	// find pool
	auto poolInfo = generatePoolInfo(setInfo);
	auto descriptorPool = getDescriptorPool(poolInfo);

	auto setLayoutInfo = genereateSetLayoutInfo(setInfo);
	auto setLayout = getDescriptorSetLayout(setLayoutInfo);

	// allocate
	const int descriptorCount = setInfo.dstBuffers.size();
	std::vector<VkDescriptorSetLayout> layouts(descriptorCount, setLayout->layout);

	VkDescriptorSetAllocateInfo allocInfo = vkh::initializers::descriptorSetAllocateInfo(
		*descriptorPool,
		descriptorCount,
		layouts.data()
	);

	std::vector<VkDescriptorSet> descriptors(descriptorCount);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(*device, &allocInfo, descriptors.data()));

	//write

	// and assing
	VD::DescriptorSet set;
	set.info = setInfo;
	set.setLayout = setLayout;
	set.sets = descriptors;

	writeDescriptorSet(set);

	descriptorSets.push_back(std::make_shared<VD::DescriptorSet>(set));

	return descriptorSets.back();
}

std::optional<VD::SharedDescriptorSet> DescriptorManager::findDescriptorSet(const Info::DescriptorSetInfo& setInfo) const
{
	std::optional<VD::SharedDescriptorSet> set;

	for (auto& descriptorSet : descriptorSets)
	{
		if (compareSetInfo(descriptorSet->info, setInfo))
		{
			set = descriptorSet;
			break;
		}
	}

	return set;
}

bool DescriptorManager::compareSetInfo(const Info::DescriptorSetInfo& lhs, const Info::DescriptorSetInfo& rhs) const
{
	if (lhs.dstBuffers[0] == rhs.dstBuffers[0] &&
		rhs.srcImage == lhs.srcImage)
	{
		for (const auto& lhsBinding : lhs.bindings)
		{
			for (const auto& rhsBinding : rhs.bindings)
			{
				if (!compareBindings(lhsBinding, rhsBinding))
					return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool DescriptorManager::compareBindings(const Info::DescriptorBinding& lhs, const Info::DescriptorBinding& rhs) const
{
	const auto& [lhsBinding, lhsStage, lhsType] = lhs;
	{
		bool suitableRhs = false;
		const auto& [rhsBinding, rhsStage, rhsType] = rhs;
		{
			if (lhsType == rhsType && lhsBinding == rhsBinding && lhsStage == rhsStage)
			{
				suitableRhs = true;
			}
		}

		if (!suitableRhs)
			return false;
	}

	return true;
}

VD::SharedDescriptorSetLayout DescriptorManager::getDescriptorSetLayout(const Info::DescriptorSetLayoutInfo& layoutInfo)
{
	auto possibleLayout = findDescriptorSetLayout(layoutInfo);
	if (possibleLayout)
		return possibleLayout.value();
	else
		return createDescriptorSetLayout(layoutInfo);
}

VD::SharedDescriptorSetLayout DescriptorManager::createDescriptorSetLayout(const Info::DescriptorSetLayoutInfo& setLayoutInfo)
{
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	for (const auto& binding : setLayoutInfo.bindings)
	{
		VkDescriptorSetLayoutBinding layoutBinding = vkh::initializers::descritptorSetLayoutBinding(
			binding.type,
			binding.stage,
			binding.binding);

		if (setLayoutInfo.sampler)
			layoutBinding.pImmutableSamplers = setLayoutInfo.sampler.value();

		layoutBindings.push_back(layoutBinding);
	}


	VkDescriptorSetLayoutCreateInfo descriptorSetlayoutInfo =
	{
		vkh::initializers::descriptorSetLayoutCreateInfo(
			layoutBindings.size(),
			layoutBindings.data()
		)
	};

	VkDescriptorSetLayout layout;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*device, &descriptorSetlayoutInfo, nullptr, &layout));


	VD::DescriptorSetLayout setLayout;
	setLayout.info = setLayoutInfo;
	setLayout.layout = layout;

	descriptorSetLayouts.push_back(std::make_shared<VD::DescriptorSetLayout>(setLayout));

	return descriptorSetLayouts.back();
}

std::optional<VD::SharedDescriptorSetLayout> DescriptorManager::findDescriptorSetLayout(const Info::DescriptorSetLayoutInfo& setInfo) const
{
	std::optional<VD::SharedDescriptorSetLayout> setLayout;
	for (const auto& descriptorSetLayot : descriptorSetLayouts)
	{
		if (compareSetLayoutInfo(descriptorSetLayot->info, setInfo))
		{
			setLayout = descriptorSetLayot;
			break;
		}
	}

	return setLayout;
}

bool DescriptorManager::compareSetLayoutInfo(const Info::DescriptorSetLayoutInfo& lhs, const Info::DescriptorSetLayoutInfo& rhs) const
{
	if (compareOptionals(lhs.sampler, rhs.sampler))
	{
		for (const auto& lhsBinding : lhs.bindings)
		{
			for (const auto& rhsBinding : rhs.bindings)
			{
				if (!compareBindings(lhsBinding, rhsBinding))
					return false;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

Info::DescriptorPoolInfo DescriptorManager::generatePoolInfo(const Info::DescriptorSetInfo& info) const
{
	Info::DescriptorPoolInfo poolInfo;
	poolInfo.maxSets = DescriptorSettings::defaultDescriptorPoolMaxSets;

	auto copyVkDescriptorType = [](const Info::DescriptorBinding& info)
	{	return info.type;	};

	std::transform(std::begin(info.bindings), std::end(info.bindings),
		std::back_inserter(poolInfo.supportedTypes), copyVkDescriptorType);

	return poolInfo;
}

Info::DescriptorSetLayoutInfo DescriptorManager::genereateSetLayoutInfo(const Info::DescriptorSetInfo& setInfo)
{
	Info::DescriptorSetLayoutInfo layoutInfo;
	layoutInfo.bindings = setInfo.bindings;
	
	return layoutInfo;
}

void DescriptorManager::processMeshData(VD::MeshData& meshData)
{
	auto descriptorSetInfo = generateSetInfoFromMeshData(meshData);

	meshData.descriptorSet = getDescriptorSet(descriptorSetInfo);
}

static VkDescriptorType bufferUsageTypeToDescriptorType(VkBufferUsageFlags bufferUsage)
{
	switch (bufferUsage)
	{
	case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	default:
		throw std::runtime_error("Unknown buffer usage!");
	}
}

Info::DescriptorSetInfo DescriptorManager::generateSetInfoFromMeshData(const VD::MeshData& meshData) const
{
	auto& uniformBuffers = vkBase->getUniformBuffers();

	Info::DescriptorBindings bindings = generateDescriptorBindingsFormMeshData(meshData);

	Info::DescriptorSetInfo setInfo;
	setInfo.bindings = bindings;
	// copy buffer pointers
	auto transformBuffer = [](const vkh::structs::Buffer& buffer) {	return &buffer;	};
	std::transform(std::begin(uniformBuffers), std::end(uniformBuffers),
		std::back_inserter(setInfo.dstBuffers), transformBuffer);

	if (meshData.drawData.textures.size())
	{
		auto& [type, sharedTexture] = *meshData.drawData.textures.begin();
		setInfo.srcImage = &sharedTexture->image;
		setInfo.sampler = &vkBase->getSampler();
	}

	return setInfo;
}

Info::DescriptorBindings DescriptorManager::generateDescriptorBindingsFormMeshData(const VD::MeshData& meshData) const
{
	auto& uniformBuffers = vkBase->getUniformBuffers();

	Info::DescriptorBindings bindings;
	{
		{
			Info::DescriptorBinding uboBinding;
			uboBinding.binding = 0;
			uboBinding.stage = VK_SHADER_STAGE_VERTEX_BIT;;
			uboBinding.type = bufferUsageTypeToDescriptorType(uniformBuffers[0].usage);

			bindings.push_back(uboBinding);
		}
		// has texture?
		if (meshData.drawData.textures.size())
		{
			Info::DescriptorBinding imageBinding;
			imageBinding.binding = 1;
			imageBinding.stage = VK_SHADER_STAGE_FRAGMENT_BIT;;
			imageBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

			bindings.push_back(imageBinding);
		}
	}

	return bindings;
}


VD::pDescriptorPool DescriptorManager::getDescriptorPool(const Info::DescriptorPoolInfo& info)
{
	auto pool = findDescriptorPool(info);
	if (pool)
		return pool.value();

	return createDescriptorPool(info);
}


VD::pDescriptorPool DescriptorManager::createDescriptorPool(const Info::DescriptorPoolInfo& poolInfo)
{
	int imageCount = vkBase->getSwapchainImageCount();

	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const auto& typeInfo : poolInfo.supportedTypes)
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.descriptorCount = imageCount * poolInfo.maxSets;
		poolSize.type = typeInfo;

		poolSizes.push_back(poolSize);
	}

	VkDescriptorPoolCreateInfo poolCreateInfo = vkh::initializers::descriptorPoolCreateInfo(
		poolSizes.size(),
		poolSizes.data(),
		poolInfo.maxSets
	);
	VD::DescriptorPool pool;
	VK_CHECK_RESULT(vkCreateDescriptorPool(*device, &poolCreateInfo, nullptr, &pool.pool));
	pool.info = poolInfo;

	descriptorPools.push_back(std::make_unique<VD::DescriptorPool>(pool));

	return descriptorPools.back().get();
}

std::optional<VD::pDescriptorPool> DescriptorManager::findDescriptorPool(const Info::DescriptorPoolInfo& poolInfo) const
{
	std::optional<VD::pDescriptorPool> pool;

	for (const auto& descriptorPool : descriptorPools)
	{
		if (comparePoolInfo(descriptorPool->info, poolInfo))
		{
			pool = descriptorPool.get();
			break;
		}
	}

	return pool;
}

bool DescriptorManager::comparePoolInfo(const Info::DescriptorPoolInfo& lhs, const Info::DescriptorPoolInfo& rhs) const
{
	const auto& lhsTypes = lhs.supportedTypes;
	const auto& rhsTypes = rhs.supportedTypes;
	if (lhsTypes.size() == lhsTypes.size())
	{
		if(std::equal(std::begin(lhsTypes), std::end(lhsTypes), std::begin(lhsTypes), std::end(lhsTypes)))
			return true;
	}
	return false;
}

void DescriptorManager::writeDescriptorSet(VD::DescriptorSet& descriptorSet)
{
	const auto& sets = descriptorSet.sets;
	const int setCount = sets.size();

	const auto& setInfo = descriptorSet.info;
	const int bindingCount = setInfo.bindings.size();

	std::vector<VkWriteDescriptorSet> writeSets(bindingCount);

	for (int setIndex = 0; setIndex < setCount; ++setIndex)
	{
		for (int binding = 0; binding < bindingCount; ++binding)
		{
			const auto& curBinding = setInfo.bindings[binding];

			if (curBinding.type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
				curBinding.type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC &&
				curBinding.type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				throw std::runtime_error("Unknown buffer type");
			}

			VkWriteDescriptorSet writeSet = vkh::initializers::writeDescriptorSet();
			writeSet.dstBinding = curBinding.binding;
			writeSet.descriptorCount = 1;
			writeSet.dstSet = sets[setIndex];
			writeSet.descriptorType = curBinding.type;

			if (curBinding.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				writeSet.pImageInfo = &setInfo.srcImage.value()->info;
			else
				writeSet.pBufferInfo = &setInfo.dstBuffers[setIndex]->info;

			writeSets[binding] = writeSet;
		}

		vkUpdateDescriptorSets(*device, writeSets.size(), writeSets.data(), 0, nullptr);
	}
}

void DescriptorManager::init(VulkanBase* vkBase)
{
	this->vkBase = vkBase;
	this->device = vkBase->getDevice();
}

void DescriptorManager::processModelData(VD::ModelData& modelData)
{
	for (auto& meshData : modelData.meshDatas)
	{
		processMeshData(meshData);
	}
}

void DescriptorManager::cleanup(const VkAllocationCallbacks* allocator)
{
	for (auto& descriptorPool : descriptorPools)
		vkDestroyDescriptorPool(*device, descriptorPool->pool, allocator);
	for (auto& descriptorSet : descriptorSetLayouts)
		vkDestroyDescriptorSetLayout(*device, descriptorSet->layout, allocator);
}
