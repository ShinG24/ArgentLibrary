#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>

//TODO Make RootSignature wrap class
namespace argent::graphics::dx12
{
	class GraphicsDevice;

	/**
	 * \brief ID3D12RootSignature �̃��b�v�N���X
	 * Add~()�֐����g���ăp�����[�^�A�����W��ǉ����ACreate()�Ŏ��ۂ̃I�u�W�F�N�g���쐬
	 */
	class RootSignature
	{
	public:

		RootSignature() = default;
		~RootSignature() = default;

		RootSignature(RootSignature&) = delete;
		RootSignature(RootSignature&&) = delete;
		RootSignature& operator=(RootSignature&) = delete;
		RootSignature& operator=(RootSignature&&) = delete;

	private:

		enum
		{
			BaseShaderRegister = 0,
			NumDescriptors = 1,
			RegisterSpace = 2,
			RangeType = 3,
		};

	public:

		/**
		 * \brief �f�B�X�N���v�^�e�[�u���ǉ��֐�
		 * d3d12descriptorrange ��1�ł��ޏꍇ�Ɏg��
		 * \param base_shader_register �V�F�[�_�[���W�X�^�X�^�[�g
		 * \param range_type range type
		 * \param num_descriptors �f�B�X�N���v�^�̐�
		 * \param register_space ���W�X�^�X�y�[�X
		 * \param offset_in_descriptors_from_table_start ���͂̕K�v�Ȃ� 
		 */
		void AddHeapRangeParameter(UINT base_shader_register, UINT num_descriptors,
			D3D12_DESCRIPTOR_RANGE_TYPE range_type,
			UINT register_space, UINT offset_in_descriptors_from_table_start = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

		/**
		 * \brief ������range������ꍇ�̃f�B�X�N���v�^�e�[�u���ǉ��֐�
		 * \param ranges ranges
		 */
		void AddHeapRangeParameters(std::vector<std::tuple<UINT, //BaseShaderRegister
		                                                   UINT, //NumDescriptors
		                                                   UINT, //RegisterSpace
		                                                   D3D12_DESCRIPTOR_RANGE_TYPE	//RangeType
		>> ranges);

		/**
		 * \brief ������range������ꍇ�̃f�B�X�N���v�^�e�[�u���ǉ��֐�
		 * \param ranges ranges
		 */
		void AddHeapRangeParameters(const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges);

		void AddRootParameter(D3D12_ROOT_PARAMETER_TYPE type, UINT shader_register, 
			UINT register_space, UINT num_root_constants);

		/**
		 * \brief RootSignatureObject�����֐�
		 * \param graphics_device GraphicsDevice�̃|�C���^
		 * \param flag RootSignature Flag
		 */
		void Create(const GraphicsDevice* graphics_device, D3D12_ROOT_SIGNATURE_FLAGS flag);

		/**
		 * \brief RootSignature�I�u�W�F�N�g���擾
		 * \return ID3D12RootSignature
		 */
		ID3D12RootSignature* GetRootSignatureObject() const { return root_signature_object_.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_object_;

		//�f�[�^�ێ��p
		std::vector<D3D12_ROOT_PARAMETER> root_parameters_;
		std::vector<D3D12_STATIC_SAMPLER_DESC> static_samplers_;
		std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> descriptor_ranges_;
		std::vector<UINT> range_locations_;

	};
}