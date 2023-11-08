#include "Subsystem/Graphics/Loader/FbxLoader.h"

#include <fstream>
#include <filesystem>
#include <functional>

#include <fbxsdk.h>

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>

#include "Subsystem/Graphics/API/D3D12/DescriptorHeap.h"
#include "Subsystem/Graphics/API/D3D12/VertexBuffer.h"
#include "Subsystem/Graphics/API/D3D12/IndexBuffer.h"
#include "Subsystem/Graphics/API/D3D12/ConstantBuffer.h"

#include "Subsystem/Graphics/Resource/Mesh.h"
#include "Subsystem/Graphics/Resource/Material.h"
#include "Subsystem/Graphics/Resource/StandardMaterial.h"
#include "Subsystem/Graphics/Resource/Model.h"


#include "Subsystem/ResourceManager/ResourceManager.h"


struct Scene
	{
		struct Node
		{
			uint64_t unique_id_{};
			std::string name_;
			UINT attribute_{};
			int64_t parent_index_{ -1 };
		};

		std::vector<Node> nodes_;
		int64_t IndexOf(uint64_t unique_id) const
		{
			int64_t index{ 0 };
			for (const auto& node : nodes_)
			{
				if (node.unique_id_ == unique_id)
					return index;
				++index;
			}
			return -1;
		}
	};

namespace argent::graphics
{
	struct MeshData
	{
		std::string name_;
		std::vector<Position> position_vec_{};
		std::vector<Normal> normal_vec_{};
		std::vector<Tangent> tangent_vec_{};
		std::vector<Binormal> binormal_vec_{};
		std::vector<Texcoord> texcoord_vec_{};
		std::vector<uint32_t> index_vec_{};

		DirectX::XMFLOAT4X4 default_global_transform_{};
		int64_t fbx_material_unique_id_{};

		void Add(const Position& position, const Normal& normal, const Tangent& tangent, const Binormal& binormal, const Texcoord& texcoord, uint32_t index_value, size_t vector_index)
		{
			position_vec_.at(vector_index) = position;
			normal_vec_.at(vector_index) = normal;
			tangent_vec_.at(vector_index) = tangent;
			binormal_vec_.at(vector_index) = binormal;
			texcoord_vec_.at(vector_index) = texcoord;
			index_vec_.at(vector_index) = index_value;
		}

		void VectorResize(size_t size)
		{
			position_vec_.resize(size);
			normal_vec_.resize(size);
			tangent_vec_.resize(size);
			binormal_vec_.resize(size);
			texcoord_vec_.resize(size);
			index_vec_.resize(size);
		}

		Mesh::Data ToResourceMeshData()
		{
			Mesh::Data data;
			data.position_vec_ = std::move(position_vec_);
			data.normal_vec_ = std::move(normal_vec_);
			data.tangent_vec_ = std::move(tangent_vec_);
			data.binormal_vec_ = std::move(binormal_vec_);
			data.texcoord_vec_ = std::move(texcoord_vec_);
			data.index_vec_ = std::move(index_vec_);
			data.default_global_transform_ = std::move(default_global_transform_);
			return data;
		}
	};

	struct MaterialData
	{
		std::string name_;
		int64_t fbx_unique_id_;
		std::unordered_map<Material::TextureUsage, std::string> filepath_map_;
	};

	void FetchMeshes(FbxScene* fbx_scene, const Scene& scene_view, std::vector<MeshData>& mesh_data_vec);
	void FetchMaterials(FbxScene* fbx_scene, std::vector<MaterialData>& material_data);

	std::shared_ptr<Model> LoadFbxFromFile(const char* filepath)
	{
		FbxManager* fbx_manager{ FbxManager::Create() };
		FbxScene* fbx_scene{ FbxScene::Create(fbx_manager, "") };

		FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager, "") };
		bool import_status{ false };
		import_status = fbx_importer->Initialize(filepath);
		_ASSERT_EXPR(import_status, L"Failed to Import Model");

		import_status = fbx_importer->Import(fbx_scene);
		_ASSERT_EXPR(import_status, L"Failed to Import Model");

		FbxGeometryConverter fbx_converter(fbx_manager);

		Scene scene_view;

		scene_view.nodes_.reserve(fbx_scene->GetNodeCount());

		std::function<void(FbxNode*)> Traverse{[&](FbxNode* fbx_node)
		{
			Scene::Node node;
			node.attribute_ = fbx_node->GetNodeAttribute() ?
				fbx_node->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
			node.name_ = fbx_node->GetName();
			node.unique_id_ = fbx_node->GetUniqueID();
			node.parent_index_ = scene_view.IndexOf(fbx_node->GetParent() ? 
				fbx_node->GetParent()->GetUniqueID() : 0);
			scene_view.nodes_.push_back(node);
			for(int child_index = 0; child_index < fbx_node->GetChildCount(); ++child_index)
			{
				Traverse(fbx_node->GetChild(child_index));
			}
		}};
		Traverse(fbx_scene->GetRootNode());

		//メッシュ情報を取得
		std::vector<MeshData> load_mesh_data;
		FetchMeshes(fbx_scene, scene_view, load_mesh_data);

		//マテリアル情報を取得
		std::vector<MaterialData> load_material_data;
		FetchMaterials(fbx_scene, load_material_data);

		//ファイルからのロードはここまで
		fbx_manager->Destroy();

		//ロードしてきたデータをライブラリ用のデータに変換していく

		//Mesh
		std::vector<Mesh::Data> resource_mesh_data(load_mesh_data.size());

		//Load用のMeshデータからリソース用のMeshデータへ変換
		for(size_t i = 0; i < resource_mesh_data.size(); ++i)
		{
			resource_mesh_data.at(i) = load_mesh_data.at(i).ToResourceMeshData();
		}

		std::vector<std::shared_ptr<Mesh>> meshes(resource_mesh_data.size());
		for(size_t i = 0; i < resource_mesh_data.size(); ++i)
		{
			meshes.at(i) = std::make_shared<Mesh>(load_mesh_data.at(i).name_, resource_mesh_data.at(i));
		}

		//Material
		std::vector<std::shared_ptr<Material>> materials(load_material_data.size());
		std::vector<StandardMaterial::Data> resource_material_data(load_material_data.size());
		for(size_t i = 0; i < resource_material_data.size(); ++i)
		{
			resource_material_data.at(i).filepath_map_ = std::move(load_material_data.at(i).filepath_map_); 
		}

		//TODO リソースマネージャからマテリアルを取ってくる
		for(size_t i = 0; i < materials.size(); ++i)
		{
			materials.at(i) = std::make_shared<StandardMaterial>(load_material_data.at(i).name_, resource_material_data.at(i));
		}

		auto model = std::make_shared<Model>(filepath, meshes, materials);
		return model;
	}

	void FetchMeshes(FbxScene* fbx_scene, const Scene& scene_view, std::vector<MeshData>& mesh_data_vec)
	{
		//1PolygonをFbxから引っ張ってくるやつ
		auto fetch_polygon = [&](FbxMesh* fbx_mesh, FbxStringList uv_names, const FbxVector4* control_points,
			int polygon_index, MeshData& dst)
			{
				for (int position_in_polygon = 0; position_in_polygon < 3; ++position_in_polygon)
				{
					const int vertex_index{ polygon_index * 3 + position_in_polygon };

					Position v_position{};
					Normal v_normal{};
					Tangent v_tangent{};
					Binormal v_binormal{};
					Texcoord v_texcoord{};

					const int polygon_vertex{ fbx_mesh->GetPolygonVertex(polygon_index, position_in_polygon) };
					v_position.x = static_cast<float>(control_points[polygon_vertex][0]);
					v_position.y = static_cast<float>(control_points[polygon_vertex][1]);
					v_position.z = static_cast<float>(control_points[polygon_vertex][2]);

					//データがない場合は生成する
					if (fbx_mesh->GetElementNormalCount() <= 0) fbx_mesh->CreateElementNormal();
					if (fbx_mesh->GetElementTangentCount() <= 0)fbx_mesh->GenerateTangentsData(0, false);
					if (fbx_mesh->GetElementBinormalCount() <= 0) fbx_mesh->CreateElementBinormal();

					FbxVector4 normal;
					fbx_mesh->GetPolygonVertexNormal(polygon_index, position_in_polygon, normal);
					v_normal.x = static_cast<float>(normal[0]);
					v_normal.y = static_cast<float>(normal[1]);
					v_normal.z = static_cast<float>(normal[2]);

					const FbxGeometryElementTangent* tangent = fbx_mesh->GetElementTangent(0);
					v_tangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[0]);
					v_tangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[1]);
					v_tangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[2]);

					const FbxGeometryElementBinormal* binormal = fbx_mesh->GetElementBinormal(0u);
					v_binormal.x = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[0]);
					v_binormal.y = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[1]);
					v_binormal.z = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[2]);

					//UVはなくても問題はなので特に特別は処置はしない　
					//ない==テクスチャマッピングはしない
					if (fbx_mesh->GetElementUVCount() > 0)
					{
						FbxVector2 uv;
						bool unmapped_uv;
						fbx_mesh->GetPolygonVertexUV(polygon_index, position_in_polygon, uv_names[0], uv, unmapped_uv);
						v_texcoord.x = static_cast<float>(uv[0]);
						v_texcoord.y = static_cast<float>(uv[1]);
					}
					dst.Add(v_position, v_normal, v_tangent, v_binormal, v_texcoord, vertex_index, vertex_index);
				}
			};

		//1MeshをFbxから引っ張ってくるやつ
		auto fetch_mesh = [&](const Scene::Node& node, MeshData& dst)
			{
				auto* fbx_node{ fbx_scene->FindNodeByName(node.name_.c_str()) };
				auto* fbx_mesh = fbx_node->GetMesh();

				const int polygon_count{ fbx_mesh->GetPolygonCount() };

				dst.name_ = node.name_;
				dst.VectorResize(polygon_count * 3LL);
				if (fbx_node->GetMaterialCount() > 0)
				{
					//TODO Multiple Material
					dst.fbx_material_unique_id_ = static_cast<int64_t>(fbx_node->GetMaterial(0)->GetUniqueID());
				}
				else
				{
					dst.fbx_material_unique_id_ = -1;
				}

				FbxStringList uv_names;
				fbx_mesh->GetUVSetNames(uv_names);
				const FbxVector4* control_points{ fbx_mesh->GetControlPoints() };
				for(int polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
				{
					fetch_polygon(fbx_mesh, uv_names, control_points, polygon_index, dst);
				}
			};

		//実際にロードしてるところ
		for(const auto& node : scene_view.nodes_)
		{
			if (node.attribute_ != FbxNodeAttribute::EType::eMesh) continue;

			auto& mesh{ mesh_data_vec.emplace_back() };
			fetch_mesh(node, mesh);
		}
	}

	void FetchMaterials(FbxScene* fbx_scene, std::vector<MaterialData>& material_data)
	{
		auto fetch_material = [&](const FbxSurfaceMaterial* fbx_surface_material, MaterialData& dst)
			{
				dst.name_ = fbx_surface_material->GetName();

				FbxProperty fbx_property{};

				//Diffuse (Albedo)
				fbx_property = fbx_surface_material->FindProperty(FbxSurfaceMaterial::sDiffuse);
				if (fbx_property.IsValid())
				{
					const auto fbx_texture = fbx_property.GetSrcObject<FbxFileTexture>();
					dst.filepath_map_[Material::TextureUsage::Albedo] = fbx_texture ?
						fbx_texture->GetRelativeFileName() : "";
				}

				//Normal
				fbx_property = fbx_surface_material->FindProperty(FbxSurfaceMaterial::sNormalMap);
				if (fbx_property.IsValid())
				{
					const auto fbx_texture = fbx_property.GetSrcObject<FbxFileTexture>();
					dst.filepath_map_[Material::TextureUsage::Normal] = fbx_texture ?
						fbx_texture->GetRelativeFileName() : "";
				}

				//Emissive
				fbx_property = fbx_surface_material->FindProperty(FbxSurfaceMaterial::sEmissive);
				if (fbx_property.IsValid())
				{
					const auto fbx_texture = fbx_property.GetSrcObject<FbxFileTexture>();
					dst.filepath_map_[Material::TextureUsage::Normal] = fbx_texture ?
						fbx_texture->GetRelativeFileName() : "";
				}

			};

		auto create_dummy_material = [&](MaterialData& dst)
			{
				dst.name_ = "StandardMaterial_Dummy";
				dst.fbx_unique_id_ = -1;
				dst.filepath_map_.clear();
			};


		const auto material_counts = fbx_scene->GetMaterialCount();
		if(material_counts <= 0)
		{
			auto& m = material_data.emplace_back();
			create_dummy_material(m);
		}

		material_data.resize(material_counts);
		for(int i = 0; i < material_counts; ++i)
		{
			const auto fbx_surface_material = fbx_scene->GetMaterial(i);
			auto& data = material_data.at(i);
			data.name_ = fbx_surface_material->GetName();
			data.fbx_unique_id_ = fbx_surface_material->GetUniqueID();

			FbxProperty fbx_property{};

			//Diffuse (Albedo)
			fbx_property = fbx_surface_material->FindProperty(FbxSurfaceMaterial::sDiffuse);
			if(fbx_property.IsValid())
			{
				const auto fbx_texture = fbx_property.GetSrcObject<FbxFileTexture>();
				data.filepath_map_[Material::TextureUsage::Albedo] = fbx_texture ? 
					fbx_texture->GetRelativeFileName() : "";
			}

			//Normal
			fbx_property = fbx_surface_material->FindProperty(FbxSurfaceMaterial::sNormalMap);
			if(fbx_property.IsValid())
			{
				const auto fbx_texture = fbx_property.GetSrcObject<FbxFileTexture>();
				data.filepath_map_[Material::TextureUsage::Normal] = fbx_texture ? 
					fbx_texture->GetRelativeFileName() : "";
			}

			//Emissive
			fbx_property = fbx_surface_material->FindProperty(FbxSurfaceMaterial::sEmissive);
			if(fbx_property.IsValid())
			{
				const auto fbx_texture = fbx_property.GetSrcObject<FbxFileTexture>();
				data.filepath_map_[Material::TextureUsage::Normal] = fbx_texture ?
					fbx_texture->GetRelativeFileName() : "";
			}
		}
	}
}