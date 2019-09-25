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
#include "DXUTAni.h"


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

	static FbxNode* GetPRootNode()
	{
		return fbxScene->GetRootNode();
	}

	static FbxNode* GetNodeByName(const char * nodeName,bool recursive = false, bool init = false)
	{
		return fbxScene->GetRootNode()->FindChild(nodeName, recursive, init);
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
	* ��FbxAMatrix�о���תΪ�о���
	* FbxAMatrix::get(row, column),���Դ˷�������
	*/
	static DirectX::XMMATRIX ToXm(const FbxAMatrix& pSrc)
	{
		//����������ϵת��Ϊ��������ϵ���Ƚ���R2��R3��Ȼ�󽻻�C2��C3
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

	static void GetNodeSkeletonNodeTransList(FbxNode *fbxNode, std::vector<NodeContent> *g_pNodeContentList, int parentIdx = -1, bool isRoot = false)
	{
		int idx = parentIdx;
		FbxNodeAttribute *nodeAttr = fbxNode->GetNodeAttribute();
		if (!isRoot && nodeAttr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			NodeContent content;
			content.name = fbxNode->GetName();
			content.parentIdx = parentIdx;
			content.index = g_pNodeContentList->size();
			content.transform = GetLocalTransform(fbxNode);
			g_pNodeContentList->push_back(content);
			idx = content.index;
		}

		for (int i = 0; i < fbxNode->GetChildCount(); i++)
		{
			FbxNode *childNode = fbxNode->GetChild(i);
			const char* nodeName = childNode->GetName();
			GetNodeSkeletonNodeTransList(childNode, g_pNodeContentList, idx);
		}
	}

	static void SetVertexPos(DirectX::XMMATRIX &matrix, SimpleVertex &simpeVertex)
	{
		simpeVertex.Pos = { DirectX::XMVectorGetX(matrix.r[3]),
			DirectX::XMVectorGetY(matrix.r[3]),
			DirectX::XMVectorGetZ(matrix.r[3]) };
	}

	static std::vector<SimpleVertex> GetNodePosList(std::vector<NodeContent> &nodeList)
	{
		int boneCnt = nodeList.size();
		std::vector<DirectX::XMMATRIX> worldMatrixs(boneCnt);
		std::vector<SimpleVertex> posList(boneCnt * 2);
		for (auto &boneContent : nodeList)
		{
			DirectX::XMMATRIX localM, worldM;
			localM = boneContent.transform.ToMatrix();
			if (boneContent.parentIdx >= 0)
			{
				worldM = localM*worldMatrixs[boneContent.parentIdx];
				SetVertexPos(worldMatrixs[boneContent.parentIdx], posList[boneContent.index * 2 ]);
				SetVertexPos(worldM, posList[boneContent.index * 2 + 1] );
			}
			else
			{
				worldM = localM;
				SetVertexPos(DirectX::XMMatrixIdentity(), posList[boneContent.index * 2]);
				SetVertexPos(worldM, posList[boneContent.index * 2 + 1]);
			}
			worldMatrixs[boneContent.index] = worldM;
		}

		return posList;
	}
};