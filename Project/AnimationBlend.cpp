#include "AnimationBlend.h"

void AnimationBlend::AddBlendUnit(BlendUnit & blendUnit)
{
	blendUnits.push_back(blendUnit);
}

std::vector<SimpleVertex>& AnimationBlend::EvaluateNodePos(float time)
{
	resultPosList.clear();
	resultNodeTrans.clear();
	float allWeights = 0;
	//������Ȩ��
	for (auto it = blendUnits.begin(); it != blendUnits.end(); it++)
	{
		allWeights += it->blendValue;
	}
	if (allWeights < 0.00001)
	{
		return resultPosList;
	}
	//���㵥��blendUnit��Ȩ��
	for (auto it = blendUnits.begin(); it != blendUnits.end(); it++)
	{
		it->blendPercent = it->blendValue / allWeights;
	}
	auto firstUnit = blendUnits.begin();
	//�Ƚ���һ��Blend Unit������д��ȥ
	for (auto firstNode = firstUnit->allNodesData.begin(); firstNode != firstUnit->allNodesData.end(); firstNode++)
	{
		resultNodeTrans.push_back(firstNode->GetNodeTransform(time)*firstUnit->blendPercent);
	}
	//��������ÿ��Blend Unit����λ�������ۼ�����
	for (std::vector<BlendUnit>::iterator unitIter = firstUnit+1; unitIter != blendUnits.end(); unitIter++)
	{
		for (int nodeIdx = 0 ; nodeIdx < unitIter->allNodesData.size(); nodeIdx++)
		{
			resultNodeTrans[nodeIdx] = resultNodeTrans[nodeIdx] + unitIter->allNodesData[nodeIdx].GetNodeTransform(time) * unitIter->blendPercent;
		}
	}
	std::vector<DirectX::XMMATRIX> worldMatrixs;
	//Ȼ�������Ϻ��λ��
	for (int nodeIdx =0 ; nodeIdx < resultNodeTrans.size(); nodeIdx++)
	{
		if (nodeContentList[nodeIdx].parentIdx >= 0)
		{
			DirectX::XMMATRIX  parentM = worldMatrixs[nodeContentList[nodeIdx].parentIdx];
			NodeTransform result(resultNodeTrans[nodeIdx].ToMatrix()*parentM);
			worldMatrixs.push_back(result.ToMatrix());
		}
		else
		{
			worldMatrixs.push_back(resultNodeTrans[nodeIdx].ToMatrix());
		}
		
		SimpleVertex pos;
		pos.Pos = { DirectX::XMVectorGetX(worldMatrixs[nodeIdx].r[3]),
			DirectX::XMVectorGetY(worldMatrixs[nodeIdx].r[3]),
			DirectX::XMVectorGetZ(worldMatrixs[nodeIdx].r[3]) };
		SimpleVertex parentPos;
		if (nodeContentList[nodeIdx].parentIdx >=0)
		{
			FBXHelper::SetVertexPos(worldMatrixs[nodeContentList[nodeIdx].parentIdx], parentPos);
		}
		else
		{
			FBXHelper::SetVertexPos(DirectX::XMMatrixIdentity(), parentPos);
		}
		resultPosList.push_back(parentPos);
		resultPosList.push_back(pos);
	}

	return resultPosList;
}

AnimationBlend::AnimationBlend(std::vector<NodeContent> &nodeList)
{
	nodeContentList = nodeList;
}

