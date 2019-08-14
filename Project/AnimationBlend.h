#pragma once
#include "Animation.h"
#include <DirectXMath.h>
#include "FBXAnimationHelper.h"

struct BlendUnit
{
	std::string stackName;
	AllNodesData allNodesData;
	void InitBlendUnit(Animation *pAnimation, std::string &stackName)
	{

	}
	float blendValue;
};

/*
��ֻ֧������Pose�ϳ�
*/
class AnimationBlend
{
public:
	void AddBlendUnit(BlendUnit &blendUnit);
	std::vector<SimpleVertex> *EvaluateNodePos();
private:
	std::vector<BlendUnit> blendUnits;
	
};