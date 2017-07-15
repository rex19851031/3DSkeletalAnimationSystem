#include "Skeleton.hpp"

#include "Engine\Core\HenryFunctions.hpp"


namespace Henry
{ 


Skeleton::Skeleton()
{
	m_numberOfBonesInserted = 0;
}


Skeleton::~Skeleton()
{
	//FIX : should recycle the bone's pointer automatically here?
	//deleteVectorOfPointer(m_allBonesVector);
}


Skeleton::Skeleton(const Skeleton& _skeleton)
{
	for (size_t index = 0; index < _skeleton.Size(); ++index)
	{
		BoneNode* node = _skeleton.GetBoneNodeWithId(index);
		if (node)
		{
			BoneNode* copyNode = new BoneNode(*node);
			m_allBonesVector.push_back(copyNode);
			m_boneNameToIdMap[copyNode->name] = copyNode->id;
		}
		else
		{
			m_allBonesVector.push_back(node);
		}
	}

	HookUpChildParentConnect();
}


bool Skeleton::InsertBoneNode(BoneNode* node)
{
	bool noConflict = true;
	if (ExpandAllBonesVectorToSize(node->id))
	{
		m_allBonesVector[node->id] = node;
		m_boneNameToIdMap[node->name] = node->id;
		++m_numberOfBonesInserted;
	}
	else
	{
		if (m_allBonesVector[node->id] == nullptr)
		{
			m_allBonesVector[node->id] = node;
			m_boneNameToIdMap[node->name] = node->id;
			++m_numberOfBonesInserted;
		}
		else
		{
 			if (m_boneNameToIdMap[node->name] == node->id &&
 				strcmp(m_allBonesVector[node->id]->name.c_str(), node->name.c_str()) == 0)
 			{
 				// update node infomation here
 			}
 			else
 				noConflict = false;
		}
	}
	
	return noConflict;
}


bool Skeleton::ExpandAllBonesVectorToSize(size_t size)
{
	if (size >= m_allBonesVector.size())
	{
		m_allBonesVector.reserve(size + 1);
		while (m_allBonesVector.size() <= size)
		{
			m_allBonesVector.push_back(nullptr);
		}
		return true;
	}
	return false;
}


BoneNode* Skeleton::GetBoneNodeWithId(int id) const
{
	return m_allBonesVector[id];
}


BoneNode* Skeleton::GetBoneNodeWithName(const std::string& boneName)
{
	BoneNode* node = nullptr;
	std::map<std::string, int>::iterator it = m_boneNameToIdMap.find(boneName);
	if (it != m_boneNameToIdMap.end())
		node = m_allBonesVector[it->second];
	return node;
}


int Skeleton::GetNumberOfBones()
{
	return m_numberOfBonesInserted;
}


size_t Skeleton::Size() const
{
	return m_allBonesVector.size();
}


void Skeleton::ReleaseData()
{
	deleteVectorOfPointer(m_allBonesVector);
}


bool Skeleton::HookUpChildParentConnect()
{
	for (size_t index = 0; index < m_allBonesVector.size(); ++index)
	{
		BoneNode* bn = m_allBonesVector[index];
		if (!bn)
			continue;

		int parentId = bn->parentId;
		if (parentId != -1)
		{
			BoneNode* parent = m_allBonesVector[parentId];
			if (!parent)
				return false;

			bn->parent = parent;
			parent->childs.push_back(bn);
		}
		else
		{
			bool alreadyExist = false;
			for (size_t r = 0; r < m_rootBones.size(); ++r)
			{
				if (m_rootBones[r] == bn)
				{
					alreadyExist = true;
					break;
				}
			}

			if (!alreadyExist)
				m_rootBones.push_back(bn);
		}
	}

	return true;
}


}