#pragma once
#include <vector>
#include "NodeTransform.h"
#include <map>
#include "DXUTAni.h"


class Skeleton
{
private:
	int boneCnt;
	std::vector<NodeContent> nodeList;
	std::map<std::string, int> nodeMap;
	void ReadSkeletonNode(FbxNode * parentNode);
public:
	Skeleton(std::string &fbxPath);
	std::vector<NodeContent>& GetNodeList()
	{
		return nodeList;
	}
	int GetNodeInedx(std::string &name);
	int getBoneCnt() { return boneCnt; };
	NodeContent* GetNode(int index);
	NodeContent* GetNode(std::string &name);
};