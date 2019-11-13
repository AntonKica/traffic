#pragma once
#include "vulkanHelper/VulkanStructs.h"
#include "GraphicsObjects.h"
#include <utility>

/*
template<class Type> using BindingPair = std::pair<uint32_t, Type>;
template<class Type> using Bindings = std::vector<BindingPair<Type>>;
using DescriptorBindingPair = BindingPair<VkDescriptorType>;
using DescriptorBindings = Bindings<DescriptorBindingPair>;
*/

namespace DescriptorSettings
{
	// types of buffer
	constexpr size_t defaultDescriptorPoolMaxSets = 100;
}

namespace Info
{
	struct DescriptorBinding
	{
		uint32_t binding;
		VkShaderStageFlags stage;
		VkDescriptorType type;
	};
	using DescriptorBindings = std::vector<DescriptorBinding>;


	struct DescriptorSetCreateInfo
	{
		DescriptorBindings bindings;

		std::vector<const vkh::structs::Buffer*> dstBuffers;
		const vkh::structs::Image* srcImage = nullptr;
		const VkSampler* sampler;
	};

	struct DescriptorPoolInfo
	{
		size_t maxSets;
		std::vector<VkDescriptorType> supportedTypes;
	};
	struct DescriptorSetInfo
	{
		Info::DescriptorBindings bindings;

		std::vector<const vkh::structs::Buffer*> dstBuffers; 
		std::optional<const vkh::structs::Image*> srcImage;
		std::optional<const VkSampler*> sampler;
	};
}

struct DescriptorPool
{
	VkDescriptorPool pool;
	Info::DescriptorPoolInfo info;

	operator VkDescriptorPool() { return pool; }
	operator VkDescriptorPool() const { return pool; }
};
struct DescriptorSetReference
{
	GO::ID layout;
	GO::ID sets;
	// id layout?
	Info::DescriptorSetInfo info;
};

class VulkanBase;
class DescriptorManager
{
private:
	VulkanBase* vkBase;
	vkh::structs::VulkanDevice* device;
	std::map<GO::ID, DescriptorPool> pools;
	std::map<GO::ID, VkDescriptorSetLayout> setLayouts;
	std::map<GO::ID, std::vector<VkDescriptorSet>> sets;
	std::map<GO::ID, DescriptorSetReference> setReferences;

	Info::DescriptorSetInfo generateSetInfo(const Info::DescriptorSetCreateInfo& info) const;
	Info::DescriptorPoolInfo generatePoolInfo(const Info::DescriptorSetInfo& info) const;

	GO::ID processDescriptorPoolInfo(const Info::DescriptorPoolInfo& info);
	std::optional<GO::ID> findDescriptorPool(const Info::DescriptorPoolInfo& poolInfo) const;
	bool comparePoolInfo(const Info::DescriptorPoolInfo& lhs, const Info::DescriptorPoolInfo& rhs) const;

	GO::ID createDescriptorSetReference(const Info::DescriptorSetInfo& info);
	std::optional<GO::ID> findDescriptorSet(const Info::DescriptorSetInfo& setInfo) const;
	bool compareSetInfo(const Info::DescriptorSetInfo& lhs, const Info::DescriptorSetInfo& rhs) const;


	GO::ID createDescriptorPool(const Info::DescriptorPoolInfo& poolInfo);
	GO::ID createDescriptorSetLayout(const Info::DescriptorSetInfo& setLayoutInfo);
	std::pair<GO::ID,GO::ID> createDescriptorSet(const Info::DescriptorSetInfo& setInfo);

	DescriptorPool getDescriptorPool(GO::ID id);
	VkDescriptorSetLayout getDescriptorSetLayout(GO::ID id);


	VkDescriptorSetLayout& getDescriptorLayout(GO::ID id);
	const VkDescriptorSetLayout& getDescriptorLayout(GO::ID id) const;
	VkDescriptorSet& getDescriptorSet(GO::ID id, int index);
	void writeDescriptorSets(std::vector<VkDescriptorSet>& sets, const Info::DescriptorSetInfo& setInfo);
public:
	void init(VulkanBase* vkBase);

	GO::ID getDescriptorReference(const Info::DescriptorSetCreateInfo& info);
	VkDescriptorSet& getDescriptorSetFromRef(GO::ID refId, int index);
	VkDescriptorSetLayout& getDescriptorLayoutFromRef(GO::ID refId);
	const VkDescriptorSetLayout& getDescriptorLayoutFromRef(GO::ID refId) const;
};

