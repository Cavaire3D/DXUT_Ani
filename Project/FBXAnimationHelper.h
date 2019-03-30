#pragma once
# include "fbxsdk.h"
#include <DXUT.h>
#include <map>
#include <list>

using namespace DirectX;

/*
* 存储一Node的变换曲线数据
* key 关键帧的时间
* value NodeTransfrom结构体
*/
typedef std::map<int, NodeTransform> NodeAnimationTransforms;

/*
* index 和FBXHelper得到的骨骼变换列表一致
* 存储一个Layer里面的所有Node的变换信息
*/
typedef std::vector<NodeAnimationTransforms> AllNodesData;

/*
* 存储一个AnimationStack下多个Layer的信息
* key LayerName的Hash值
* value AllNodesData
*/
typedef std::map<int, AllNodesData> NodeAnimationLayersData;


/*
* 存储一个Fbx的所有骨骼变换信息
* StakcData->LayerData->AllNodeTrans
* key stackName的Hash值
* value NodeAnimationLayersData 
*/
typedef std::map<int, NodeAnimationLayersData> NodeAnimationStacksData;

class FBXAnimationHelper
{
public:

	static NodeAnimationTransforms GetNodeAnimationTransform(FbxAnimLayer *pLayer, FbxNode *pNode)
	{
		NodeAnimationTransforms nodeAnimatinforms;
		FbxAnimCurve* pCurveSX, pCurveSY, pCurveSZ, pCurveRX, pCurveRY, pCurveRZ, pCurveTX, pCurveTY, pCurveTZ, ;
		pCurveTX = pNode->LclTranslation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_X);
		pCurveTY = pNode->LclTranslation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		pCurveTZ = pNode->LclTranslation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Z);

		pCurveRX = pNode->LclRotation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_X);
		pCurveRY = pNode->LclRotation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		pCurveRZ = pNode->LclRotation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Z);

		pCurveSX = pNode->LclScaling.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_X);
		pCurveSY = pNode->LclScaling.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		pCurveSZ = pNode->LclScaling.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Z);

		int keyCountTX, keyCountTY, keyCountTZ, keyCountRX, keyCountRY, keyCountRZ, keyCountSX, keyCountSY, keyCountSZ;
		keyCountTX = pCurveTX->KeyGetCount();
		keyCountTY = pCurveTY->KeyGetCount();
		keyCountTZ = pCurveTZ->KeyGetCount();

		keyCountRX = pCurveRX->KeyGetCount();
		keyCountRY = pCurveRY->KeyGetCount();
		keyCountRZ = pCurveRZ->KeyGetCount();

		keyCountSX = pCurveSX->KeyGetCount();
		keyCountSY = pCurveSY->KeyGetCount();
		keyCountSZ = pCurveSZ->KeyGetCount();
		return nodeAnimatinforms;
	}

	static AllNodesData GetAllNodesData(FbxAnimLayer *pLayer, std::list<NodeContent> *pNodeContentList)
	{
		AllNodesData allNodesData;
		for (auto it = pNodeContentList.begin(); it != pNodeContentList.end(); it++)
		{
			NodeAnimationTransforms animTrans = GetNodeAnimationTransform(pLayer, it);
			allNodesData.push_back(animTrans);
		}
		return allNodesData;
	}

	static NodeAnimationLayersData GetNodeLayersData(FbxAnimStack *pStack,std::list<NodeContent> *pNodeContentList)
	{
		NodeAnimationLayersData nodeLayersData;
		int layerCnt = pStack->GetMemberCount<FbxAnimLayer>();
		for (int i = 0; i < layerCnt; i++)
		{
			FbxAnimLayer* pLayer = pStack->GetMember<FbxAnimLayer>(i);
			int cNameHash = std::hash(pLayer->GetName());
			FormatLog("jbx:layerName:%s", pLayer->GetName());
			nodeLayersData[cNameHash] = GetAllNodesData(pLayer, pNodeContentList);
		}
		return nodeLayersData
	}

	static NodeAnimationStacksData GetNodeStacksData(std::list<NodeContent> *pNodeContentList)
	{
		NodeAnimationStacksData nodeStacksData;
		FbxScene* pScene = FBXHelper::GetScene();
		for (int i = 0; i<pScene->GetSrcObjectCount<FbxAnimStack>(); i++)
		{
			FbxAnimStack* pStack = pScene->GetSrcObject<FbxAnimStack>(i);
			FbxString stackName = "Animation Stack Name:";
			stackName += pStack->GetName();
			char* cName = stackName.Buffer();
			FBXHelper::Log(cName);
			int stackNameHash = std::hash(pStack->GetName());
			nodeStacksData[stackNameHash] = GetNodeLayersData(pStack, pNodeContentList);
		}
		return nodeStacksData;
	}
};
