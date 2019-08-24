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

	NodeTransform operator*(float value)
	{
		NodeTransform nodeTrans;
		nodeTrans.scales = { DirectX::XMVectorGetX(scales)*value,
			DirectX::XMVectorGetY(scales)*value,
			DirectX::XMVectorGetZ(scales)*value,
			DirectX::XMVectorGetW(scales)*value };
		nodeTrans.quaternion = { DirectX::XMVectorGetX(quaternion)*value,
			DirectX::XMVectorGetY(quaternion)*value,
			DirectX::XMVectorGetZ(quaternion)*value,
			DirectX::XMVectorGetW(quaternion)*value};
		nodeTrans.translation = { DirectX::XMVectorGetX(translation)*value,
			DirectX::XMVectorGetY(translation)*value,
			DirectX::XMVectorGetZ(translation)*value,
			DirectX::XMVectorGetW(translation)*value};
		return nodeTrans;
	}

	NodeTransform operator+(NodeTransform another)
	{
		NodeTransform nodeTrans;
		nodeTrans.scales = DirectX::XMVectorAdd(scales, another.scales);
		nodeTrans.quaternion = DirectX::XMVectorAdd(quaternion, another.quaternion);
		nodeTrans.translation = DirectX::XMVectorAdd(translation, another.translation);
		return nodeTrans;
	}

};

struct NodeContent
{
	int parentIdx;
	int index;
	NodeTransform transform;
	FbxNode* pNode;
};
