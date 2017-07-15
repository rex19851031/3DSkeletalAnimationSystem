#pragma once

#include <map>
#include <vector>

#include "Engine\Parsing\BufferParser\BufferParser.hpp"

#include "GameCode\Skeleton.hpp"

#ifndef SKELETALANIMATIONOBJECT_HPP
#define SKELETALANIMATIONOBJECT_HPP


namespace Henry
{ 

struct KeyframeData
{
	std::map<int, Skeleton> Keyframes;
};


class SkeletalAnimationObject
{
public:
	SkeletalAnimationObject();
	~SkeletalAnimationObject();
	bool LoadAnim(const std::string& filePath, const std::string& animName);

public:
	std::string m_name;
	std::map<std::string, KeyframeData> m_animNameToKeyframeData;
	Skeleton m_skeleton;

private:
	BufferParser m_bufferParser;
};

}

#endif