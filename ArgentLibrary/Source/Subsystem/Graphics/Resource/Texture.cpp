#include "Subsystem/Graphics/Resource/Texture.h"

#include <filesystem>


#include <ResourceUploadBatch.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

#include "Subsystem/Graphics/API/D3D12/GraphicsDevice.h"
#include "Subsystem/Graphics/API/D3D12/CommandQueue.h"
#include "Subsystem/Graphics/API/D3D12/DescriptorHeap.h"
#include "Subsystem/Graphics/Common/GraphicsContext.h"


namespace argent::graphics
{
	void CreateDummyTexture(const dx12::GraphicsDevice* graphics_device, ID3D12Resource** pp_resource);

	Texture::Texture(const GraphicsContext* graphics_context, const char* filename)
	{
		need_to_wait_ = true;
		descriptor_ = graphics_context->cbv_srv_uav_descriptor_heap_->PopDescriptor();
		DirectX::ResourceUploadBatch resource_upload_batch(graphics_context->graphics_device_->GetDevice());

		resource_upload_batch.Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

		HRESULT hr{ S_OK };

		std::filesystem::path path = filename;
		path.replace_extension(".DDS");

		if(std::filesystem::exists(path))
		{
			hr = DirectX::CreateDDSTextureFromFile(graphics_context->graphics_device_->GetDevice(), resource_upload_batch, path.wstring().c_str(),
				resource_object_.ReleaseAndGetAddressOf());}
		else
		{
			path.replace_extension(".PNG");
			hr = DirectX::CreateWICTextureFromFile(graphics_context->graphics_device_->GetDevice(), 
				resource_upload_batch, path.wstring().c_str(),
				resource_object_.ReleaseAndGetAddressOf());
		}

		if(FAILED(hr))
		{
			CreateDummyTexture(graphics_context->graphics_device_, resource_object_.ReleaseAndGetAddressOf());	
		}

		wait_for_finish_upload_ = resource_upload_batch.End(graphics_context->resource_upload_command_queue_->GetCommandQueue());

		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = resource_object_->GetDesc().Format;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = resource_object_->GetDesc().MipLevels;

		graphics_context->graphics_device_->CreateTexture2DSRV(resource_object_.Get(), descriptor_.cpu_handle_);

		//graphics_context->graphics_device_->GetDevice()->CreateShaderResourceView(resource_object_.Get(), &desc,
		//	descriptor_.cpu_handle_);

		auto res_desc = resource_object_->GetDesc();
		width_ = static_cast<UINT>(res_desc.Width);
		height_ = res_desc.Height;
	}

	void Texture::WaitBeforeUse() const
	{
		if(need_to_wait_)
		{
			wait_for_finish_upload_.wait();
		}
	}

	void CreateDummyTexture(const dx12::GraphicsDevice* graphics_device, ID3D12Resource** pp_resource)
	{
		D3D12_HEAP_PROPERTIES heap_prop{};
		heap_prop.Type = D3D12_HEAP_TYPE_CUSTOM;
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heap_prop.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC desc{};
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Width = 1u;
		desc.Height = 1u;
		desc.DepthOrArraySize = 1u;
		desc.SampleDesc.Count = 1u;
		desc.SampleDesc.Quality = 0u;
		desc.MipLevels = 1u;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		graphics_device->CreateResource(heap_prop, desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
			pp_resource);

		std::vector<UINT> data(1);
		for(auto it = data.begin(); it != data.end(); ++it)
		{
			auto col = 0xFFFF7F7F;
			std::fill(it, it + 1, col);
		}

		HRESULT hr = (*pp_resource)->WriteToSubresource(0, nullptr, data.data(), sizeof(UINT), 
			static_cast<UINT>(sizeof(UINT)) * static_cast<UINT>(data.size()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Write To Subresource");
	}
}
