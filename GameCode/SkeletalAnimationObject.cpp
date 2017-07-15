#include "SkeletalAnimationObject.hpp"

#include <Windows.h>


namespace Henry
{ 

SkeletalAnimationObject::SkeletalAnimationObject()
{
}


SkeletalAnimationObject::~SkeletalAnimationObject()
{
}


bool SkeletalAnimationObject::LoadAnim(const std::string& filePath, const std::string& animName)
{
	if (m_animNameToKeyframeData.find(animName) != m_animNameToKeyframeData.end())
	{
		std::string errorInfo;
		errorInfo = "Animation Name '" + animName + "' Already Exist In This Skeletal Object : ";
		MessageBoxA(NULL, errorInfo.c_str(), "Failed to parsing the animation", MB_ICONERROR | MB_OK);
		return false;
	}

	if (!m_bufferParser.LoadFile(filePath))
		return false;

	char fileExtention[5];
	char subType;
	char versionNum;
	std::string commentText;
	
	memset(fileExtention, 0, 5);
	for (int index = 0; index < 4; index++)
		m_bufferParser.ReadChar(fileExtention[index]);
	
	if (strcmp(fileExtention, "GC23") != 0)
		return false;
	
	m_bufferParser.ReadChar(subType);
	m_bufferParser.ReadChar(versionNum);
	m_bufferParser.ReadString(commentText);
	
	int numOfBones;
	m_bufferParser.ReadInt(numOfBones);
	
	//load skeleton
	for (int index = 0; index < numOfBones; ++index)
	{
		BoneNode* bn = new BoneNode();
	
		m_bufferParser.ReadInt(bn->id);
		m_bufferParser.ReadString(bn->name);
	
		m_bufferParser.ReadFloat(bn->translation.x);
		m_bufferParser.ReadFloat(bn->translation.y);
		m_bufferParser.ReadFloat(bn->translation.z);
	
		m_bufferParser.ReadFloat(bn->orientation.x);
		m_bufferParser.ReadFloat(bn->orientation.y);
		m_bufferParser.ReadFloat(bn->orientation.z);
	
		m_bufferParser.ReadFloat(bn->scale.x);
		m_bufferParser.ReadFloat(bn->scale.y);
		m_bufferParser.ReadFloat(bn->scale.z);
	
		m_bufferParser.ReadUChar(bn->color.r);
		m_bufferParser.ReadUChar(bn->color.g);
		m_bufferParser.ReadUChar(bn->color.b);
	
		m_bufferParser.ReadInt(bn->parentId);
	
		if (!m_skeleton.InsertBoneNode(bn))
			return false;
	}
	
	//load Keyframes
	int numOfKeyframe;
	int numOfBonesInThisFrame;
	m_bufferParser.ReadInt(numOfKeyframe);
	for (int frameIndex = 0; frameIndex < numOfKeyframe; ++frameIndex)
	{
		m_bufferParser.ReadInt(numOfBonesInThisFrame);
		Skeleton skeletonForThisFrame;
		for (int boneIndex = 0; boneIndex < numOfBonesInThisFrame; ++boneIndex)
		{
			BoneNode* bn = new BoneNode();
			m_bufferParser.ReadInt(bn->id);
	
			m_bufferParser.ReadFloat(bn->translation.x);
			m_bufferParser.ReadFloat(bn->translation.y);
			m_bufferParser.ReadFloat(bn->translation.z);
	
			m_bufferParser.ReadFloat(bn->orientation.x);
			m_bufferParser.ReadFloat(bn->orientation.y);
			m_bufferParser.ReadFloat(bn->orientation.z);
	
			m_bufferParser.ReadFloat(bn->scale.x);
			m_bufferParser.ReadFloat(bn->scale.y);
			m_bufferParser.ReadFloat(bn->scale.z);
	
			m_bufferParser.ReadInt(bn->parentId);
	
			if (!skeletonForThisFrame.InsertBoneNode(bn))
				return false;
		}
	
		m_animNameToKeyframeData[animName].Keyframes[frameIndex] = skeletonForThisFrame;
	}
	
	if (!m_skeleton.HookUpChildParentConnect())
		return false;
	
	return true;
}


}