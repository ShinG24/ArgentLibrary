#pragma once

#include <d3d12.h>
#include <wrl.h>

//TODO Make RootSignature wrap class
namespace argent::graphics
{
	class RootSignature
	{
	public:
		RootSignature() = default;
		~RootSignature() = default;

		RootSignature(RootSignature&) = delete;
		RootSignature(RootSignature&&) = delete;
		RootSignature& operator=(RootSignature&) = delete;
		RootSignature& operator=(RootSignature&&) = delete;

		void AddDescriptorTable();

		void Create();

	private:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_object_;
	};
}