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
/*
* �洢һNode�ı任��������
* �洢SRT������ϵ�ı任��������
*/
struct NodeStackTransforms
{
	std::vector<NodeTransform> nodeTransforms;
	std::vector<float> keyTimes;
	/*
	* ���ݴ����ʱ�����������ı任
	*/
	XMMATRIX GetMatrix(float time)
	{
		int startIdx = 0;
		int endIdx = 0;
		time = fmod(time, keyTimes[keyTimes.size() - 1] - keyTimes[0]);
		while (endIdx < keyTimes.size() && keyTimes.at(endIdx) < time) {
			endIdx += 1;
		}
		endIdx = endIdx < (keyTimes.size() - 1) ? endIdx : (keyTimes.size() - 1);
		if (endIdx == 0) {
			DirectX::XMMATRIX sM = DirectX::XMMatrixScalingFromVector(nodeTransforms[0].scales);
			DirectX::XMMATRIX rM = DirectX::XMMatrixRotationQuaternion(nodeTransforms[0].quaternion);
			DirectX::XMMATRIX tM = DirectX::XMMatrixTranslationFromVector(nodeTransforms[0].translation);
			return sM*rM*tM;
		}
		else {
			startIdx = endIdx - 1;
			float lerpPercent = (time - keyTimes[startIdx]) / (keyTimes[endIdx] - keyTimes[startIdx]);
			XMVECTOR scale = XMVectorLerp(nodeTransforms[startIdx].scales, nodeTransforms[endIdx].scales, lerpPercent);
			XMVECTOR quater = XMQuaternionSlerp(nodeTransforms[startIdx].quaternion, nodeTransforms[endIdx].quaternion, lerpPercent);
			XMVECTOR tranlation = XMVectorLerp(nodeTransforms[startIdx].translation, nodeTransforms[endIdx].translation, lerpPercent);
			DirectX::XMMATRIX sM = DirectX::XMMatrixScalingFromVector(scale);
			DirectX::XMMATRIX rM = DirectX::XMMatrixRotationQuaternion(quater);
			DirectX::XMMATRIX tM = DirectX::XMMatrixTranslationFromVector(tranlation);
			return sM*rM*tM;
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
		FbxTime durationTime;
		durationTime.SetSecondDouble(1.0 / 30.0);
		for (;timeI < end; timeI+=durationTime)
		{
			FbxAMatrix fbxMatrix = pNode->EvaluateLocalTransform(timeI);
			DirectX::XMMATRIX dxMatrix = FBXHelper::ToXm(fbxMatrix);
			NodeTransform nodeT;
			XMMatrixDecompose(&(nodeT.scales), &(nodeT.quaternion), &(nodeT.translation), dxMatrix);
			nodeAnimatinforms.nodeTransforms.push_back(nodeT);
			nodeAnimatinforms.keyTimes.push_back((timeI - start).GetSecondDouble());
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
			nodeStacksData[stackNameHash] = GetAllNodesData(pStack, pNodeContentList);
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
		float time,
		std::vector<SimpleVertex> &vertextList)
	{
		vertextList.clear();
		std::vector<DirectX::XMMATRIX> outMatrix;
		int stackHash = std::hash<std::string>()(animationName);
		NodeAnimationStacksData::iterator stacksData = nodeStacksData.find(stackHash);
		if (stacksData == nodeStacksData.end())
		{
			return;
		}
		AllNodesData nodesData = stacksData->second;
		for (int i= 0;i < nodesData.size(); i++)
		{
			XMMATRIX pMatrix = XMMatrixIdentity();
			if (g_pNodeContentList[i].parentIdx >= 0)
			{
				pMatrix = outMatrix[g_pNodeContentList[i].parentIdx];
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
			vertextList.push_back(pVertext);
			vertextList.push_back(cVertext);
		}
	}
};
