#pragma once

#include <d3d12.h>
#include "../External/DXC/Inc/dxcapi.h"
#include <wrl.h>
namespace argent::graphics
{
	class GraphicsDevice;

	class Raytracer
	{
	public:
		Raytracer() = default;
		~Raytracer() = default;

		Raytracer(Raytracer&) = delete;
		Raytracer(Raytracer&&) = delete;
		Raytracer& operator=(Raytracer&) = delete;
		Raytracer& operator=(Raytracer&&) = delete;

		void Awake(const GraphicsDevice& graphics_device);

	private:
		void CreatePipeline(const GraphicsDevice& graphics_device);

	private:
		//Raytracing Object
		Microsoft::WRL::ComPtr<ID3D12RootSignature> raytracing_global_root_signature_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> raytracing_local_root_signature_;

		//Shader
		Microsoft::WRL::ComPtr<IDxcBlob> ray_gen_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> miss_library_;
		Microsoft::WRL::ComPtr<IDxcBlob> hit_library_;
	};
}