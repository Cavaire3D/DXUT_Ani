#pragma once
# include "fbxsdk.h"
#include <DXUT.h>
#include <map>
#include <list>
#include <string>
#include <math.h>
#include <winuser.h>
#include "DXUTAni.h"

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
		XMVECTOR scale, rotation, transform;
		XMMatrixDecompose(&scale, &rotation, &transform,  originalMatrix);
		float tx = GetCurveValue(time, curveTX,  XMVectorGetX(transform));
		float ty = GetCurveValue(time, curveTY, XMVectorGetY(transform));
		float tz = GetCurveValue(time, curveTZ, XMVectorGetZ(transform));

		XMVECTOR eulerRotation;
		float angle;
		XMQuaternionToAxisAngle(&eulerRotation, &angle, rotation);

		float rx = GetCurveValue(time, curveRX, XMVectorGetX(eulerRotation));
		float ry = GetCurveValue(time, curveRY, XMVectorGetY(eulerRotation));
		float rz = GetCurveValue(time, curveRZ, XMVectorGetZ(eulerRotation));

		float sx = GetCurveValue(time, curveSX, XMVectorGetX(scale));
		float sy = GetCurveValue(time, curveSY, XMVectorGetY(scale));
		float sz = GetCurveValue(time, curveSZ, XMVectorGetZ(scale));

		XMMATRIX scaleM = XMMatrixScaling(sx, sy, sz);
		XMMATRIX rotationM = XMMatrixRotationRollPitchYaw(rx/180.0*3.1415, ry/180.0*3.1415, rz/180.0*3.1415);
		XMMATRIX transM = XMMatrixTranslation(tx, ty, tz);
		return scaleM*rotationM*transM;
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

	static AllNodesData GetAllNodesData(FbxAnimLayer *pLayer, std::vector<NodeContent> *pNodeContentList)
	{
		AllNodesData allNodesData;
		for (auto it = pNodeContentList->begin(); it != pNodeContentList->end(); it++)
		{
			NodeAnimationTransforms animTrans = GetNodeAnimationTransform(pLayer, it->pNode);
			allNodesData.push_back(animTrans);
		}
		return allNodesData;
	}

	static NodeAnimationLayersData GetNodeLayersData(FbxAnimStack *pStack,std::vector<NodeContent> *pNodeContentList)
	{
		NodeAnimationLayersData nodeLayersData;
		int layerCnt = pStack->GetMemberCount<FbxAnimLayer>();
		for (int i = 0; i < layerCnt; i++)
		{
			FbxAnimLayer* pLayer = pStack->GetMember<FbxAnimLayer>(i);
			std::string sPlayerName(pLayer->GetName());
			int cNameHash = std::hash<std::string>()(sPlayerName);
			FormatLog("jbx:LayerName %s", pLayer->GetName());
			nodeLayersData[cNameHash] = GetAllNodesData(pLayer, pNodeContentList);
		}
		return nodeLayersData;
	}

	static void GetNodeStacksData(std::vector<NodeContent> *pNodeContentList, NodeAnimationStacksData &nodeStacksData)
	{
		FbxScene* pScene = FBXHelper::GetScene();
		for (int i = 0; i<pScene->GetSrcObjectCount<FbxAnimStack>(); i++)
		{
			FbxAnimStack* pStack = pScene->GetSrcObject<FbxAnimStack>(i);
			FbxString stackName = "Animation Stack Name:";
			stackName += pStack->GetName();
			char* cName = stackName.Buffer();
			FormatLog("jbx:stackName %s;", cName);
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
	static void EvalAllNodePos(std::vector<DirectX::XMMATRIX> &originalTransform,
		std::vector<NodeContent> &g_pNodeContentList,
		NodeAnimationStacksData &nodeStacksData,
		std::string animationName,
		std::string layerName,
		float time,
		std::vector<SimpleVertex> &vertextList)
	{
		vertextList.clear();
		std::vector<DirectX::XMMATRIX> outMatrix;
		int stackHash = std::hash<std::string>()(animationName);
		NodeAnimationStacksData::iterator stackIter = nodeStacksData.find(stackHash);
		if (stackIter != nodeStacksData.end())
		{
			int layerHash = std::hash<std::string>()(layerName);
			NodeAnimationLayersData::iterator layerIter = stackIter->second.find(layerHash);
			if (layerIter != stackIter->second.end())
			{
				AllNodesData *pNodesData = &(layerIter->second);
				for (int i =0; i < pNodesData->size(); i++)
				{
					XMMATRIX pMatrix = XMMatrixIdentity();
					if (g_pNodeContentList[i].parentIdx >= 0)
					{
						pMatrix = outMatrix[g_pNodeContentList[i].parentIdx];
					}
					NodeAnimationTransforms nodeT = pNodesData->at(i);
					if (i == 12)
					{
						FBXHelper::Log("break----------------------");
					}
					XMMATRIX localM = nodeT.GetMatrix(time, originalTransform[i]);
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
					vertextList.push_back(pVertext);
					vertextList.push_back(cVertext);
					char log[100];
					snprintf(log, 100, "jbx:index%d, pos:%.3f, %.3f, %.3f\n", i, cVertext.Pos.x, cVertext.Pos.y, cVertext.Pos.z);
					FBXHelper::Log(log);
				}
			}
			else
			{
				MessageBox(0, L"Layer Name Error", L"Error", MB_ICONEXCLAMATION);
				exit(-1);
			}
		}
		else
		{
			MessageBox(0, L"Animation Name Error", L"Error", MB_ICONEXCLAMATION);
			exit(-1);
		}
	}
};
