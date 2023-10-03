#include "../Inc/ArgentGraphicsLibrary.h"

#include <windows.h>


#include "../Inc/ShaderCompiler.h"

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D12.lib")

namespace argent::graphics
{
	void GraphicsLibrary::Awake(HWND hwnd)
	{
		hwnd_ = hwnd;

#ifdef _DEBUG
		OnDebugLayer();
#endif

		dxgi_factory_.Awake();
		graphics_device_.Awake(dxgi_factory_.GetIDxgiFactory());
		main_rendering_queue_.Awake(graphics_device_.GetLatestDevice(), L"Main Rendering Queue");
		swap_chain_.Awake(hwnd, dxgi_factory_.GetIDxgiFactory(), main_rendering_queue_.GetCommandQueue(), kNumBackBuffers);

		cbv_srv_uav_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::CbvSrvUav, 10000);
		rtv_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::Rtv, 100);
		dsv_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::Dsv, 100);
		smp_heap_.Awake(graphics_device_, DescriptorHeap::HeapType::Smp, 50);

		graphics_command_list_[0].OnAwake(graphics_device_.GetDevice());
		graphics_command_list_[1].OnAwake(graphics_device_.GetDevice());
		graphics_command_list_[2].OnAwake(graphics_device_.GetDevice());

		for(int i = 0; i < kNumBackBuffers; ++i)
		{
			frame_resources_[i].Awake(graphics_device_, swap_chain_, i, rtv_heap_.PopDescriptor(), dsv_heap_.PopDescriptor());
		}

		fence_.Awake(graphics_device_);

		back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();

		RECT rect{};
		GetWindowRect(hwnd_, &rect);
		viewport_ = D3D12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(rect.right - rect.left), 
			static_cast<FLOAT>(rect.bottom - rect.top), 0.0f, 1.0f);
		scissor_rect_ = D3D12_RECT(rect);



		const ShaderCompiler compiler;
		compiler.Compile(L"./Assets/Shader/VertexShader.hlsl", L"vs_6_6", vertex_shader_.ReleaseAndGetAddressOf());
		compiler.Compile(L"./Assets/Shader/PixelShader.hlsl", L"ps_6_6", pixel_shader_.ReleaseAndGetAddressOf());
		Vertex vertices[4]
		{
			Vertex(float2(-0.5f, 0.5f), float2( 0.0f, 0.0f )),
			Vertex(float2(0.5f, 0.5f), float2( 0.0f, 0.0f )),
			Vertex(float2(-0.5f, -0.6f), float2( 0.0f, 0.0f )),
			Vertex(float2(0.5f, -0.6f), float2( 0.0f, 0.0f )),
		};

		D3D12_HEAP_PROPERTIES heap_prop{};
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.CreationNodeMask = 0u;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC res_desc{};
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		res_desc.Alignment = 0u;
		res_desc.DepthOrArraySize = 1;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Height = 1u;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.MipLevels = 1u;
		res_desc.SampleDesc.Count = 1u;
		res_desc.SampleDesc.Quality = 0u;
		res_desc.Width = sizeof(vertices);
		HRESULT hr = graphics_device_.GetDevice()->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, 
			&res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(vertex_buffer_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create a Buffer");

		vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
		vertex_buffer_view_.SizeInBytes = sizeof(vertices);
		vertex_buffer_view_.StrideInBytes = sizeof(Vertex);

		Vertex* v;
		vertex_buffer_->Map(0u, nullptr, reinterpret_cast<void**>(&v));
		v[0] = vertices[0];
		v[1] = vertices[1];
		v[2] = vertices[2];
		v[3] = vertices[3];


		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		desc.NumParameters = 0u;
		desc.pParameters = nullptr;
		desc.NumStaticSamplers = 0u;
		desc.pStaticSamplers = nullptr;

		Microsoft::WRL::ComPtr<ID3DBlob> root_sig_blob;
		Microsoft::WRL::ComPtr<ID3DBlob> error_blob;
		hr = D3D12SerializeRootSignature(&desc	, D3D_ROOT_SIGNATURE_VERSION_1_0, 
			root_sig_blob.ReleaseAndGetAddressOf(), error_blob.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Serialize RootSignature");

		hr = graphics_device_.GetDevice()->CreateRootSignature(0u, root_sig_blob->GetBufferPointer(),
			root_sig_blob->GetBufferSize(), IID_PPV_ARGS(root_signature_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create RootSignature");


		D3D12_INPUT_ELEMENT_DESC input_desc[]
		{
			{"POSITION", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u},
			{"TEXCOORD", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u},
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc{};
		pipeline_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		pipeline_desc.SampleDesc.Count = 1u;
		pipeline_desc.SampleDesc.Quality = 0u;
		pipeline_desc.NodeMask = 1u;
		pipeline_desc.BlendState.AlphaToCoverageEnable = false;
		pipeline_desc.BlendState.IndependentBlendEnable = false;

		pipeline_desc.BlendState.RenderTarget[0].BlendEnable = true;
		pipeline_desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		pipeline_desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		pipeline_desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		pipeline_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		pipeline_desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		pipeline_desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		pipeline_desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

		pipeline_desc.BlendState.RenderTarget[0].LogicOpEnable = false;

		pipeline_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		pipeline_desc.DepthStencilState.DepthEnable = true;

		pipeline_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; 
		pipeline_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		pipeline_desc.DepthStencilState.StencilEnable = false;


		pipeline_desc.InputLayout.NumElements = 2u;
		pipeline_desc.InputLayout.pInputElementDescs = input_desc;
		pipeline_desc.NumRenderTargets = 1u;
		pipeline_desc.PS.pShaderBytecode = pixel_shader_.Get()->GetBufferPointer();
		pipeline_desc.PS.BytecodeLength = pixel_shader_->GetBufferSize();
		pipeline_desc.VS.pShaderBytecode = vertex_shader_->GetBufferPointer();
		pipeline_desc.VS.BytecodeLength = vertex_shader_->GetBufferSize();
		pipeline_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipeline_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pipeline_desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		pipeline_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		pipeline_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		pipeline_desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		pipeline_desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		pipeline_desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		pipeline_desc.RasterizerState.DepthClipEnable = false;
		pipeline_desc.RasterizerState.MultisampleEnable = FALSE;
		pipeline_desc.RasterizerState.AntialiasedLineEnable = FALSE;
		pipeline_desc.RasterizerState.FrontCounterClockwise = true;
		pipeline_desc.RasterizerState.ForcedSampleCount = 0;
		pipeline_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		pipeline_desc.pRootSignature = root_signature_.Get();

		hr = graphics_device_.GetDevice()->CreateGraphicsPipelineState(&pipeline_desc, IID_PPV_ARGS(pipeline_state_.ReleaseAndGetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Create Graphics Pipeline");
	}

	void GraphicsLibrary::Shutdown()
	{
		INT last_back_buffer_index = static_cast<INT>(back_buffer_index_) - 1;
		if(last_back_buffer_index < 0) { last_back_buffer_index = kNumBackBuffers; }
		fence_.WaitForGpu(last_back_buffer_index);
	}

	void GraphicsLibrary::FrameBegin()
	{
		auto& command_list = graphics_command_list_[back_buffer_index_];
		command_list.Activate();

		frame_resources_[back_buffer_index_].Activate(command_list);

		command_list.GetCommandList()->RSSetViewports(1u, &viewport_);
		command_list.GetCommandList()->RSSetScissorRects(1u, &scissor_rect_);

		OnRender();
	}

	void GraphicsLibrary::FrameEnd()
	{
		auto& command_list = graphics_command_list_[back_buffer_index_];

		frame_resources_[back_buffer_index_].Deactivate(command_list);

		command_list.Deactivate();

		ID3D12CommandList* command_lists[]
		{
			command_list.GetCommandList()
		};
		main_rendering_queue_.Execute(1u, command_lists);
		main_rendering_queue_.Signal(fence_);

		swap_chain_.Present();

		back_buffer_index_ = swap_chain_.GetCurrentBackBufferIndex();

		fence_.WaitForGpu(back_buffer_index_);
	}

	void GraphicsLibrary::OnRender()
	{
		auto command_list = graphics_command_list_[back_buffer_index_].GetCommandList();
		command_list->IASetVertexBuffers(0u, 1u, &vertex_buffer_view_);
		command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		command_list->SetGraphicsRootSignature(root_signature_.Get());
		command_list->SetPipelineState(pipeline_state_.Get());

		command_list->DrawInstanced(4u, 1u, 0u, 0u);
	}

	void GraphicsLibrary::OnDebugLayer() const
	{
		HRESULT hr{ S_OK };
		ID3D12Debug* debugLayer = nullptr;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
		_ASSERT_EXPR(SUCCEEDED(hr), L"Failed to Valied D3d12 Debug Layer");

		debugLayer->EnableDebugLayer();
		debugLayer->Release();
	}
}
