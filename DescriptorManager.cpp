#include "DescriptorManager.h"
#include "VulkanBase.h"
#include "Utilities.h"
using namespace Utility;


GO::ID DescriptorManager::createDescriptorPool(const Info::DescriptorPoolInfo& poolInfo)
{
	int imageCount = vkBase->getSwapchainImageCount();

	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const auto& typeInfo : poolInfo.supportedTypes)
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.descriptorCount = imageCount;
		poolSize.type = typeInfo;

		poolSizes.push_back(poolSize);
	}

	VkDescriptorPoolCreateInfo poolCreateInfo = vkh::initializers::descriptorPoolCreateInfo(
		poolSizes.size(),
		poolSizes.data(),
		poolInfo.maxSets
	);
	DescriptorPool pool;
	VK_CHECK_RESULT(vkCreateDescriptorPool(*device, &poolCreateInfo, nullptr, &pool.pool));

	GO::ID poolId = generateNextContainerID(pools);
	pools[poolId] = pool;

	return poolId;
}

std::pair<GO::ID, GO::ID> DescriptorManager::createDescriptorSet(const Info::DescriptorSetInfo& setInfo)
{
	// find pool
	auto poolInfo = generatePoolInfo(setInfo);
	GO::ID poolID = processDescriptorPoolInfo(poolInfo);
	GO::ID setLayoutID = createDescriptorSetLayout(setInfo);

	// allocate
	const int descriptorCount = setInfo.dstBuffers.size();
	std::vector<VkDescriptorSetLayout> layouts(descriptorCount, getDescriptorSetLayout(setLayoutID));

	VkDescriptorSetAllocateInfo allocInfo = vkh::initializers::descriptorSetAllocateInfo(
			getDescriptorPool(poolID),
			descriptorCount,
			layouts.data()
		);

	std::vector<VkDescriptorSet> descriptors(descriptorCount);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(*device, &allocInfo, descriptors.data()));

	//write
	writeDescriptorSets(descriptors, setInfo);

	// and assing
	GO::ID descriptorSetID = generateNextContainerID(sets);
	sets[descriptorSetID] = descriptors;

	return std::make_pair(setLayoutID, descriptorSetID);
}

GO::ID DescriptorManager::createDescriptorSetLayout(const Info::DescriptorSetInfo& setLayoutInfo)
{
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	for (const auto& binding : setLayoutInfo.bindings)
	{
		VkDescriptorSetLayoutBinding layoutBinding = vkh::initializers::descritptorSetLayoutBinding(
			binding.type,
			binding.stage,
			binding.binding);

		if (setLayoutInfo.srcImage)
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

	GO::ID setID = generateNextContainerID(setLayouts);
	setLayouts[setID] = layout;

	return setID;
}

DescriptorPool DescriptorManager::getDescriptorPool(GO::ID id)
{
	return pools[id];
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

Info::DescriptorSetInfo DescriptorManager::generateSetInfo(const Info::DescriptorSetCreateInfo& info) const
{
	Info::DescriptorSetInfo setInfo;
	setInfo.bindings = info.bindings;
	setInfo.dstBuffers = info.dstBuffers;
	if (info.srcImage)
	{
		setInfo.srcImage = info.srcImage;
		setInfo.sampler = info.sampler;
	}

	return setInfo;
}


GO::ID DescriptorManager::processDescriptorPoolInfo(const Info::DescriptorPoolInfo& info)
{
	auto poolID = findDescriptorPool(info);
	if (poolID)
		return poolID.value();

	return createDescriptorPool(info);
}

std::optional<GO::ID> DescriptorManager::findDescriptorPool(const Info::DescriptorPoolInfo& poolInfo) const
{
	std::optional<GO::ID> poolID;

	for (const auto& [id, descriptorPool] : pools)
	{
		if (comparePoolInfo(descriptorPool.info, poolInfo))
		{
			poolID = id;
			break;
		}
	}

	return poolID;
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

GO::ID DescriptorManager::createDescriptorSetReference(const Info::DescriptorSetInfo& info)
{
	const auto& [layoutID, setsID] = createDescriptorSet(info);

	DescriptorSetReference setRef;
	setRef.info = info;
	setRef.layout = layoutID;
	setRef.sets = setsID;

	auto setRefID = generateNextContainerID(setReferences);
	setReferences[setRefID] = setRef;

	return setRefID;
}

std::optional<GO::ID> DescriptorManager::findDescriptorSet(const Info::DescriptorSetInfo& setInfo) const
{
	std::optional<GO::ID> setID;

	for (const auto& [id, descriptorSet] : setReferences)
	{
		if (compareSetInfo(descriptorSet.info, setInfo))
		{
			setID = id;
			break;
		}
	}

	return setID;
}

bool DescriptorManager::compareSetInfo(const Info::DescriptorSetInfo& lhs, const Info::DescriptorSetInfo& rhs) const
{
	if (lhs.dstBuffers[0] == rhs.dstBuffers[0] &&
		rhs.srcImage == lhs.srcImage)
	{
		for (const auto& [lhsBinding, lhsStage, lhsType] : lhs.bindings)
		{
			bool suitableRhs = false;
			for (const auto& [rhsBinding, rhsStage, rhsType] : rhs.bindings)
			{
				if (lhsType == rhsType && lhsBinding == rhsBinding && lhsStage == rhsStage)
				{
					suitableRhs = true;
				}
			}

			if (!suitableRhs)
				break;
		}

		return true;
	}
	else
	{
		return false;
	}
}

void DescriptorManager::writeDescriptorSets(std::vector<VkDescriptorSet>& sets, const Info::DescriptorSetInfo& setInfo)
{
	const int setCount = sets.size();
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

GO::ID DescriptorManager::getDescriptorReference(const Info::DescriptorSetCreateInfo& info)
{
	// sanity check
	if (info.dstBuffers.empty())
		throw std::runtime_error("Failed to supply dstBuffer for desc. set!\n");

	// find previous
	auto setInfo = generateSetInfo(info);
	std::optional<GO::ID> setRefID = findDescriptorSet(setInfo);
	if (setRefID)
		return setRefID.value();

	// create new
	GO::ID newSetRefID = createDescriptorSetReference(setInfo);

	return newSetRefID;
}

VkDescriptorSet& DescriptorManager::getDescriptorSetFromRef(GO::ID refID, int index)
{
	auto setRefIT = setReferences.find(refID);
	if (setRefIT == std::end(setReferences))
		throw std::runtime_error("Unknown layout id " + std::to_string(refID));

	return getDescriptorSet(setRefIT->second.sets, index);
}

VkDescriptorSetLayout& DescriptorManager::getDescriptorLayoutFromRef(GO::ID refID) 
{
	auto setRefIT = setReferences.find(refID);
	if (setRefIT == std::end(setReferences))
		throw std::runtime_error("Unknown layout id " + std::to_string(refID));

	return getDescriptorLayout(setRefIT->second.layout);
}

const VkDescriptorSetLayout& DescriptorManager::getDescriptorLayoutFromRef(GO::ID refID) const
{
	auto setRefIT = setReferences.find(refID);
	if (setRefIT == std::end(setReferences))
		throw std::runtime_error("Unknown layout id " + std::to_string(refID));

	return getDescriptorLayout(setRefIT->second.layout);
}

VkDescriptorSetLayout& DescriptorManager::getDescriptorLayout(GO::ID id)
{
	auto layoutIt = setLayouts.find(id);
	if (layoutIt == std::end(setLayouts))
		throw std::runtime_error("Unknown layout id " + std::to_string(id));

	return layoutIt->second;
}

const VkDescriptorSetLayout& DescriptorManager::getDescriptorLayout(GO::ID id) const
{
	auto layoutIt = setLayouts.find(id);
	if (layoutIt == std::end(setLayouts))
		throw std::runtime_error("Unknown layout id " + std::to_string(id));

	return layoutIt->second;
}

VkDescriptorSet& DescriptorManager::getDescriptorSet(GO::ID id, int index)
{
	auto setIT = sets.find(id);
	if (setIT == std::end(sets))
		throw std::runtime_error("Unknown layout id " + std::to_string(id));

	return setIT->second[index];
}

VkDescriptorSetLayout DescriptorManager::getDescriptorSetLayout(GO::ID id)
{
	auto setLayoutIT = setLayouts.find(id);
	if (setLayoutIT == std::end(setLayouts))
		throw std::runtime_error("Unknown layout id " + std::to_string(id));

	return setLayoutIT->second;
}

void DescriptorManager::cleanup(const VkAllocationCallbacks* allocator)
{
	for (auto& [id, pool] : pools)
		vkDestroyDescriptorPool(*device, pool, allocator);
	for (auto& [id, setLayout] : setLayouts)
		vkDestroyDescriptorSetLayout(*device, setLayout, allocator);
}
