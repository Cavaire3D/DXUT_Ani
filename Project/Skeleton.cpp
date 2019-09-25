#include "DXUT.h"
#include "Skeleton.h"
#include "FBXHelper.h"
#include "fbxsdk/scene/geometry/fbxnode.h"
#include "fbxsdk/scene/fbxscene.h"
#include "FBXAnimationHelper.h"
#include <WinUser.h>

void Skeleton::ReadSkeletonNode(FbxNode * parentNode)
{
	FBXHelper::GetNodeSkeletonNodeTransList(parentNode, &nodeList, -1, true);
}

Skeleton::Skeleton(std::string & fbxName)
{
	if (!FBXHelper::LoadFbx(fbxName.c_str()))
	{
		MessageBox(0, L"LoadFbxError", L"Error", MB_ICONEXCLAMATION);
	}
	FbxNode* lRootNode = FBXHelper::GetScene()->GetRootNode();
	if (!lRootNode) {
		MessageBox(0, L"LoadFbxError", L"Error", MB_ICONEXCLAMATION);
	}
	ReadSkeletonNode(lRootNode);
	std::vector<FbxNode*> pNodeList;
	for (int i = 0; i < lRootNode->GetChildCount(); i++)
	{
		pNodeList.push_back(lRootNode->GetChild(i));
	}
	for (auto pNode : pNodeList)
	{
		pNode->Destroy();
	}
	pNodeList.clear();
	boneCnt = nodeList.size();
}

int Skeleton::GetNodeInedx(std::string & name)
{
	return 0;
}

NodeContent * Skeleton::GetNode(int index)
{
	return nullptr;
}

NodeContent * Skeleton::GetNode(std::string & name)
{
	return nullptr;
}
