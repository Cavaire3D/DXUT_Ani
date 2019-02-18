#pragma once

#include <DirectXMath.h>

struct NodeTransform
{
	XMVECTOR scales;
	XMVECTOR rotations;
	XMVECTOR translations;
};

struct NodeContent
{
	int parentIdx;
	int index;
	NodeTransform transform;
};
