#include "AnimationActor.hpp"

#include "Engine\Renderer\OpenGLRenderer.hpp"
#include "Engine\Core\HenryFunctions.hpp"


namespace Henry
{ 

AnimationActor::AnimationActor()
{
	m_firstFrame = true;
	m_simulateRagdoll = false;
	m_boneInWorldMatrixs.reserve(MAX_BONE_SUPPORT);
	m_boneToObjectMatrixs.reserve(MAX_BONE_SUPPORT);

	for (int index = 0; index < MAX_BONE_SUPPORT; ++index)
	{
		m_boneInWorldMatrixs.push_back(Matrix4());
		m_boneToObjectMatrixs.push_back(Matrix4());
		m_boneInitialInverseMatrixs.push_back(Matrix4());
	}
}


AnimationActor::~AnimationActor()
{
}


void AnimationActor::Render()
{
	OpenGLRenderer::LineWidth(10.0f);
	OpenGLRenderer::DrawVertexWithVertexArray(m_boneVertices, OpenGLRenderer::SHAPE_LINES);
	OpenGLRenderer::PointSize(20.0f);
	OpenGLRenderer::DrawVertexWithVertexArray(m_boneVertices, OpenGLRenderer::SHAPE_POINTS);
}


void AnimationActor::Update(double deltaSeconds)
{
	if (!m_sao)
		return;

	if (m_simulateRagdoll)
	{
		m_boneVertices.clear();
		for (size_t index = 0; index < m_pointMasses.size(); index += 2)
		{
			Vertex_PosColor v0;
			v0.position = m_pointMasses[index]->currentPosition;
			v0.color = m_pointMasses[index]->color;

			Vertex_PosColor v1;
			v1.position = m_pointMasses[index + 1]->currentPosition;
			v1.color = m_pointMasses[index + 1]->color;

			m_boneVertices.push_back(v0);
			m_boneVertices.push_back(v1);
		}

		UpdateRagdoll(deltaSeconds);
		return;
	}

	//create a copy of skeleton for interpolate between orientation
	std::vector<AnimationWeight*> fullBodyAnims;
	std::vector<AnimationWeight*> partialBodyAnims;

	std::map<std::string, AnimationInfo>::iterator it = m_animNameInfoMaps.begin();
	while (it != m_animNameInfoMaps.end())
	{
		AnimationInfo& currentAnimInfo = it->second;
		//Should check for currentWeight = 0? if yes erase from the list??
		if (currentAnimInfo.blendState == BLENDING_OUT && currentAnimInfo.currentWeight <= 0.0)
		{
			currentAnimInfo.play = false;
		}
		
		if (!currentAnimInfo.play)
		{
			++it;
			continue;
		}

		currentAnimInfo.cuurentPlayTime += deltaSeconds;
		currentAnimInfo.currentFrameTimer += deltaSeconds;
		if (currentAnimInfo.currentFrameTimer > currentAnimInfo.sampleTime)
		{
			currentAnimInfo.currentFrameTimer = 0.0;
			++currentAnimInfo.currentFrameIndex;

			if (currentAnimInfo.currentFrameIndex >= currentAnimInfo.numOfKeyframe)
			{
				currentAnimInfo.currentFrameIndex = 0;

				if (!currentAnimInfo.loop)
				{
					currentAnimInfo.play = false;
					currentAnimInfo.cuurentPlayTime = 0.0;
				}
			}
		}

		//update blending parameters
		if (currentAnimInfo.blendState != BLENDING_NONE)
		{
			currentAnimInfo.currentBlendTime += deltaSeconds;
			if (currentAnimInfo.blendState == BLENDING_IN)
				currentAnimInfo.currentWeight = currentAnimInfo.weight * (float)(currentAnimInfo.currentBlendTime / currentAnimInfo.blendTime);
			if (currentAnimInfo.blendState == BLENDING_OUT)
				currentAnimInfo.currentWeight = currentAnimInfo.weight * (float)((currentAnimInfo.blendTime - currentAnimInfo.currentBlendTime) / currentAnimInfo.blendTime);

			if (currentAnimInfo.currentWeight > 1.0)
			{
				currentAnimInfo.currentWeight = 1.0;
				if (currentAnimInfo.blendState == BLENDING_IN)
					currentAnimInfo.blendState = BLENDING_NONE;
			}
			if (currentAnimInfo.currentWeight < 0.0)
			{
				currentAnimInfo.currentWeight = 0.0;
				if (currentAnimInfo.blendState == BLENDING_OUT)
					currentAnimInfo.blendState = BLENDING_NONE;
			}
		}

 		//update T, R, S with name
 		std::map<std::string, KeyframeData>::iterator a_it = m_sao->m_animNameToKeyframeData.find(currentAnimInfo.name);
 		if (a_it == m_sao->m_animNameToKeyframeData.end())
 			return;

 		//Interpolate between this frame and next frame
		AnimationWeight* aw = new AnimationWeight(currentAnimInfo.currentWeight);
		LinearInterpolateAnimation(currentAnimInfo, aw->skeleton);
		aw->branchFilter = currentAnimInfo.branchFilter;

		if (currentAnimInfo.isFullBodyAnim)
  			fullBodyAnims.push_back(aw);		//Push_back of non pointer object is actually create a pointer?
  		else
  			partialBodyAnims.push_back(aw);

		++it;
 	}

	//blending between fullBodyAnims
	for (size_t animIndex = 0; animIndex < fullBodyAnims.size(); ++animIndex)
	{
		for (size_t index = 0; index < m_skeleton.Size(); ++index)
		{
			BoneNode* boneA = m_skeleton.GetBoneNodeWithId(index);
			BoneNode* boneB = fullBodyAnims[animIndex]->skeleton.GetBoneNodeWithId(index);
			if (boneA && boneB)
			{
				BlendBetweenBones(boneA, boneB, fullBodyAnims[animIndex]->weight);
			}
		}
		delete fullBodyAnims[animIndex];
	}

	//blending between partialBodyAnims
	for (size_t animIndex = 0; animIndex < partialBodyAnims.size(); ++animIndex)
	{
		if (partialBodyAnims[animIndex]->branchFilter.empty())
		{
			for (size_t index = 0; index < m_skeleton.Size(); ++index)
			{
				BoneNode* boneA = m_skeleton.GetBoneNodeWithId(index);
				BoneNode* boneB = partialBodyAnims[animIndex]->skeleton.GetBoneNodeWithId(index);
				if (boneA && boneB)
				{
					BlendBetweenBones(boneA, boneB, partialBodyAnims[animIndex]->weight);
				}
			}
		}
		else
		{
			for (size_t index = 0; index < partialBodyAnims[animIndex]->branchFilter.size(); ++index)
			{
				int filterRootBoneId = partialBodyAnims[animIndex]->branchFilter[index].first;
				int filterDepth = partialBodyAnims[animIndex]->branchFilter[index].second;
				BoneNode* filterRoot = m_skeleton.GetBoneNodeWithId(filterRootBoneId);
				if (filterRoot)
				{
					std::vector<BoneNode*> blendingList;
					blendingList.push_back(filterRoot);
					int depth = 0;
					while (!blendingList.empty() && (filterDepth == 0 || depth < filterDepth) )
					{
						BoneNode* boneA = blendingList.back();
						BoneNode* boneB = partialBodyAnims[animIndex]->skeleton.GetBoneNodeWithId(boneA->id);
						BlendBetweenBones(boneA, boneB, partialBodyAnims[animIndex]->weight);
						blendingList.pop_back();
						for (size_t c = 0; c < boneA->childs.size(); ++c)
							blendingList.push_back(boneA->childs[c]);
						++depth;
					}
				}
			}
		}

		delete partialBodyAnims[animIndex];
	}
	
	// construct vertices data for rendering
	if (m_sao)
	{
		m_boneVertices.clear();
		for (size_t index = 0; index < m_skeleton.m_rootBones.size(); ++index)
		{
			int rootId = m_skeleton.m_rootBones[index]->id;
			BoneNode* rootInCurrentActor = m_skeleton.GetBoneNodeWithId(rootId);
			if (rootInCurrentActor)
			{
				Matrix4 matrix;
				matrix.LoadIdentity();
				rootInCurrentActor->iterateAllChilds(matrix, m_boneVertices, m_boneInWorldMatrixs);
			}
		}

		if (m_firstFrame)
		{
			m_firstFrame = false;
			for (size_t index = 0; index < m_skeleton.Size(); ++index)
			{
				m_boneInitialInverseMatrixs[index] = m_boneInWorldMatrixs[index];
				m_boneInitialInverseMatrixs[index].Inverse();
			}
		}

		for (size_t index = 0; index < m_skeleton.Size(); ++index)
		{
			m_boneToObjectMatrixs[index] = m_boneInWorldMatrixs[index];
			m_boneToObjectMatrixs[index].ApplyTransformMatrix(m_boneInitialInverseMatrixs[index].m_matrix);
		}
	}
}


bool AnimationActor::LinearInterpolateAnimation(const AnimationInfo& currentAnimInfo, Skeleton& _skeleton)
{
	float percentage = (float)(currentAnimInfo.currentFrameTimer / currentAnimInfo.sampleTime);
	int indexNextFrame = currentAnimInfo.currentFrameIndex + 1 >= currentAnimInfo.numOfKeyframe ? 0 : currentAnimInfo.currentFrameIndex + 1;
	int indexCurrentFrame = currentAnimInfo.currentFrameIndex;
	for (size_t index = 0; index < m_sao->m_animNameToKeyframeData[currentAnimInfo.name].Keyframes[indexNextFrame].Size(); ++index)
	{
		BoneNode* nodeNextFrame = m_sao->m_animNameToKeyframeData[currentAnimInfo.name].Keyframes[indexNextFrame].GetBoneNodeWithId(index);
		if (nodeNextFrame)
		{
			BoneNode* nodeThisFrame = m_sao->m_animNameToKeyframeData[currentAnimInfo.name].Keyframes[indexCurrentFrame].GetBoneNodeWithId(index);
			if (nodeThisFrame)
			{
				BoneNode* node = new BoneNode();
				node->id = nodeThisFrame->id;
				node->parentId = nodeThisFrame->id;
				node->translation = nodeThisFrame->translation + (nodeNextFrame->translation - nodeThisFrame->translation) * percentage;
				node->orientation.x = InterpolateOrientation(nodeThisFrame->orientation.x, nodeNextFrame->orientation.x, percentage);
				node->orientation.y = InterpolateOrientation(nodeThisFrame->orientation.y, nodeNextFrame->orientation.y, percentage);
				node->orientation.z = InterpolateOrientation(nodeThisFrame->orientation.z, nodeNextFrame->orientation.z, percentage);
				node->scale = nodeThisFrame->scale + (nodeNextFrame->scale - nodeThisFrame->scale) * percentage;
				if (!_skeleton.InsertBoneNode(node))
					return false;
			}
		}
	}

	return true;
}


float AnimationActor::InterpolateOrientation(float radiansA, float radiansB, float percentage)
{
	float result; 
	float PI = 3.1415926f;

	float radiansToChange = radiansB - radiansA;
	if (radiansToChange > PI)
		radiansToChange -= 2 * PI;
	else if (radiansToChange < -PI)
		radiansToChange += 2 * PI;

	result = radiansA + radiansToChange * percentage;
	return result;
}


float AnimationActor::CorrectOrientation(float radiansA, float radiansB)
{
	float PI = 3.1415926f;
	float radiansToChange = radiansB - radiansA;
	if (radiansToChange > PI)
		radiansToChange -= 2 * PI;
	else if (radiansToChange < -PI)
 		radiansToChange += 2 * PI;

	return radiansA + radiansToChange;
}


bool AnimationActor::PlayAnimation(const std::string& animName, double duration, bool loop /* = true */, float weight /* = 1.0f */, bool isFullBodyAnim /* = true */)
{
	if (!m_sao)
		return false;

	std::map<std::string, KeyframeData>::iterator k_it = m_sao->m_animNameToKeyframeData.find(animName);
	if (k_it == m_sao->m_animNameToKeyframeData.end())
		return false;

	std::map<std::string, AnimationInfo>::iterator a_it = m_animNameInfoMaps.find(animName);
	if (a_it != m_animNameInfoMaps.end())
	{
		AnimationInfo& anim = a_it->second;
		anim.play = true;
		anim.loop = loop;
		anim.currentWeight = weight;
		anim.weight = weight;
		anim.isFullBodyAnim = isFullBodyAnim;
		anim.blendState = BLENDING_NONE;
		if (duration != anim.duration)
		{
			anim.sampleTime = duration / anim.numOfKeyframe;
			anim.duration = duration;
		}
		return true;
	}

	KeyframeData& animKeyframeData = k_it->second;
	AnimationInfo animInfo;
	animInfo.name = animName;
	animInfo.currentFrameIndex = 0;
	animInfo.cuurentPlayTime = 0.0;
	animInfo.currentFrameTimer = 0.0;
	animInfo.loop = loop;
	animInfo.play = true;
	animInfo.isFullBodyAnim = isFullBodyAnim;
	animInfo.numOfKeyframe = animKeyframeData.Keyframes.size();
	animInfo.sampleTime = duration / animInfo.numOfKeyframe;
	animInfo.duration = duration;
	animInfo.currentWeight = weight;
	animInfo.weight = weight;
	animInfo.blendState = BLENDING_NONE;

	m_animNameInfoMaps[animName] = animInfo;
	return true;
}


bool AnimationActor::BlendToAnimation(const std::string& animName, double duration, double blendTime, bool loop /* = true */, float weight /* = 1.0f */, bool isFullBodyAnim /* = true */)
{
	if (!m_sao)
		return false;

	std::map<std::string, KeyframeData>::iterator k_it = m_sao->m_animNameToKeyframeData.find(animName);
	if (k_it == m_sao->m_animNameToKeyframeData.end())
		return false;

 	std::map<std::string, AnimationInfo>::iterator a_it = m_animNameInfoMaps.begin();
	bool alreadyExist = false;
	while (a_it != m_animNameInfoMaps.end())
	{
		AnimationInfo& anim = a_it->second;

		if (anim.name == animName)
		{
			alreadyExist = true;
			anim.play = true;
			anim.loop = loop;
			anim.weight = weight;
			anim.isFullBodyAnim = isFullBodyAnim;
			anim.blendState = BLENDING_IN;
			anim.currentBlendTime = 0.0;
			anim.blendTime = blendTime;
			if (anim.duration != duration)
			{
				anim.sampleTime = duration / anim.numOfKeyframe;
				anim.duration = duration;
			}
		}
		else
		{
			if (anim.isFullBodyAnim == isFullBodyAnim)
			{
				anim.blendTime = blendTime;
				anim.blendState = BLENDING_OUT;
			}
		}

		++a_it;
	}

	if (alreadyExist)
		return true;

	KeyframeData& animKeyframeData = k_it->second;
	AnimationInfo animInfo;
	animInfo.name = animName;
	animInfo.currentFrameIndex = 0;
	animInfo.cuurentPlayTime = 0.0;
	animInfo.currentBlendTime = 0.0;
	animInfo.currentFrameTimer = 0.0;
	animInfo.loop = loop;
	animInfo.play = true;
	animInfo.isFullBodyAnim = isFullBodyAnim;
	animInfo.numOfKeyframe = animKeyframeData.Keyframes.size();
	animInfo.sampleTime = duration / animInfo.numOfKeyframe;
	animInfo.duration = duration;
	animInfo.currentWeight = 0.0;
	animInfo.weight = weight;
	animInfo.blendState = BLENDING_IN;
	animInfo.blendTime = blendTime;

	m_animNameInfoMaps[animName] = animInfo;
	return true;
}


bool AnimationActor::StopAnimation(const std::string& animName, double duration)
{
	std::map<std::string, AnimationInfo>::iterator it = m_animNameInfoMaps.find(animName);
	if (it == m_animNameInfoMaps.end())
		return false;
	if (duration <= 0.0)
		it->second.play = false;
	else
	{
		it->second.blendState = BLENDING_OUT;
		it->second.blendTime = duration;
		it->second.currentBlendTime = 0;
	}

	return true;
}


bool AnimationActor::AttachToSkeletalAnimationObject(SkeletalAnimationObject* sao)
{
	if (!sao)
		return false;

	m_sao = sao;
	m_skeleton = Skeleton(sao->m_skeleton);
	m_boneVertices.reserve(m_skeleton.Size() * 2 + 10);

	return true;
}


void AnimationActor::SimulateRagdoll()
{
	m_simulateRagdoll = true;
	float stiffnesses = 0.0f;
	float restingDistances = 6.0f;
	float tearSensitivity = 150.0f;
//	bool test = false;
	for (size_t index = 0; index < m_boneVertices.size(); index += 2)
	{
		PointMass* pointA = new PointMass(m_boneVertices[index].position, m_boneVertices[index].color);
		PointMass* pointB = new PointMass(m_boneVertices[index + 1].position, m_boneVertices[index+1].color);
		pointA->attachTo(pointB, restingDistances, stiffnesses, tearSensitivity);

		pointA->mass = 10.0f;
		pointB->mass = 10.0f;
//		if (!test)
//		{
//			pointA->applyForce(Vec3f(0.0, 100000.0, 0.0));
//			pointB->applyForce(Vec3f(0.0, 100000.0, 0.0));
//		}
//		test = !test;
		m_pointMasses.push_back(pointA);
		m_pointMasses.push_back(pointB);
	}
}


void AnimationActor::UpdateRagdoll(double deltaSeconds)
{
	int fixedDeltaTime = 16;
	float fixedDeltaTimeSeconds = (float)fixedDeltaTime / 1000.0f;
	int leftOverDeltaTime = 0;
	int constraintAccuracy = 3;
	for (int x = 0; x < constraintAccuracy; x++)
	{
		for (size_t index = 0; index < m_pointMasses.size(); index++)
		{
			PointMass* pointmass = m_pointMasses[index];
			pointmass->solveConstraints();
		}
	}

	for (size_t index = 0; index < m_pointMasses.size(); index++)
	{
		PointMass* pointmass = m_pointMasses[index];
		pointmass->updatePhysics(fixedDeltaTimeSeconds);
	}
}


void AnimationActor::BlendBetweenBones(BoneNode* boneA, BoneNode* boneB, float percentage)
{
	boneA->translation = boneA->translation + (boneB->translation - boneA->translation) * percentage;
	boneA->orientation.x = InterpolateOrientation(boneA->orientation.x, boneB->orientation.x, percentage);
	boneA->orientation.y = InterpolateOrientation(boneA->orientation.y, boneB->orientation.y, percentage);
	boneA->orientation.z = InterpolateOrientation(boneA->orientation.z, boneB->orientation.z, percentage);
	boneA->scale = boneA->scale + (boneB->scale - boneA->scale) * percentage;
}


bool AnimationActor::AddFilter(const std::string& animName, const std::string& boneName, int depth /* = 0 */)
{
	std::map<std::string, AnimationInfo>::iterator it = m_animNameInfoMaps.find(animName);
	if (it == m_animNameInfoMaps.end())
		return false;

	BoneNode* node = m_skeleton.GetBoneNodeWithName(boneName);
	if (!node)
		return false;

	BoneIdDepthPair bidp;
	bidp.first = node->id;
	bidp.second = depth;
	AnimationInfo& anim = it->second;
	anim.branchFilter.push_back(bidp);

	return true;
}


}