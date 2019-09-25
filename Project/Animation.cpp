#include "DXUT.h"
#include "Animation.h"
#include "FBXHelper.h"
#include <WinUser.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>

Animation::Animation(Skeleton *pSkeleton)
{
	this->pSkeleton = pSkeleton;
	std::vector<NodeContent>& nodeList = pSkeleton->GetNodeList();
	this->nodeContentList.insert(nodeContentList.begin(), nodeList.begin(), nodeList.end());
}

bool Animation::Init(std::string &fbxName)
{
	this->fbxName = fbxName;
	if (!FBXHelper::LoadFbx(fbxName.c_str()))
	{
		MessageBox(0, L"LoadFbxError", L"Error", MB_ICONEXCLAMATION);
		return false;
	}
	FbxNode* lRootNode = FBXHelper::GetScene()->GetRootNode();
	if (!lRootNode) {
		MessageBox(0, L"LoadFbxError", L"Error", MB_ICONEXCLAMATION);
		return false;
	}
	ReadAnimationNode(lRootNode);
	std::vector<FbxNode*> pNodeList;
	for (int i =0; i < lRootNode->GetChildCount(); i++)
	{
		pNodeList.push_back(lRootNode->GetChild(i));
	}
	for (auto pNode : pNodeList)
	{
		pNode->Destroy();
	}
	pNodeList.clear();
	boneCnt = nodeContentList.size();
	return true;
}

std::vector<SimpleVertex>* Animation::EvalAllNodePos(std::string & stackName, float time)
{
	realTimeVertextList.clear();
	outMatrix.clear();
	int stackHash = std::hash<std::string>()(stackName);
	NodeAnimationStacksData::iterator findData = stacksData.find(stackHash);
	if (findData == stacksData.end())
	{
		return &realTimeVertextList;
	}
	AllNodesData nodesData = findData->second;
	for (int i = 0; i < nodesData.size(); i++)
	{
		XMMATRIX pMatrix = XMMatrixIdentity();
		if (nodeContentList[i].parentIdx >= 0)
		{
			pMatrix = outMatrix[nodeContentList[i].parentIdx];
		}
		XMMATRIX localM = nodesData[i].GetMatrix(time);
		outMatrix.push_back(localM * pMatrix);

		XMVECTOR outS, outQ, outT;
		XMMatrixDecompose(&outS, &outQ, &outT, pMatrix);
		SimpleVertex pVertext;
		pVertext.Pos.x = XMVectorGetX(outT);
		pVertext.Pos.y = XMVectorGetY(outT);
		pVertext.Pos.z = XMVectorGetZ(outT);
		SimpleVertex cVertext;
		XMMatrixDecompose(&outS, &outQ, &outT, outMatrix[i]);
		cVertext.Pos.x = XMVectorGetX(outT);
		cVertext.Pos.y = XMVectorGetY(outT);
		cVertext.Pos.z = XMVectorGetZ(outT);
		realTimeVertextList.push_back(pVertext);
		realTimeVertextList.push_back(cVertext);
	}
	return &realTimeVertextList;
}

AllNodesData * Animation::GetStackAllNodesData(std::string & stackName)
{
	int hash = std::hash<std::string>()(stackName);
	NodeAnimationStacksData::iterator it = stacksData.find(hash);
	if (it != stacksData.end())
	{
		return &(it->second);
	}
	return nullptr;
}

void Animation::ReadAnimationNode(FbxNode * parentNode)
{
	FBXAnimationHelper::GetNodeStacksData(&nodeContentList, stacksData);
}
