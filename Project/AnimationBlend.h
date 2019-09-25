#pragma once
#include "Animation.h"
#include <DirectXMath.h>
#include "FBXAnimationHelper.h"
#include "NodeTransform.h"

struct BlendUnit
{
	std::string stackName;
	AllNodesData allNodesData;
	void InitBlendUnit(Animation *pAnimation, std::string &stackName)
	{
		int stackHash = std::hash<std::string>()(stackName);
		NodeAnimationStacksData::iterator it = pAnimation->stacksData.find(stackHash);
		if (it != pAnimation->stacksData.end())
		{
			allNodesData = it->second;
		}
	}
	float blendValue;
	float blendPercent;
};

/*
先只支持两个Pose合成
*/
class AnimationBlend
{
public:
	void AddBlendUnit(BlendUnit &blendUnit);
	std::vector<SimpleVertex> &EvaluateNodePos(float time);
	//用来找父亲节点
	std::vector<NodeContent> nodeContentList;
	std::vector<BlendUnit> blendUnits;
	AnimationBlend(std::vector<NodeContent> &nodeList);
private:
	std::vector<SimpleVertex> resultPosList;
	std::vector<NodeTransform> resultNodeTrans;
};