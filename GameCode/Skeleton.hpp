#pragma once

#include <vector>
#include <map>
#include <string>

#include "Engine\Math\Matrix4.hpp"
#include "Engine\Core\VertexStruct.hpp"

#ifndef SKELETON_HPP 
#define SKELETON_HPP

namespace Henry
{ 

class Skeleton;
struct BoneNode
{
	BoneNode(){};
	BoneNode(const BoneNode& node)
	{
		id = node.id;
		parentId = node.parentId;
		name = node.name;
		translation = node.translation;
		orientation = node.orientation;
		scale = node.scale;
		color = node.color;
	}

	int id;
	int parentId;
	std::string name;
	BoneNode* parent;
	std::vector<BoneNode*> childs;
	//Matrix4 matrix;
	Vec3f translation;
	Vec3f orientation;
	Vec3f scale;
	RGBA color;

	void iterateAllChilds(Matrix4 _matrix, std::vector<Vertex_PosColor>& vertices, std::vector<Matrix4>& matrixList)
	{
		static Vertex_PosColor verts[2];

		verts[0].position = _matrix.GetTranslation();
		verts[0].color = color;
		vertices.push_back(verts[0]);

		//_matrix.ApplyTransformMatrix(matrix.m_matrix);
		_matrix.ApplyTransform(translation.x, translation.y, translation.z);

		_matrix.ApplyRotateRadians(orientation.z, 0.0f, 0.0f, 1.0f);
		_matrix.ApplyRotateRadians(orientation.y, 0.0f, 1.0f, 0.0f);
		_matrix.ApplyRotateRadians(orientation.x, 1.0f, 0.0f, 0.0f);
		
		_matrix.ApplyScale(scale.x, scale.y, scale.z);		

		verts[1].position = _matrix.GetTranslation();
		verts[1].color = color;
		vertices.push_back(verts[1]);
		
		matrixList[id] = _matrix;

		for (size_t index = 0; index < childs.size(); ++index)
		{
			childs[index]->iterateAllChilds(_matrix, vertices, matrixList);
		}
	}
};


class Skeleton
{
public:
	Skeleton();
	~Skeleton();
	Skeleton(const Skeleton& _skeleton);
	bool InsertBoneNode(BoneNode* node);
	BoneNode* GetBoneNodeWithId(int id) const;
	BoneNode* GetBoneNodeWithName(const std::string& boneName);
	int GetNumberOfBones();
	size_t Size() const;
	void ReleaseData();
	bool HookUpChildParentConnect();
	
public:
	std::string m_name;
	std::vector<BoneNode*> m_rootBones;

private:
	bool ExpandAllBonesVectorToSize(size_t size);

private:
	std::vector<BoneNode*> m_allBonesVector;
	std::map<std::string, int> m_boneNameToIdMap;
	int m_numberOfBonesInserted;
};

}

#endif