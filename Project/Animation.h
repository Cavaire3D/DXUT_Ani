#pragma once
#include <string>
#include "NodeTransform.h"
#include <vector>
#include "FBXAnimationHelper.h"
#include "DXUTAni.h"
#include "Skeleton.h"

class Animation
{
public: 
	std::string fbxName;
	bool Init(std::string &fbxName);
	Animation(Skeleton *pSkeleton);
	std::vector<SimpleVertex>* EvalAllNodePos(std::string &stackName, float time);
	std::vector<DirectX::XMMATRIX> outMatrix;
	std::vector<NodeContent> nodeContentList;
	std::vector<SimpleVertex> realTimeVertextList;
	AllNodesData *GetStackAllNodesData(std::string &stackName);
	int boneCnt;
	NodeAnimationStacksData stacksData;
	Skeleton *pSkeleton;
private:
	void ReadAnimationNode(FbxNode *parentNode);
	
};