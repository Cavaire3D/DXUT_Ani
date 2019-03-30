#pragma once

#include <DirectXMath.h>
#include "fbxsdk.h"

struct NodeTransform
{
	DirectX::XMVECTOR scales;
	DirectX::XMVECTOR quaternion;
	DirectX::XMVECTOR translation;
	DirectX::XMMATRIX ToMatrix()
	{
		DirectX::XMMATRIX sM = DirectX::XMMatrixScalingFromVector(scales);
		DirectX::XMMATRIX rM = DirectX::XMMatrixRotationQuaternion(quaternion);
		DirectX::XMMATRIX tM = DirectX::XMMatrixTranslationFromVector(translation);
		return sM*rM*tM;
	}
};

struct NodeContent
{
	int parentIdx;
	int index;
	NodeTransform transform;
	FbxNode* pNode;
};
