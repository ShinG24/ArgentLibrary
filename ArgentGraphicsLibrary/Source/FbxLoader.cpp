#include "../Inc/FbxLoader.h"

#include <fstream>
#include <filesystem>
#include <functional>

#include <fbxsdk.h>

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>


#include "../Inc/Model.h"

namespace argent::game_resource
{
	//For Fbx
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

	struct FbxMesh
	{
		std::string name_;
		std::vector<Mesh::Vertex> vertices_;
		std::vector<uint32_t> indices_;
	};

	void FetchMesh(FbxScene* fbx_scene, const Scene& scene_view, std::vector<FbxMesh>& meshes);

	std::shared_ptr<Model> LoadFbx(const char* filename)
	{
		std::filesystem::path cereal_filename(filename);
		cereal_filename.replace_extension("cereal");

		if(std::filesystem::exists(cereal_filename.c_str()))
		{
			std::ifstream ifs(cereal_filename.c_str(), std::ios::binary);
			cereal::BinaryInputArchive deserialization(ifs);
			std::shared_ptr<Model> model;
			deserialization(model);

			return model;
		}
		else
		{
			FbxManager* fbx_manager{ FbxManager::Create() };
			FbxScene* fbx_scene{ FbxScene::Create(fbx_manager, "") };

			FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager, "") };
			bool import_status{ false };
			import_status = fbx_importer->Initialize(filename);
			_ASSERT_EXPR(import_status, L"Failed to Import Model");

			import_status = fbx_importer->Import(fbx_scene);
			_ASSERT_EXPR(import_status, L"Failed to Import Model");

			FbxGeometryConverter fbx_converter(fbx_manager);

			Scene scene_view;

			std::function<void(FbxNode*)> Traverse{[&](FbxNode* fbx_node)
			{
				Scene::Node& node{ scene_view.nodes_.emplace_back() };
				node.attribute_ = fbx_node->GetNodeAttribute() ?
					fbx_node->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
				node.name_ = fbx_node->GetName();
				node.unique_id_ = fbx_node->GetUniqueID();
				node.parent_index_ = scene_view.IndexOf(fbx_node->GetParent() ? 
					fbx_node->GetParent()->GetUniqueID() : 0);
				for(int child_index = 0; child_index < fbx_node->GetChildCount(); ++child_index)
				{
					Traverse(fbx_node->GetChild(child_index));
				}
			}};
			Traverse(fbx_scene->GetRootNode());

			std::vector<FbxMesh> meshes;
			FetchMesh(fbx_scene, scene_view, meshes);

			fbx_manager->Destroy();

			auto model = std::make_shared<Model>(filename, meshes.at(0).name_, meshes.at(0).vertices_, meshes.at(0).indices_, "", "");
			std::ofstream ofs(cereal_filename.c_str(), std::ios::binary);
			cereal::BinaryOutputArchive serialization(ofs);
			serialization(model);

			return model;
		}
	}


	void FetchMesh(FbxScene* fbx_scene, const Scene& scene_view, std::vector<FbxMesh>& meshes)
	//Fbxシーンからメッシュのアトリビュートを持っているノードを探し、そこから
	//メッシュデータを取って来る
	{
		for(const auto& node : scene_view.nodes_)
		{
			if (node.attribute_ != FbxNodeAttribute::EType::eMesh) continue;

			//TODO FindNodeByNameは欠陥品なので修正する
			auto* fbx_node{ fbx_scene->FindNodeByName(node.name_.c_str()) };
			auto* fbx_mesh{ fbx_node->GetMesh() };

			auto& mesh{ meshes.emplace_back() };
			mesh.name_ = node.name_;

			const int polygon_count{ fbx_mesh->GetPolygonCount() };
			mesh.vertices_.resize(polygon_count * 3LL);
			mesh.indices_.resize(polygon_count * 3LL);

			FbxStringList uv_names;
			fbx_mesh->GetUVSetNames(uv_names);
			const FbxVector4* control_points{ fbx_mesh->GetControlPoints() };
			for (int polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
			{
				for (int position_in_polygon = 0; position_in_polygon < 3; ++position_in_polygon)
				{
					const int vertex_index{ polygon_index * 3 + position_in_polygon };

					Mesh::Vertex vertex;
					const int polygon_vertex{ fbx_mesh->GetPolygonVertex(polygon_index, position_in_polygon) };
					vertex.position_.x = static_cast<float>(control_points[polygon_vertex][0]);
					vertex.position_.y = static_cast<float>(control_points[polygon_vertex][1]);
					vertex.position_.z = static_cast<float>(control_points[polygon_vertex][2]);

					if (fbx_mesh->GetElementNormalCount() > 0)
					{
						FbxVector4 normal;
						fbx_mesh->GetPolygonVertexNormal(polygon_index, position_in_polygon, normal);
						vertex.normal_.x = static_cast<float>(normal[0]);
						vertex.normal_.y = static_cast<float>(normal[1]);
						vertex.normal_.z = static_cast<float>(normal[2]);
					}

					if (fbx_mesh->GetElementUVCount() > 0)
					{
						FbxVector2 uv;
						bool unmapped_uv;
						fbx_mesh->GetPolygonVertexUV(polygon_index, position_in_polygon, uv_names[0], uv, unmapped_uv);
						vertex.texcoord_.x = static_cast<float>(uv[0]);
						vertex.texcoord_.y = static_cast<float>(uv[1]);
					}

					if(fbx_mesh->GenerateTangentsData(0, false))
					{
						const FbxGeometryElementTangent* tangent = fbx_mesh->GetElementTangent(0);
						vertex.tangent_.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[0]);
						vertex.tangent_.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[1]);
						vertex.tangent_.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[2]);
				//		vertex.tangent_.w = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[3]);
					}

					if(fbx_mesh->GetElementBinormalCount() <= 0)
					{
						fbx_mesh->CreateElementBinormal();
					}

					const FbxGeometryElementBinormal* binormal = fbx_mesh->GetElementBinormal(0u);
					vertex.binormal_.x = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[0]);
					vertex.binormal_.y = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[1]);
					vertex.binormal_.z = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[2]);
					//vertex.binormal_.w = static_cast<float>(binormal->GetDirectArray().GetAt(vertex_index)[3]);
					

					mesh.vertices_.at(vertex_index) = std::move(vertex);
					mesh.indices_.at(vertex_index) = vertex_index;
				}
			}
		}
	}
}