#pragma once
# include "fbxsdk.h"
#include <DXUT.h>
#include <map>
#include <list>

using namespace DirectX;

/*
* �洢һNode�ı任��������
* key �ؼ�֡��ʱ��
* value NodeTransfrom�ṹ��
*/
typedef std::map<int, NodeTransform> NodeAnimationTransforms;

/*
* index ��FBXHelper�õ��Ĺ����任�б�һ��
* �洢һ��Layer���������Node�ı任��Ϣ
*/
typedef std::vector<NodeAnimationTransforms> AllNodesData;

/*
* �洢һ��AnimationStack�¶��Layer����Ϣ
* key LayerName��Hashֵ
* value AllNodesData
*/
typedef std::map<int, AllNodesData> NodeAnimationLayersData;


/*
* �洢һ��Fbx�����й����任��Ϣ
* StakcData->LayerData->AllNodeTrans
* key stackName��Hashֵ
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
