#pragma once
# include "fbxsdk.h"
#include <DXUT.h>
#include <map>
#include <list>
#include <string>
#include <math.h>

using namespace DirectX;

/* 单个动画节点的数据
* first 时间
* second 变换的值
*/
typedef std::pair<float, float> CurveNode;

/*
* 动画的曲线数据
*/
typedef std::vector<CurveNode> CurveData;

/*
* 存储一Node的变换曲线数据
* 存储SRT各坐标系的变换曲线数据
*/
struct NodeAnimationTransforms
{
	CurveData curveTX;
	CurveData curveTY;
	CurveData curveTZ;

	CurveData curveRX;
	CurveData curveRY;
	CurveData curveRZ;

	CurveData curveSX;
	CurveData curveSY;
	CurveData curveSZ;

	//todo 先用线性插值算一版看看效果
	float GetCurveValue(float time, CurveData& curveData, float originalValue)
	{
		if (!curveData.size())
		{
			return originalValue;
		}
		float timeLen = curveData.back().first;
		float loopTime = std::fmod(time, timeLen);
		CurveData::iterator prev = curveData.begin();
		CurveData::iterator next = curveData.end();
		for (auto it = curveData.begin(); it != curveData.end(); it++)
		{
			if ((it->first - loopTime) > 0.00001)
			{
				next = it;
				break;
			}
			prev = it;
		}
		float percent = (loopTime - prev->second) / (next->first - prev->second);
		return prev->second + (next->second - prev->second)*percent;
	}

	/*
	* 根据传入的时间算出骨骼点的变换
	*/
	XMMATRIX GetMatrix(float time, XMMATRIX& originalMatrix)
	{
		XMMATRIX matrix;
		XMVECTOR scale, rotation, transform;
		XMMatrixDecompose(&scale, &rotation, &transform,  originalMatrix);
		float tx = GetCurveValue(time, curveTX,  XMVectorGetX(transform));
		float ty = GetCurveValue(time, curveTX, XMVectorGetY(transform));
		float tz = GetCurveValue(time, curveTX, XMVectorGetZ(transform));

		XMVECTOR eulerRotation;
		float angle;
		XMQuaternionToAxisAngle(&eulerRotation, &angle, rotation);

		return matrix;
	}
};

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

	static CurveData GetCurveData(FbxAnimCurve* pCurve)
	{
		CurveData curveData;
		if (pCurve)
		{
			for (int keyIndex = 0; keyIndex < pCurve->KeyGetCount(); keyIndex++)
			{
				CurveNode node;
				node.first = pCurve->KeyGetTime(keyIndex).GetSecondDouble();
				node.second = pCurve->KeyGetValue(keyIndex);
				curveData.push_back(node);
			}
		}
		else
		{
		}
		return curveData;
	}

	static NodeAnimationTransforms GetNodeAnimationTransform(FbxAnimLayer *pLayer, FbxNode *pNode)
	{
		NodeAnimationTransforms nodeAnimatinforms;
		FbxAnimCurve* pCurveSX, *pCurveSY, *pCurveSZ, *pCurveRX, *pCurveRY, *pCurveRZ, *pCurveTX, *pCurveTY, *pCurveTZ ;
		pCurveTX = pNode->LclTranslation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_X);
		pCurveTY = pNode->LclTranslation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		pCurveTZ = pNode->LclTranslation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Z);

		pCurveRX = pNode->LclRotation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_X);
		pCurveRY = pNode->LclRotation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		pCurveRZ = pNode->LclRotation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Z);

		pCurveSX = pNode->LclScaling.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_X);
		pCurveSY = pNode->LclScaling.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		pCurveSZ = pNode->LclScaling.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Z);

		nodeAnimatinforms.curveTX = GetCurveData(pCurveTX);
		nodeAnimatinforms.curveTY = GetCurveData(pCurveTY);
		nodeAnimatinforms.curveTZ = GetCurveData(pCurveTZ);

		nodeAnimatinforms.curveRX = GetCurveData(pCurveRX);
		nodeAnimatinforms.curveRY = GetCurveData(pCurveRY);
		nodeAnimatinforms.curveRZ = GetCurveData(pCurveRZ);

		nodeAnimatinforms.curveSX = GetCurveData(pCurveSX);
		nodeAnimatinforms.curveSY = GetCurveData(pCurveSY);
		nodeAnimatinforms.curveSZ = GetCurveData(pCurveSZ);

		if (nodeAnimatinforms.curveRX.size() == nodeAnimatinforms.curveRY.size()&&
			nodeAnimatinforms.curveRZ.size() == nodeAnimatinforms.curveRY.size() &&
			nodeAnimatinforms.curveTX.size() == nodeAnimatinforms.curveTY.size() &&
			nodeAnimatinforms.curveTY.size() == nodeAnimatinforms.curveTZ.size() &&
			nodeAnimatinforms.curveSX.size() == nodeAnimatinforms.curveSY.size() &&
			nodeAnimatinforms.curveSZ.size() == nodeAnimatinforms.curveSY.size())
		{

		}
		else
		{
			//FBXHelper::Log("jbx: Curve Len Not Equal");
		}

		return nodeAnimatinforms;
	}

	static AllNodesData GetAllNodesData(FbxAnimLayer *pLayer, std::list<NodeContent> *pNodeContentList)
	{
		AllNodesData allNodesData;
		for (auto it = pNodeContentList->begin(); it != pNodeContentList->end(); it++)
		{
			NodeAnimationTransforms animTrans = GetNodeAnimationTransform(pLayer, it->pNode);
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
			std::string sPlayerName(pLayer->GetName());
			int cNameHash = std::hash<std::string>()(sPlayerName);
			FormatLog("jbx:%s", pLayer->GetName());
			nodeLayersData[cNameHash] = GetAllNodesData(pLayer, pNodeContentList);
		}
		return nodeLayersData;
	}

	static void GetNodeStacksData(std::list<NodeContent> *pNodeContentList, NodeAnimationStacksData &nodeStacksData)
	{
		FbxScene* pScene = FBXHelper::GetScene();
		for (int i = 0; i<pScene->GetSrcObjectCount<FbxAnimStack>(); i++)
		{
			FbxAnimStack* pStack = pScene->GetSrcObject<FbxAnimStack>(i);
			FbxString stackName = "Animation Stack Name:";
			stackName += pStack->GetName();
			char* cName = stackName.Buffer();
			FormatLog("jbx:%s;", cName);
			std::string sStackName(pStack->GetName());
			int stackNameHash = std::hash<std::string>()(sStackName);
			nodeStacksData[stackNameHash] = GetNodeLayersData(pStack, pNodeContentList);
		}
	}
	/*
	根据传入的时间算出对应骨骼位置
	* originalTransform 默认骨骼的变换
	* pNodeStacksData 动画数据
	* animationName 动画的名字
	* layer的名字
	* 时间
	*/
	static std::vector<DirectX::XMMATRIX> EvalAllNodePos(std::vector<DirectX::XMMATRIX> &originalTransform,
		NodeAnimationStacksData *pNodeStacksData,
		char* animationName,
		char* layerName,
		float time)
	{
		std::vector<DirectX::XMMATRIX> currentTrans;
		return currentTrans;
	}
};
