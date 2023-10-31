#pragma once

#include <memory>

namespace argent::graphics
{
	class Model;
	std::shared_ptr<Model> LoadFbxFromFile(const char* filepath);
}

namespace argent::game_resource
{
	//TODO ���������̃��[�_�[�͏㑤�����C�g���ɑΉ����������
	class Model;

	std::shared_ptr<Model> LoadFbx(const char* filename);	
}