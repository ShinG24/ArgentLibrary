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

		enum class IdentifierType
		{
			ClosestHit,
			Intersection,
			AnyHit,
		};

		struct LibraryData
		{
			IdentifierType type_;
			std::wstring identifier_;
		};

	public:

		ShaderLibrary(const char* filepath, std::vector<LibraryData>& library_data);

		const std::vector<LibraryData>& GetLibraryData() const { return library_data_;  }

	private:

		Microsoft::WRL::ComPtr<IDxcBlob> library_object_;
		std::vector<LibraryData> library_data_;

		std::string filepath_;
	};
}

