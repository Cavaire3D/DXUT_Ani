#pragma once

#include "../Common/Common.h"
#include <fbxsdk.h>
#include "DXUT.h"
#define UseDebugView
#ifdef UseDebugView
#include "windows.h"
#endif
#include <DirectXMath.h>

#define GET_LOCAL_MATRIX(scales, roations, trans, i) XMMatrixScaling((scales[i])->mData[0], (scales[i])->mData[1], (scales[i])->mData[2])* \
	XMMatrixRotationRollPitchYaw((rotations[i])->mData[0], (rotations[i])->mData[1], (rotations[i])->mData[2])* \
	XMMatrixTranslation((trans[i])->mData[0], (trans[i])->mData[1], (trans[i])->mData[2]); \

#define FormatLog(formatStr, args) \
		static char msg[100]; \
		snprintf(msg, 100, (formatStr), (args)); \
		FBXHelper::Log(msg); \

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
		static wchar_t wMsg[256];
		static char copyMsg[256];
		strcpy(copyMsg, logMsg);
		strcat(copyMsg, "\n");
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

	static list<int> GetNodeSkeletonIndexList(FbxNode *fbxNode)
	{
		int index = 0;
		list<int> indexList = new list<int>();
		for (int i= 0; i < fbxNode->GetChildCount(); I++)
		{
			FbxNode *childNode = fbxNode->GetChild(i);
			if (childNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::EType::eSkeleton)
			{
				indexList.push_back(index);
				index++;
			}
		}
		return indexList;
	}

	static FbxAMatrix GetLocalTransform(FbxNode* fbxNode)
	{
		FbxVector4 lTransform, lRotationOffset, lRotationPivot, lPreRotation, lRotation, lPostRotation, lScalingOffset, lScalingPivot, lScaling;
		FbxAMatrix lTransformM, lRotationOffsetM, lRotationPivotM, lPreRotationM, lRotationM, lPostRotationM, lScalingOffsetM, lScalingPivotM, lScalingM;
		lTransform = fbxNode->LclTranslation.Get(); 
		lRotationOffset = fbxNode->RotationOffset.Get();
		lRotationPivot = fbxNode->RotationPivot.Get();
		lPreRotation = fbxNode->PreRotation.Get();
		lRotation = fbxNode->LclRotation.Get();
		lPostRotation = fbxNode->PostRotation.Get();
		lScalingOffset = fbxNode->ScalingOffset.Get();
		lScalingPivot = fbxNode->ScalingPivot.Get();
		lScaling = fbxNode->LclScaling.Get();
		lTransformM.SetT(lTransform);
		lRotationOffsetM.SetT(lRotationOffset);
		lRotationPivotM.SetT(lRotationPivot);
		lPreRotationM.SetR(lPreRotation);
		lRotationM.SetR(lRotation);
		lPostRotationM.SetR(lPostRotation);
		lScalingOffsetM.SetT(lScalingOffset);
		lScalingPivotM.SetT(lScalingOffset);
		lScalingM.SetS(lScaling);
		return lTransformM*lRotationM*lScalingM;
		//return lScalingM*lRotationM*lTransformM;
		/*return lTransformM*lRotationOffsetM*lRotationPivotM*lPreRotationM*lRotationM*lPostRotationM.Inverse()*lRotationPivotM.Inverse()*\
			lScalingOffsetM*lScalingPivotM*lScalingM*lScalingPivotM.Inverse();*/
	}
};