#include "DXUT.h"
#include "FBXHelper.h"
#include "SDKmisc.h"
#include <stdlib.h>
#include <d3d9types.h>
#include "fbxsdk/core/math/fbxaffinematrix.h"
#include <list>
#define FBXSDK_ENV_WINSTORE
#pragma warning( disable : 4100 )

using namespace DirectX;

#define RAND_IN_RANGE(a,b ) (rand() % ((b)-(a)+1))+ (a)

#define USE_GLOBAL_TRANS 0

//记得把内存释放掉
FbxDouble3** trans;
FbxDouble3** rotations;
FbxDouble3** scales;
int nodeCnts = 0;
int* parents;

XMMATRIX                    g_World;
XMMATRIX                    g_View;
XMMATRIX                    g_Projection;
ID3D11VertexShader* g_pVertexShader;
ID3D11PixelShader* g_pPixelShader;
ID3D11InputLayout* g_pVertextLayout;
ID3D11InputLayout* g_pPixelLayout;
int g_lineCnt = 0;
ID3D11SamplerState*         g_pSamplerLinear = nullptr;
ID3D11Buffer*               g_pVertexBuffer = nullptr;
ID3D11Buffer*               g_pCBChangesEveryFrame = nullptr;
FbxAMatrix*          matrixList = nullptr;
struct SimpleVertex
{
	XMFLOAT3 Pos;
};
SimpleVertex* g_pVertexs;


struct CBChangesEveryFrame
{
	XMFLOAT4X4 mWorldViewProj;
	XMFLOAT4X4 mWorld;
};

void ReCalculateVertexs();

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}

void ReadNode(FbxNode *parentNode, int parentIdx, int &totalCnt)
{
	parents[totalCnt] = parentIdx;
#if USE_GLOBAL_TRANS
	FbxAMatrix pm = parentNode->EvaluateGlobalTransform();
	scales[totalCnt] = new FbxDouble3(pm.GetS().mData[0], pm.GetS().mData[1], pm.GetS().mData[2]);
	rotations[totalCnt] = new FbxDouble3(pm.GetR().mData[0], pm.GetR().mData[1], pm.GetR().mData[2]);
	trans[totalCnt] = new FbxDouble3(pm.GetT().mData[0], pm.GetT().mData[1], pm.GetT().mData[2]);
#else
	FbxAMatrix parentM;
	parentM.SetIdentity();
	if (parentIdx >= 0)
	{
		parentM = matrixList[parentIdx];
	}

	FbxAMatrix localMatrix = FBXHelper::GetLocalTransform(parentNode);
	matrixList[totalCnt] = parentM*localMatrix;
	FbxVector4 globalPos = matrixList[totalCnt].GetT();
	trans[totalCnt] = new FbxDouble3(globalPos.mData[0], globalPos.mData[1], globalPos.mData[2]);
	
#endif // USE_GLOBAL_TRANS
	
	totalCnt += 1;
	int index = totalCnt - 1;
	if (parentNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::EType::eSkeleton)
	{
		FBXHelper::Log("isSkeleton");
	}
	else
	{
		FormatLog("Not Skeleton, name:%s", parentNode->GetName());
	}
	for (int i = 0; i < parentNode->GetChildCount(); i++)
	{
		FbxNode* childNode = parentNode->GetChild(i);
		g_lineCnt += 1;
		ReadNode(childNode, index, totalCnt);
	}
}

void ReadFbx()
{
	if (!FBXHelper::LoadFbx("humanoid.fbx"))
	{
		MessageBox(0, L"LoadFbxError", L"Error", MB_ICONEXCLAMATION);
		exit(-1);
	}
	FbxNode* lRootNode = FBXHelper::GetScene()->GetRootNode();
	if (!lRootNode) {
		MessageBox(0, L"LoadFbxError", L"Error", MB_ICONEXCLAMATION);
		exit(-1);
	}
	FbxNode* firstNode = lRootNode->GetChild(0);
	int allNodeCnt = firstNode->GetChildCount(true) + 1; //把父节点也算上
	trans = new FbxDouble3*[allNodeCnt];
	rotations = new FbxDouble3 *[allNodeCnt];
	scales = new FbxDouble3 *[allNodeCnt];
	parents = new int[allNodeCnt];
	matrixList = new FbxAMatrix[allNodeCnt];
	int totalCnt = 0;
	ReadNode(firstNode, -1, totalCnt);
	nodeCnts = totalCnt;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
	ReadFbx();
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	ID3DBlob *pVSBlob;
	HRESULT hr;
	DXUTCompileFromFile(L"SimpleShader.fx", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob);
	hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pVSBlob);
	}
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertextLayout);
	SAFE_RELEASE(pVSBlob);
	if (FAILED(hr))
	{
		return hr;
	}
	pd3dImmediateContext->IASetInputLayout(g_pVertextLayout);

	ID3DBlob *pPSBlob = NULL;
	V_RETURN(DXUTCompileFromFile(L"SimpleShader.fx", NULL, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob));
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	SAFE_RELEASE(pPSBlob);
	if (FAILED(hr))
	{
		return hr;
	}

	g_pVertexs = new SimpleVertex[g_lineCnt * 2];
	ReCalculateVertexs();
	//srand((unsigned)time(NULL));
	//for (int i = 0; i < g_lineCnt * 2; i++)
	//{
	//	g_pVertexs[i].Pos.x = RAND_IN_RANGE(-10, 10);
	//	g_pVertexs[i].Pos.y = RAND_IN_RANGE(-10, 10);
	//	g_pVertexs[i].Pos.z = RAND_IN_RANGE(-10, 10);
	//}
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * g_lineCnt*2;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = g_pVertexs;
	V_RETURN(pd3dDevice->CreateBuffer(&bd, &initData, &g_pVertexBuffer));
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	pd3dImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	V_RETURN(pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame));

	g_World = XMMatrixIdentity();

	XMVECTORF32 eye = { 0.0f, 3.0f, -1000.0f, 0.0f };
	XMVECTORF32 at = { 0.0f, 1.0f, 0.0f };
	XMVECTORF32 up = { 0.0f, 1.0f, 0.0f };
	g_View = XMMatrixLookAtLH(eye, at, up);
	
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	float fAspect = static_cast<float>(pBackBufferSurfaceDesc->Width) / static_cast<float>(pBackBufferSurfaceDesc->Height);
	g_Projection = XMMatrixPerspectiveFovLH(XM_PI * 0.25f, fAspect, 0.1f, 50000);
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_World = XMMatrixRotationY(60.0f * XMConvertToRadians((float)fTime));
}

//重新计算顶点位置
void ReCalculateVertexs()
{
	int parentIdx = -1;
	char strA[50];
	char strB[50];
	char strC[100];
#if USE_GLOBAL_TRANS
	XMMATRIX * localMatrix = new XMMATRIX[nodeCnts];
	localMatrix[0] = GET_LOCAL_MATRIX(scales, rotations, trans, 0);
#endif
	for (int i = 1; i<nodeCnts;i++)
	{
		parentIdx = parents[i];
		
#if USE_GLOBAL_TRANS
		XMMATRIX* pm = &(localMatrix[parentIdx]);
		XMMATRIX m = GET_LOCAL_MATRIX(scales, rotations, trans, i);
		XMMATRIX local = (*pm)*m;
		localMatrix[i] = local;
		g_pVertexs[2 * (i - 1)].Pos.x = trans[i]->mData[0];
		g_pVertexs[2 * (i - 1)].Pos.y = trans[i]->mData[1];
		g_pVertexs[2 * (i - 1)].Pos.z = trans[i]->mData[2];
		snprintf(strA, 50, "pos:%d", 2 * (i - 1));
		snprintf(strB, 50, "x:%.1f,y:%.1f,z:%.1f", g_pVertexs[2 * (i - 1)].Pos.x, g_pVertexs[2 * (i - 1)].Pos.y, g_pVertexs[2 * (i - 1)].Pos.z);
		strcpy(strC, strA);
		strcat(strC, strB);
		FBXHelper::Log(strC);
		g_pVertexs[2 * (i - 1) + 1].Pos.x = trans[parentIdx]->mData[0];
		g_pVertexs[2 * (i - 1) + 1].Pos.y = trans[parentIdx]->mData[1];
		g_pVertexs[2 * (i - 1) + 1].Pos.z = trans[parentIdx]->mData[2];
		//g_pVertexs[2 * (i - 1) + 1].Pos.x = XMVectorGetX(pm->r[3]);
		//g_pVertexs[2 * (i - 1) + 1].Pos.y = XMVectorGetY(pm->r[3]);
		//g_pVertexs[2 * (i - 1) + 1].Pos.z = XMVectorGetZ(pm->r[3]);
#else
		g_pVertexs[2 * (i - 1)].Pos.x = trans[i]->mData[0];
		g_pVertexs[2 * (i - 1)].Pos.y = trans[i]->mData[1];
		g_pVertexs[2 * (i - 1)].Pos.z = trans[i]->mData[2];
		g_pVertexs[2 * (i - 1) + 1].Pos.x = trans[parentIdx]->mData[0];
		g_pVertexs[2 * (i - 1) + 1].Pos.y = trans[parentIdx]->mData[1];
		g_pVertexs[2 * (i - 1) + 1].Pos.z = trans[parentIdx]->mData[2];
#endif // USE_GLOBAL_TRANS
		
		
		
	}
#if USE_GLOBAL_TRANS
	delete localMatrix;
#endif
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
    // Clear render target and the depth stencil 
    auto pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, Colors::MidnightBlue );

    auto pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	HRESULT hr;

	XMMATRIX mvp = g_World*g_View*g_Projection;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V(pd3dImmediateContext->Map(g_pCBChangesEveryFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	CBChangesEveryFrame *pCB = reinterpret_cast<CBChangesEveryFrame*>(MappedResource.pData);
	XMStoreFloat4x4(&pCB->mWorldViewProj, XMMatrixTranspose(mvp));
	XMStoreFloat4x4(&pCB->mWorld, XMMatrixTranspose(g_World));
	pd3dImmediateContext->Unmap(g_pCBChangesEveryFrame, 0);

	pd3dImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->Draw(g_lineCnt*2, 0);
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // DXUT will create and use the best device
    // that is available on the system depending on which D3D callbacks are set below

    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackMouse( OnMouse );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // Perform any application-level initialization here
	FBXHelper::Init();
    DXUTInit( true, true, nullptr ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"DXUT_Ani" );

    // Only require 10-level hardware or later
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 800, 600 );
    DXUTMainLoop(); // Enter into the DXUT ren  der loop

    // Perform any application-level cleanup here

    return DXUTGetExitCode();
}


