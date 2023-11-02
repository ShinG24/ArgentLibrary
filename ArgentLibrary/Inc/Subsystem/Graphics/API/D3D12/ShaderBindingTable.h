#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <vector>

namespace argent::graphics::dx12
{
	class GraphicsDevice;
}

namespace argent::graphics::dx12
{
	struct ShaderTable
	{
		//Raygen Miss�̏ꍇ�͕��ʂɊ֐����A
		//Hit�̏ꍇ��HitGroupName
		std::wstring shader_identifier_;
		//Input�f�[�^�A���[�g�V�O�l�`���ƕR�Â��Ă���̂�
		//RootParameter��Index�ɍ��킹�ăo�C���h����΂����Ǝv��
		//��{�I��Buffer��GpuAddress��Descriptor��GpuHandle
		std::vector<void*> input_data_;
	};

	/**
	 * \brief Shader Table��Wrap�����N���X
	 * �o�C���h���邽�߂̃o�b�t�@�ƃo�C���h�������ێ�
	 * Shader Identifier�̐������o�C���h����ϐ����K�v�ɂȂ�
	 */
	class ShaderBindingTable
	{
	public:
		ShaderBindingTable() = default;
		~ShaderBindingTable() = default;

		ShaderBindingTable(const ShaderBindingTable&) = delete;
		ShaderBindingTable(const ShaderBindingTable&&) = delete;
		ShaderBindingTable& operator=(const ShaderBindingTable&) = delete;
		ShaderBindingTable& operator=(const ShaderBindingTable&&) = delete;

		/**
		 * \brief �V�F�[�_�[�̃V���{���ƃo�C���h����f�[�^��ǉ�
		 * \param shader_identifier Shader Identifier Raygen or Miss = �֐��� HitGroup = Hit Group Name
		 * \param data Data ���[�g�V�O�l�`�����݂č����Ă邩�m�F����
		 * \return �C���f�b�N�X
		 */
		UINT AddShaderIdentifierAndInputData(const std::wstring& shader_identifier, const std::vector<void*>& data);

		/**
		 * \brief �V�F�[�_�[�V���{���̒ǉ�
		 * \param shader_identifier shader_identifier Shader Identifier Raygen or Miss = �֐��� HitGroup = Hit Group Name
		 * \return �C���f�b�N�X
		 */
		UINT AddShaderIdentifier(const std::wstring& shader_identifier);

		/**
		 * \brief �V�F�[�_�[�e�[�u���̒ǉ�
		 * \param shader_table Shader Table
		 * \return Index
		 */
		UINT AddShaderTable(const ShaderTable& shader_table);

		/**
		 * \brief �����̃V�F�[�_�[�e�[�u���ǉ�
		 * \param shader_tables Shader Table
		 */
		void AddShaderTables(const std::vector<ShaderTable>& shader_tables);

		/**
		 * \brief �^����ꂽ��񂩂�V�F�[�_�[�e�[�u���I�u�W�F�N�g���쐬
		 * \param graphics_device Graphics Device
		 * \param state_object_properties State Object Properties id���擾���邽�߂�
		 * \param resource_object_name Buffer�̖��O
		 */
		void Generate(const GraphicsDevice* graphics_device, ID3D12StateObjectProperties* state_object_properties,
		              LPCWSTR resource_object_name);

		/**
		 * \brief �o�b�t�@�֏����R�s�[����
		 * \param state_object_properties ID3D12StateObjectProperties
		 */
		void CopyToBuffer(ID3D12StateObjectProperties* state_object_properties);

		/**
		 * \brief Resource Object�̃|�C���^
		 * \return D3D12_GPU_VIRTUAL_ADDRESS
		 */
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return resource_object_->GetGPUVirtualAddress(); }

		/**
		 * \brief �o�b�t�@�̃T�C�Y
		 * \return Buffer Size
		 */
		UINT GetSize() const { return static_cast<UINT>(resource_object_->GetDesc().Width); }

		/**
		 * \brief Buffer�̋�؂�T�C�Y == EntrySize
		 * \return Stride Size
		 */
		UINT GetStride() const { return entry_size_; }

	private:

		std::vector<ShaderTable> shader_tables_{};					//���܂Ƃ�
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_object_{};	//���ۂɏ���ێ����Ă���o�b�t�@
																	//�`�悷��Ƃ��ɃZ�b�g����
		UINT entry_size_{};	//ShaderIdentifier��Ƃ���ɕR�Â��������o�C���h����̂ɕK�v�ȃT�C�Y
							//Buffer�̃T�C�Y�ł͂Ȃ��̂Œ���
							//Buffer�̃T�C�Y==EntrySize * ShaderTables.size()

	};
}