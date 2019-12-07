#pragma once
#include "vulkanHelper/VulkanStructs.h"

// for namespaec
#include "VulkanInfo.h"
#include "VulkanData.h"
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


class VulkanBase;
class DescriptorManager
{
private:
	
	void processMeshData(VD::MeshData& meshData);

	Info::DescriptorSetInfo generateSetInfoFromMeshData(const VD::MeshData& meshData) const;
	Info::DescriptorBindings generateDescriptorBindingsFormMeshData(const VD::MeshData& meshData) const;
	Info::DescriptorPoolInfo generatePoolInfo(const Info::DescriptorSetInfo& info) const;
	Info::DescriptorSetLayoutInfo genereateSetLayoutInfo(const Info::DescriptorSetInfo& setInfo);

	VD::pDescriptorPool getDescriptorPool(const Info::DescriptorPoolInfo& info);
	VD::pDescriptorPool createDescriptorPool(const Info::DescriptorPoolInfo& poolInfo);
	std::optional<VD::pDescriptorPool> findDescriptorPool(const Info::DescriptorPoolInfo& poolInfo) const;
	bool comparePoolInfo(const Info::DescriptorPoolInfo& lhs, const Info::DescriptorPoolInfo& rhs) const;

	VD::SharedDescriptorSet getDescriptorSet(const Info::DescriptorSetInfo& setInfo);
	VD::SharedDescriptorSet createDescriptorSet(const Info::DescriptorSetInfo& setInfo);
	std::optional<VD::SharedDescriptorSet> findDescriptorSet(const Info::DescriptorSetInfo& setInfo) const;
	bool compareSetInfo(const Info::DescriptorSetInfo& lhs, const Info::DescriptorSetInfo& rhs) const;
	bool compareBindings(const Info::DescriptorBinding& lhs, const Info::DescriptorBinding& rhs) const;

	VD::SharedDescriptorSetLayout getDescriptorSetLayout(const Info::DescriptorSetLayoutInfo& layoutInfo);
	VD::SharedDescriptorSetLayout createDescriptorSetLayout(const Info::DescriptorSetLayoutInfo& setLayoutInfo);
	std::optional<VD::SharedDescriptorSetLayout> findDescriptorSetLayout(const Info::DescriptorSetLayoutInfo& setInfo) const;
	bool compareSetLayoutInfo(const Info::DescriptorSetLayoutInfo& lhs, const Info::DescriptorSetLayoutInfo& rhs) const;

	void writeDescriptorSet(VD::DescriptorSet& descriptorSet);

	VulkanBase* vkBase;
	vkh::structs::VulkanDevice* device;

	std::vector<VD::UniqueDescriptorPool> descriptorPools;
	std::vector<VD::SharedDescriptorSet> descriptorSets;
	std::vector<VD::SharedDescriptorSetLayout> descriptorSetLayouts;
	//std::vector<

public:
	void init(VulkanBase* vkBase);
	void cleanup(const VkAllocationCallbacks* allocator);

	void processModelData(VD::ModelData& modelData);
	//GO::ID getDescriptorReference(const Info::DescriptorSetCreateInfo& info);
};

