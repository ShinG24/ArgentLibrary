#pragma once

#include <wrl.h>

#include <dxcapi.h>

#include <string>
#include <vector>

namespace argent::graphics
{
	class ShaderLibrary
	{
	public:

		ShaderLibrary(const char* filepath, const std::vector<std::wstring>& library_data);

		const std::vector<std::wstring>& GetExportName() const { return export_name_;  }

	private:

		Microsoft::WRL::ComPtr<IDxcBlob> library_object_;
		std::vector<std::wstring> export_name_;

		std::string filepath_;
	};
}

