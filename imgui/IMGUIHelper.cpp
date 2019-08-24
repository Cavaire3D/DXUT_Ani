#include "IMGUIHelper.h"

bool IMGUIHelper::show_demo_window = true;
bool IMGUIHelper::show_another_window = false;
ImVec4 IMGUIHelper::clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);
ImGUIData* IMGUIHelper::data = new ImGUIData();

void IMGUIHelper::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	DXUTGetDXGISwapChain()->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	auto pTargetView = DXUTGetD3D11RenderTargetView();
	DXUTGetD3D11Device()->CreateRenderTargetView(pBackBuffer, NULL, &pTargetView);
	pBackBuffer->Release();
}

void IMGUIHelper::ImGUIResize()
{
	DXUTGetDXGISwapChain()->ResizeBuffers(0, (UINT)LOWORD(DXUTGetWindowWidth()), (UINT)HIWORD(DXUTGetWindowHeight()), DXGI_FORMAT_UNKNOWN, 0);
	CreateRenderTarget();
}

void IMGUIHelper::ImGUIInit()
{
	// ---------------------------imgui start--------------------------------------
	IMGUI_CHECKVERSION();
	auto pd3dDevice = DXUTGetD3D11Device();
	data->cameraCirecle = 100;
	data->cameraPos[0] = 0;
	data->cameraPos[1] = 34;
	data->cameraPos[2] = -19;
	CreateRenderTarget();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(DXUTGetHWND());
	ImGui_ImplDX11_Init(pd3dDevice, DXUTGetD3D11DeviceContext());
	// ---------------------------imgui end----------------------------------------
}

void IMGUIHelper::ImGUIUpdate()
{
	auto pd3dDevice = DXUTGetD3D11Device();
	if (pd3dDevice != nullptr)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		//if (show_demo_window)
			//ImGui::ShowDemoWindow(&show_demo_window);
		{
			//static float f = 0.0f;
			//static int counter = 0;

			//ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			//ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			//ImGui::Checkbox("Another Window", &show_another_window);

			//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			//	counter++;
			//ImGui::SameLine();
			//ImGui::Text("counter = %d", counter);

			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			//ImGui::End();
			ImGui::SliderFloat("cameraPos:x", &(data->cameraPos[0]), -data->cameraCirecle, data->cameraCirecle);
			ImGui::SliderFloat("cameraPos:y", &(data->cameraPos[1]), -data->cameraCirecle, data->cameraCirecle);
			ImGui::SliderFloat("cameraPos:z", &(data->cameraPos[2]), -data->cameraCirecle, data->cameraCirecle);
			ImGui::SliderFloat("blendPercent", &(data->blendPercent), 0, 1);
		}
		//if (show_another_window)
		//{
		//	ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		//	ImGui::Text("Hello from another window!");
		//	if (ImGui::Button("Close Me"))
		//		show_another_window = false;
		//	ImGui::End();
		//}
	}
}

void IMGUIHelper::ImGUIDraw()
{
	ImGui::Render();
	auto pRTV = DXUTGetD3D11RenderTargetView();
	DXUTGetD3D11DeviceContext()->OMSetRenderTargets(1, &pRTV, NULL);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void IMGUIHelper::ImGUIClose()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}