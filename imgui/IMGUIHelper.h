#pragma once
#include "imgui.h"
#include "DXUT.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <DirectXMath.h>

/*
用来存储和DXUT交互的数据
*/
struct ImGUIData
{
	float cameraPos[3];
	float cameraCirecle;
	float blendPercent;
};

class IMGUIHelper
{
public:
	static bool show_demo_window;
	static bool show_another_window;
	static ImVec4 clear_color;
	static void CreateRenderTarget();

	static void ImGUIResize();
	static void ImGUIInit();

	static void ImGUIUpdate();

	static void ImGUIDraw();

	static void ImGUIClose();

	static ImGUIData* GetData()
	{
		return data;
	}
private:
	static ImGUIData *data;
};
