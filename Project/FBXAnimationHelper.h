#pragma once
# include "fbxsdk.h"
#include <DXUT.h>
#include <map>
#include <list>
#include <string>
#include <math.h>
#include <winuser.h>
#include "FBXHelper.h"
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
static DirectX::XMMATRIX ToXm(const FbxAMatrix& pSrc)
/*
* 存储一Node的变换曲线数据
* 存储SRT各坐标系的变换曲线数据
*/
struct NodeStackTransforms
{
	std::vector<NodeTransform> nodeTransforms;
	std::vector<float> times;
	/*
	* 根据传入的时间算出骨骼点的变换
	*/
	XMMATRIX GetMatrix(float time)
	{
		int startIdx = 0;
		int endIdx = 0;
		time = fmod(time, times.end() - times.begin());
		while (endIdx < times.size() && times[endIdx] < time) {
			endIdx += 1;
			startIdx += 1;
		}
		
	}
};

/*
* index 和FBXHelper得到的骨骼变换列表一致
* 存储一个Layer里面的所有Node的变换信息
*/
typedef std::vector<NodeStackTransforms> AllNodesData;

/*
* 存储一个AnimationStack下多个Layer的信息
* key LayerName的Hash值
* value AllNodesData
导出的时候应该用不到
typedef std::map<int, AllNodesData> NodeAnimationLayersData;
*/



/*
* 存储一个Fbx的所有骨骼变换信息
* StakcData->LayerData->AllNodeTrans
* key stackName的Hash值
* value NodeAnimationLayersData 

*/
typedef std::map<int, AllNodesData> NodeAnimationStacksData;

class FBXAnimationHelper
{
public:

	static NodeStackTransforms GetNodeAnimationTransform(FbxAnimStack *pStack, FbxNode *pNode)
	{
		NodeStackTransforms nodeAnimatinforms;
		FbxTimeSpan timeSpan = pStack->GetLocalTimeSpan();
		FbxTime start = timeSpan.GetStart();
		FbxTime timeI = timeSpan.GetStart();
		FbxTime end = timeSpan.GetStop();
		nodeAnimatinforms.start = start;
		nodeAnimatinforms.end = end;
		FbxTime durationTime = timeSpan.GetDuration();
		for (;timeI < tend; timeI+=durationTime)
		{
			FbxAMatrix fbxMatrix = pNode->EvaluateLocalTransform(timeI);
			DirectX::XMMATRIX dxMatrix = ToXm(fbxMatrix);
			NodeTransform nodeT;
			XMMatrixDecompose(&(nodeT.scales), &(nodeT.quaternion), &(nodeT.translation), dxMatrix);
			nodeAnimatinforms.nodeTransforms.push_back(nodeT);
			nodeAnimatinforms.times.push_back((timeI - start).GetSecondDouble());
		}
		return nodeAnimatinforms;
	}

	static AllNodesData GetAllNodesData(FbxAnimStack *pStack, std::vector<NodeContent> *pNodeContentList)
	{
		AllNodesData allNodesData;
		for (auto it = pNodeContentList->begin(); it != pNodeContentList->end(); it++)
		{
			NodeStackTransforms animTrans = GetNodeAnimationTransform(pStack, it->pNode);
			allNodesData.push_back(animTrans);
		}
		return allNodesData;
	}

	static NodeAnimationLayersData GetNodeLayersData(FbxAnimStack *pStack,std::vector<NodeContent> *pNodeContentList)
	{
		pStack->GetLocalTimeSpan();

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
			pScene->SetCurrentAnimationStack(pStack);
			FbxString stackName = "Animation Stack Name:";
			stackName += pStack->GetName();
			char* cName = stackName.Buffer();
			FormatLog("jbx:stackName %s;", cName);
			std::string sStackName(pStack->GetName());
			int stackNameHash = std::hash<std::string>()(sStackName);
			nodeStacksData[stackNameHash] = GetAllNodesData(pNodeContentList);
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
					NodeStackTransforms nodeT = pNodesData->at(i);
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
