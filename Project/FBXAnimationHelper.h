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

/* ���������ڵ������
* first ʱ��
* second �任��ֵ
*/
typedef std::pair<float, float> CurveNode;

/*
* ��������������
*/
typedef std::vector<CurveNode> CurveData;
static DirectX::XMMATRIX ToXm(const FbxAMatrix& pSrc)
/*
* �洢һNode�ı任��������
* �洢SRT������ϵ�ı任��������
*/
struct NodeStackTransforms
{
	std::vector<NodeTransform> nodeTransforms;
	std::vector<float> times;
	/*
	* ���ݴ����ʱ�����������ı任
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
* index ��FBXHelper�õ��Ĺ����任�б�һ��
* �洢һ��Layer���������Node�ı任��Ϣ
*/
typedef std::vector<NodeStackTransforms> AllNodesData;

/*
* �洢һ��AnimationStack�¶��Layer����Ϣ
* key LayerName��Hashֵ
* value AllNodesData
������ʱ��Ӧ���ò���
typedef std::map<int, AllNodesData> NodeAnimationLayersData;
*/



/*
* �洢һ��Fbx�����й����任��Ϣ
* StakcData->LayerData->AllNodeTrans
* key stackName��Hashֵ
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
	���ݴ����ʱ�������Ӧ����λ��
	* originalTransform Ĭ�Ϲ����ı任
	* pNodeStacksData ��������
	* animationName ����������
	* layer������
	* ʱ��
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
