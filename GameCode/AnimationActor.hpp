#pragma once

#include <string>

#include "GameCode\SkeletalAnimationObject.hpp"
#include "GameCode\PointMass.hpp"

#include "Engine\Parsing\C23Parser\C23Parser.hpp"


#ifndef ANIMATIONACTOR_HPP
#define ANIMATIONACTOR_HPP

namespace Henry
{ 

enum INTERPLATION_MODE{ INTERP_NONE = 0, INTERP_LINEAR, INTERP_EASE_IN, INTERP_EASE_OUT, INTERP_EASE_IN_AND_OUT };
enum BLENDING_STATE{ BLENDING_NONE = 0, BLENDING_IN, BLENDING_OUT };
typedef std::pair<int, int> BoneIdDepthPair;

struct AnimationWeight
{
	AnimationWeight(float _weight) : weight(_weight) { }
	~AnimationWeight(){ skeleton.ReleaseData(); }
	std::vector<BoneIdDepthPair> branchFilter;
	Skeleton skeleton;
	float weight;
};

struct AnimationInfo
{
	AnimationInfo() { 
		play = false; 
		loop = true; 
		isFullBodyAnim = true;
		currentFrameIndex = 0;
		numOfKeyframe = 0; 
		currentFrameTimer = 0.0; 
		cuurentPlayTime = 0.0; 
		currentBlendTime = 0.0;
		sampleTime = 0.0; 
		duration = 0.0; 
		currentWeight = 1.0f; 
		weight = 1.0f; 
		curveMode = INTERP_NONE; 
		blendState = BLENDING_NONE;
	};

	std::string name;
	int currentFrameIndex;
	int numOfKeyframe;
	bool loop;
	bool play;
	bool isFullBodyAnim;
	double cuurentPlayTime;
	double currentFrameTimer;
	double currentBlendTime;
	double sampleTime;		//should be loaded from file?
	double duration;
	double blendTime;
	float currentWeight;
	float weight;
	std::vector<BoneIdDepthPair> branchFilter;
	INTERPLATION_MODE curveMode;
	BLENDING_STATE blendState;
};


class AnimationActor
{
public:
	AnimationActor();
	~AnimationActor();
	void Render();
	void Update(double deltaSeconds);
	bool PlayAnimation(const std::string& animName, double duration, bool loop = true, float weight = 1.0f, bool isFullBodyAnim = true);
	bool BlendToAnimation(const std::string& animName, double duration, double blendTime, bool loop = true, float weight = 1.0f, bool isFullBodyAnim = true);
	//bool PlayAnimationAtTime(const std::string animName, double clip);
	//bool PlayAnimationAtFrame(const std::string animName, int frameIndex);
	bool StopAnimation(const std::string& animName, double duration = 0.0);
	bool AttachToSkeletalAnimationObject(SkeletalAnimationObject* sao);
	void SimulateRagdoll();
	bool AddFilter(const std::string& animName, const std::string& boneName, int depth = 0);
	void FillLocalMatrixs();

public:
	std::vector<Matrix4> m_boneToObjectMatrixs;

private:
	bool LinearInterpolateAnimation(const AnimationInfo& currentAnimInfo, Skeleton& _skeleton);
	float InterpolateOrientation(float radiansA, float radiansB, float percentage);
	float CorrectOrientation(float radiansA, float radiansB);
	void UpdateRagdoll(double deltaSeconds);
	void BlendBetweenBones(BoneNode* boneA, BoneNode* boneB, float percentage);

private:
	SkeletalAnimationObject* m_sao;
	Skeleton m_skeleton;
	std::vector<Vertex_PosColor> m_boneVertices;
	C23Parser m_modelParser;
	std::map<std::string, AnimationInfo> m_animNameInfoMaps;
	std::vector<Matrix4> m_boneInWorldMatrixs;
	std::vector<Matrix4> m_boneInitialInverseMatrixs;

	bool m_simulateRagdoll;
	bool m_firstFrame;
	std::vector<PointMass*> m_pointMasses;
};

}

#endif