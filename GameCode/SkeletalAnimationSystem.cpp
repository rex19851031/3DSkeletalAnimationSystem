#include "SkeletalAnimationSystem.hpp"
#include "Engine\Renderer\OpenGLRenderer.hpp"

#include <stack>


namespace Henry
{

SkeletalAnimationSystem::SkeletalAnimationSystem()
{
}


SkeletalAnimationSystem::~SkeletalAnimationSystem()
{
}


bool SkeletalAnimationSystem::LoadAnimationFile(const std::string& filePath, const std::string& skeletalName, const std::string& animName)
{
	SkeletalAnimationObject* sao;

	if (m_skeletalNameToAnimObject.find(skeletalName) == m_skeletalNameToAnimObject.end())
	{
		sao = new SkeletalAnimationObject();
		m_skeletalNameToAnimObject[skeletalName] = sao;
		sao->m_name = skeletalName;
		sao->m_skeleton.m_name = skeletalName;
	}
	else
		sao = m_skeletalNameToAnimObject[skeletalName];

	bool loadSuccess = sao->LoadAnim(filePath, animName);
	return loadSuccess;
}


SkeletalAnimationObject* SkeletalAnimationSystem::GetSkeletalAnimationObject(const std::string& skeletalName)
{
	if (m_skeletalNameToAnimObject.find(skeletalName) == m_skeletalNameToAnimObject.end())
		return nullptr;
	else
		return m_skeletalNameToAnimObject[skeletalName];
}


}