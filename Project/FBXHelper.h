#pragma once

#include "../Common/Common.h"
#include <fbxsdk.h>
#include "DXUT.h"
#define UseDebugView
#ifdef UseDebugView
#include "windows.h"
#endif
#include <DirectXMath.h>
#include "NodeTransform.h"
#include <list>


#define FormatLog(formatStr, args) \
		{char msg[100]; \
		snprintf(msg, 100, (formatStr), (args)); \
		FBXHelper::Log(msg);} \

class FBXHelper 
{
private:
	static FbxManager* fbxManger;
	static FbxScene* fbxScene;
public:
	static void Init()
	{
		InitializeSdkObjects(fbxManger, fbxScene);
	}

	static FbxManager* GetFbxManager()
	{
		return fbxManger;
	}

	static FbxScene* GetScene()
	{
		return fbxScene;
	}

	static void Log(const char * logMsg)
	{
		wchar_t wMsg[256];
		char copyMsg[256];
		strcpy(copyMsg, logMsg);
		strcat(copyMsg, "\n\0");
		mbstowcs(wMsg, copyMsg, strlen(copyMsg) + 1);
		OutputDebugString(wMsg);
	}

	static bool LoadFbx(const char * fileName)
	{
		FbxIOSettings *ios = FbxIOSettings::Create(fbxManger, IOSROOT);
		fbxManger->SetIOSettings(ios);
		FbxImporter* lImporter = FbxImporter::Create(fbxManger, "");
		if (!lImporter->Initialize(fileName, -1, fbxManger->GetIOSettings()))
		{
			FbxString error = lImporter->GetStatus().GetErrorString();
			Log("Call to FbxImporter::Initialize() failed.\n");
			Log(error.Buffer());
			if(lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
			{
				Log("wrong verison");
			}
			lImporter->Destroy();
			return false;
		}
		lImporter->Import(fbxScene);
		lImporter->Destroy();
		return true;
	}	

	/*
	* 把FbxAMatrix列矩阵转为行矩阵
	* FbxAMatrix::get(row, column),所以此方法可行
	*/
	static DirectX::XMMATRIX ToXm(const FbxAMatrix& pSrc)
	{
		//将右手坐标系转换为左手坐标系，先交换R2，R3，然后交换C2，C3
		return DirectX::XMMatrixSet(
			static_cast<FLOAT>(pSrc.Get(0, 0)), static_cast<FLOAT>(pSrc.Get(0, 2)), static_cast<FLOAT>(pSrc.Get(0, 1)), static_cast<FLOAT>(pSrc.Get(0, 3)),
			static_cast<FLOAT>(pSrc.Get(2, 0)), static_cast<FLOAT>(pSrc.Get(2, 2)), static_cast<FLOAT>(pSrc.Get(2, 1)), static_cast<FLOAT>(pSrc.Get(2, 3)),
			static_cast<FLOAT>(pSrc.Get(1, 0)), static_cast<FLOAT>(pSrc.Get(1, 2)), static_cast<FLOAT>(pSrc.Get(1, 1)), static_cast<FLOAT>(pSrc.Get(1, 3)),
			static_cast<FLOAT>(pSrc.Get(3, 0)), static_cast<FLOAT>(pSrc.Get(3, 2)), static_cast<FLOAT>(pSrc.Get(3, 1)), static_cast<FLOAT>(pSrc.Get(3, 3)));
	}

	static NodeTransform GetLocalTransform(FbxNode* fbxNode)
	{
		FbxAMatrix fbxM = fbxNode->EvaluateLocalTransform();
		DirectX::XMMATRIX lM = ToXm(fbxM);
		NodeTransform nodeT;
		XMMatrixDecompose(&(nodeT.scales), &(nodeT.quaternion), &(nodeT.translation), lM);
		return nodeT;
	}

	static void GetNodeSkeletonNodeTransList(FbxNode *fbxNode, std::vector<NodeContent> *g_pNodeContentList, int parentIdx = -1)
	{
		int idx = -1;
		FbxNodeAttribute *nodeAttr = fbxNode->GetNodeAttribute();
		if (nodeAttr == nullptr ||
			nodeAttr->GetAttributeType() == FbxNodeAttribute::eSkeleton ||
			nodeAttr->GetAttributeType() == FbxNodeAttribute::eNull)
		{
			NodeContent content;
			content.parentIdx = parentIdx;
			content.index = g_pNodeContentList->size();
			content.pNode = fbxNode;
			content.transform = GetLocalTransform(fbxNode);
			g_pNodeContentList->push_back(content);
			idx = content.index;
		}
		
		for (int i= 0; i < fbxNode->GetChildCount(); i++)
		{
			FbxNode *childNode = fbxNode->GetChild(i);
			GetNodeSkeletonNodeTransList(childNode, g_pNodeContentList, idx);
		}
	}
};