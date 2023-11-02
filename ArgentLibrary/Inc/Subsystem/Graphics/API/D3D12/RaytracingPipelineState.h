#pragma once

#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>

#include "../External/DXC/Inc/dxcapi.h"

namespace argent::graphics::dx12
{
	class GraphicsDevice;
}

namespace argent::graphics::dx12
{
	/**
	 * \brief DXR�p�̃p�C�v�����X�e�[�g�N���X
	 * �����̃T�u�I�u�W�F�N�g�̂܂Ƃ܂肩��Ȃ�
	 */
	class RaytracingPipelineState
	{
	public:

		RaytracingPipelineState() = default;
		~RaytracingPipelineState() = default;

		RaytracingPipelineState(const RaytracingPipelineState&) = delete;
		RaytracingPipelineState(const RaytracingPipelineState&&) = delete;
		RaytracingPipelineState& operator=(const RaytracingPipelineState&) = delete;
		RaytracingPipelineState& operator=(const RaytracingPipelineState&&) = delete;

		/**
		 * \brief Shader Library�̒ǉ�
		 * �V�F�[�_�[�I�u�W�F�N�g�ւ̃|�C���^�ƃV���{���t���̊֐�����R�t����
		 * \param p_dxc_blob Compiled Shader Pointer
		 * \param export_symbol Export Symbol
		 */
		void AddLibrary(IDxcBlob* p_dxc_blob, const std::vector<std::wstring>& export_symbol);

		/**
		 * \brief �q�b�g�O���[�v�̒ǉ�
		 * \param hit_group_name Hit Group Name �C�ӂ̖��O�A���������j�[�N�Ȃ���
		 * \param closest_hit Closest Hit Shader Name 
		 * \param any_hit Any Hit Shader Name
		 * \param intersection Intersection Shader Name
		 */
		void AddHitGroup(const std::wstring& hit_group_name, const std::wstring& closest_hit,
		                 const std::wstring& any_hit = L"", const std::wstring& intersection = L"");

		/**
		 * \brief ���[�J�����[�g�V�O�l�`���Ɗe�V���{����R�t����
		 * Ray Generation or Miss Shader�̏ꍇ�͂��̂܂܂̊֐���
		 * Hit�̏ꍇ��HitGroup�̖��O
		 * \param root_signature Local Root Signature
		 * \param symbols Symbol Name
		 */
		void AddRootSignatureAssociation(ID3D12RootSignature* root_signature, 
		                                 const std::vector<std::wstring>& symbols);

		/**
		 * \brief RayPayload�̍ő�T�C�Y
		 * \param size Size
		 */
		void SetMaxPayloadSize(UINT size) { max_payload_size_ = size; }

		/**
		 * \brief Intersection Shader����ق�Hit Shader�֑���\���̂̍ő�T�C�Y
		 * \param size Size
		 */
		void SetMaxAttributeSize(UINT size) { max_attribute_size_ = size; }

		/**
		 * \brief �ő�ċA�� ���ˉ񐔂̐ݒ�i�����ɂ͈Ⴄ���ǁj
		 * \param depth Depth
		 */
		void SetMaxRecursionDepth(UINT depth) { max_recursion_depth_ = depth; }

		/**
		 * \brief �p�C�v���C���̍쐬
		 * Local�͑����_�~�[�œK���Ȃ��p�ӂ��Ă�����
		 * Global�̓V�[���R���X�^���g�Ƃ��w�i�Ƃ��o�C���h������񂪂���̂�
		 * �����ƍ��������o�C���h����̂��]�܂����Ǝv��
		 * \param graphics_device GraphicsDevice
		 * \param global_root_signature Global Root Signature
		 * \param local_root_signature Local Root Signature
		 */
		void Generate(const GraphicsDevice* graphics_device,
		              const ID3D12RootSignature* global_root_signature,
		              const ID3D12RootSignature* local_root_signature);

		/**
		 * \brief ID3D12StateObject���擾
		 * \return ID3D12StateObject
		 */
		ID3D12StateObject* GetStateObject() const { return state_object_.Get(); }

		/**
		 * \brief ID3D12StateObjectProperties���擾
		 * \return ID3D12StateObjectProperties
		 */
		ID3D12StateObjectProperties* GetStateObjectProperties() const { return state_object_properties_.Get(); }

	private:
		/**
		 * \brief Shader Library�L�^�p
		 */
		struct ShaderLibrary
		{
			ShaderLibrary(IDxcBlob* p_dxc_blob, const std::vector<std::wstring>& export_symbol);
			IDxcBlob* p_dxc_blob_{};	
			std::vector<std::wstring> export_symbols_{};
			std::vector<D3D12_EXPORT_DESC> export_desc_{};
			D3D12_DXIL_LIBRARY_DESC library_desc_{};
		};

		/**
		 * \brief Hit�O���[�v�L���p
		 */
		struct HitGroup
		{
			HitGroup(std::wstring hit_group_name, std::wstring closest_hit,
				std::wstring any_hit = L"", std::wstring intersection = L"");

			std::wstring hit_group_name_{};
			std::wstring closest_hit_symbol_{};
			std::wstring any_hit_symbol_{};
			std::wstring intersection_symbol_{};
			D3D12_HIT_GROUP_DESC hit_group_desc_{};
		};

		/**
		 * \brief ���[�g�V�O�l�`���ƃV���{���̊֌W�ێ��p
		 */
		struct RootSignatureAssociation
		{
			RootSignatureAssociation(ID3D12RootSignature* root_signature, 
				const std::vector<std::wstring>& symbols);

			ID3D12RootSignature* root_signature_{};
			std::vector<std::wstring> symbols_{};
			std::vector<LPCWSTR> symbol_pointers_{};
			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association_{};
		};

	private:

		//Object
		Microsoft::WRL::ComPtr<ID3D12StateObject> state_object_{};
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> state_object_properties_{};

		//�f�[�^�ێ��p
		std::vector<ShaderLibrary> shader_libraries_{};
		std::vector<HitGroup> hit_groups_{};
		std::vector<RootSignatureAssociation> root_signature_associations_{};
		UINT max_payload_size_{};
		UINT max_attribute_size_{};
		UINT max_recursion_depth_{};
	};
}