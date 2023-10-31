#include "Subsystem/Graphics/Resource/Material.h"

#include "Subsystem/Graphics/API/D3D12/ConstantBuffer.h"

#include "Subsystem/Graphics/Resource/Texture.h"

namespace argent::graphics
{
	Material::Material(const std::string& name) :
		GameResource(name)
	{}

	void Material::WaitForGpu()
	{
		for(const auto& t : texture_map_)
		{
			t.second->WaitBeforeUse();
		}
	}

	std::shared_ptr<Texture> Material::GetTexture(TextureUsage type)
	{
		return texture_map_.contains(type) ? texture_map_.at(type) : nullptr;
	}

	dx12::Descriptor Material::GetConstantDescriptorHandle(UINT frame_index) const
	{
		 return constant_buffer_->GetDescriptor(frame_index); 
	}

	D3D12_GPU_VIRTUAL_ADDRESS Material::GetConstantGpuVirtualAddress(UINT frame_index) const
	{
		 return constant_buffer_->GetGpuVirtualAddress(frame_index);
	}
}
