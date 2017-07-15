#pragma once

#ifndef SKELETALANIMATIONSYSTEM_HPP 
#define SKELETALANIMATIONSYSTEM_HPP

#include <vector>

#include "Engine\Core\VertexStruct.hpp"
#include "Engine\Parsing\C23Parser\C23Parser.hpp"

#include "GameCode\SkeletalAnimationObject.hpp"


namespace Henry
{

struct Vertex_Anim
{
	Vec3f position;
	Vec3f normal;
	Vec2f textCoords;
	std::vector<int> boneIndexs;
	std::vector<float> boneWeights;
};


class SkeletalAnimationSystem
{
public:
	SkeletalAnimationSystem();
	~SkeletalAnimationSystem();
	bool LoadAnimationFile(const std::string& filePath, const std::string& skeletalName, const std::string& animName);
	SkeletalAnimationObject* GetSkeletalAnimationObject(const std::string& skeletalName);
	
private:
	bool LinearInterpolateAnimation(const std::string animName, int frameIndex, double percentage);
	
private:
	std::map<std::string, SkeletalAnimationObject*> m_skeletalNameToAnimObject;
};

}

#endif