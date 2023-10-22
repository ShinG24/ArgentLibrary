#include "../Inc/Texture.h"

#include "../External/DirectXTK12/Inc/ResourceUploadBatch.h"
#include "../External/DirectXTK12/Inc/DDSTextureLoader.h"
#include "../External/DirectXTK12/Inc/WICTextureLoader.h"

#include "../Inc/GraphicsDevice.h"
#include "../Inc/CommandQueue.h"
#include "../Inc/DescriptorHeap.h"

namespace argent::graphics
{
	Texture::Texture(const GraphicsDevice* graphics_device, const CommandQueue* command_queue,
		DescriptorHeap* cbv_srv_uav_heap, const wchar_t* filename, bool is_dds)
	{
		descriptor_ = cbv_srv_uav_heap->PopDescriptor();
		DirectX::ResourceUploadBatch resource_upload_batch(graphics_device->GetDevice());

		resource_upload_batch.Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

		if(is_dds)
		{
			HRESULT hr = DirectX::CreateDDSTextureFromFile(graphics_device->GetDevice(), resource_upload_batch, filename,
				resource_object_.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Load Texture");
		}
		else
		{
			HRESULT hr = DirectX::CreateWICTextureFromFile(graphics_device->GetDevice(), 
				resource_upload_batch, filename,
				resource_object_.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Load Texture");
		}



		auto upload_resources_finished = resource_upload_batch.End(command_queue->GetCommandQueue());
		upload_resources_finished.wait();

		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = resource_object_->GetDesc().Format;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = resource_object_->GetDesc().MipLevels;

		graphics_device->GetDevice()->CreateShaderResourceView(resource_object_.Get(), &desc,
			descriptor_.cpu_handle_);
	}
}
