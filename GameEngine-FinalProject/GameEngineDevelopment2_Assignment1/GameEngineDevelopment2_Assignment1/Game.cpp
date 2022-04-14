/*
-------------------------------------------------------------------------
//Final Assignment
// author : Negar Saeidi - 101261396  and  NeginSaeidi - 101261395
//Game.cpp
//Handles the directX computations
*/

#include "Game.hpp"

#include "StateStack.hpp"
const int gNumFrameResources = 3;

Game::Game(HINSTANCE hInstance)
	: D3DApp(hInstance)
	, mStateStack(State::Context(&mPlayer, this))
{
}

Game::~Game()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

bool Game::Initialize()
{
	if (!D3DApp::Initialize())
		return false;


	
	mCamera.SetPosition(0.0f, 15.0f, -5.0f);
	mCamera.Pitch(XM_PI / 3);

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildConeGeometry();
	BuildCylinderGeometry();
	BuildQubeGeometry();
	BuildSphereGeometry();
	BuildMaterials();
	RegisterStates();

	//BuildRenderItems();
	//BuildFrameResources();
	BuildPSOs();
	mPlayer = Player();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	mStateStack.pushState(States::Title);

	return true;
}

void Game::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	//XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	//XMStoreFloat4x4(&mProj, P);

	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void Game::Update(const GameTimer& gt)
{
	//ProcessInput();
	//mWorld.update(gt);

	mStateStack.update(gt);

	if (mStateStack.isEmpty())
	{
		exit(0);
	}

	UpdateCamera(gt);


	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);


}

void Game::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	//step5
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	mStateStack.draw();
	//step5
	//DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["alphaTested"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTested]);


	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Game::RegisterStates()
{
	mStateStack.registerState<TitleState>(States::Title);
	mStateStack.registerState<MenuState>(States::Menu);
	mStateStack.registerState<GameState>(States::Game);
	mStateStack.registerState<PauseState>(States::Pause);
	
}

void Game::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Game::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Game::OnMouseMove(WPARAM btnState, int x, int y)
{

}

void Game::OnKeyboardInput(WPARAM btnState)
{
	FlushCommandQueue();
	mStateStack.handleEvent(btnState);
}



void Game::UpdateCamera(const GameTimer& gt)
{

	mCamera.UpdateViewMatrix();

}

void Game::AnimateMaterials(const GameTimer& gt)
{

}

void Game::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for (auto& e : mStateStack.GetStateStack()->front().get()->mAllRitems)
	{
		/*mRitemLayer[(int)RenderLayer::Opaque].push_back(e.get());
		mRitemLayer[(int)RenderLayer::AlphaTested].push_back(e.get());*/
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void Game::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}


void Game::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mCamera.GetPosition3f();
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	mMainPassCB.Lights[0].Position= { 0.6f, 10.6f, 0.6f };
	mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void Game::LoadTextures()
{
	//Eagle
	auto EagleTex = std::make_unique<Texture>();
	EagleTex->Name = "EagleTex";
	EagleTex->Filename = L"../../Textures/Eagle.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), EagleTex->Filename.c_str(),
		EagleTex->Resource, EagleTex->UploadHeap));

	mTextures[EagleTex->Name] = std::move(EagleTex);

	//Raptor
	auto RaptorTex = std::make_unique<Texture>();
	RaptorTex->Name = "RaptorTex";
	RaptorTex->Filename = L"../../Textures/Red.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), RaptorTex->Filename.c_str(),
		RaptorTex->Resource, RaptorTex->UploadHeap));

	mTextures[RaptorTex->Name] = std::move(RaptorTex);

	//Desert
	auto DesertTex = std::make_unique<Texture>();
	DesertTex->Name = "DesertTex";
	DesertTex->Filename = L"../../Textures/Desert.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), DesertTex->Filename.c_str(),
		DesertTex->Resource, DesertTex->UploadHeap));

	mTextures[DesertTex->Name] = std::move(DesertTex);

	////TitleBackground
	auto TitleBackgroundTex = std::make_unique<Texture>();
	TitleBackgroundTex->Name = "TitleBackground";
	TitleBackgroundTex->Filename = L"../../Textures/TitleBackground.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), TitleBackgroundTex->Filename.c_str(),
		TitleBackgroundTex->Resource, TitleBackgroundTex->UploadHeap));

	mTextures[TitleBackgroundTex->Name] = std::move(TitleBackgroundTex);

	//TitlePrompt
	auto TitlePromptTex = std::make_unique<Texture>();
	TitlePromptTex->Name = "TitlePrompt";
	TitlePromptTex->Filename = L"../../Textures/TitlePrompt1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), TitlePromptTex->Filename.c_str(),
		TitlePromptTex->Resource, TitlePromptTex->UploadHeap));

	mTextures[TitlePromptTex->Name] = std::move(TitlePromptTex);

	//Menu play button
	auto MenuPlayTex = std::make_unique<Texture>();
	MenuPlayTex->Name = "MenuPlay";
	MenuPlayTex->Filename = L"../../Textures/Play.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), MenuPlayTex->Filename.c_str(),
		MenuPlayTex->Resource, MenuPlayTex->UploadHeap));

	mTextures[MenuPlayTex->Name] = std::move(MenuPlayTex);


	//Menu quit button
	auto MenuQuitTex = std::make_unique<Texture>();
	MenuQuitTex->Name = "MenuQuit";
	MenuQuitTex->Filename = L"../../Textures/Quit.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), MenuQuitTex->Filename.c_str(),
		MenuQuitTex->Resource, MenuQuitTex->UploadHeap));

	mTextures[MenuQuitTex->Name] = std::move(MenuQuitTex);

	//Menu airplane arrow
	auto MenuArrowTex = std::make_unique<Texture>();
	MenuArrowTex->Name = "Airplane";
	MenuArrowTex->Filename = L"../../Textures/airplane.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), MenuArrowTex->Filename.c_str(),
		MenuArrowTex->Resource, MenuArrowTex->UploadHeap));

	mTextures[MenuArrowTex->Name] = std::move(MenuArrowTex);

	//PausePrompt
	auto PausePromptTex = std::make_unique<Texture>();
	PausePromptTex->Name = "PausePrompt";
	PausePromptTex->Filename = L"../../Textures/PausePrompt.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), PausePromptTex->Filename.c_str(),
		PausePromptTex->Resource, PausePromptTex->UploadHeap));

	mTextures[PausePromptTex->Name] = std::move(PausePromptTex);

	//MenuBackground
	auto SilverTex = std::make_unique<Texture>();
	SilverTex->Name = "SilverTex";
	SilverTex->Filename = L"../../Textures/Silver.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), SilverTex->Filename.c_str(),
		SilverTex->Resource, SilverTex->UploadHeap));

	mTextures[SilverTex->Name] = std::move(SilverTex);

	//MenuBackground
	auto Blue = std::make_unique<Texture>();
	Blue->Name = "Blue";
	Blue->Filename = L"../../Textures/Blue.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), Blue->Filename.c_str(),
		Blue->Resource, Blue->UploadHeap));

	mTextures[Blue->Name] = std::move(Blue);

	//MenuBackground
	auto Sky = std::make_unique<Texture>();
	Sky->Name = "Sky";
	Sky->Filename = L"../../Textures/Sky.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), Sky->Filename.c_str(),
		Sky->Resource, Sky->UploadHeap));

	mTextures[Sky->Name] = std::move(Sky);




	//Planet 
	auto Planet = std::make_unique<Texture>();
	Planet->Name = "Planet";
	Planet->Filename = L"../../Textures/Planet.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), Planet->Filename.c_str(),
		Planet->Resource, Planet->UploadHeap));

	mTextures[Planet->Name] = std::move(Planet);

}

void Game::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	//The Init function of the CD3DX12_ROOT_SIGNATURE_DESC class has two parameters that allow you to
		//define an array of so - called static samplers your application can use.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),  //6 samplers!
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

//Once a texture resource is created, we need to create an SRV descriptor to it which we
//can set to a root signature parameter slot for use by the shader programs.
void Game::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 13;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto EagleTex = mTextures["EagleTex"]->Resource;
	auto RaptorTex = mTextures["RaptorTex"]->Resource;
	auto DesertTex = mTextures["DesertTex"]->Resource;
	auto TitleBackgroundTex = mTextures["TitleBackground"]->Resource;
	auto TitlePromptTex = mTextures["TitlePrompt"]->Resource;
	auto MenuPlayTex = mTextures["MenuPlay"]->Resource;
	auto MenuQuitTex = mTextures["MenuQuit"]->Resource;
	auto MenuArrowTex = mTextures["Airplane"]->Resource;
	auto PausePromptTex = mTextures["PausePrompt"]->Resource;
	auto SilverTex = mTextures["SilverTex"]->Resource;
	auto Blue = mTextures["Blue"]->Resource;
	auto Sky = mTextures["Sky"]->Resource;
	auto Planet = mTextures["Planet"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	//This mapping enables the shader resource view (SRV) to choose how memory gets routed to the 4 return components in a shader after a memory fetch.
	//When a texture is sampled in a shader, it will return a vector of the texture data at the specified texture coordinates.
	//This field provides a way to reorder the vector components returned when sampling the texture.
	//D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING  will not reorder the components and just return the data in the order it is stored in the texture resource.
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Format = EagleTex->GetDesc().Format;

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	//The number of mipmap levels to view, starting at MostDetailedMip.This field, along with MostDetailedMip allows us to
	//specify a subrange of mipmap levels to view.You can specify - 1 to indicate to view
	//all mipmap levels from MostDetailedMip down to the last mipmap level.

	srvDesc.Texture2D.MipLevels = EagleTex->GetDesc().MipLevels;

	//Specifies the minimum mipmap level that can be accessed. 0.0 means all the mipmap levels can be accessed.
	//Specifying 3.0 means mipmap levels 3.0 to MipCount - 1 can be accessed.
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	md3dDevice->CreateShaderResourceView(EagleTex.Get(), &srvDesc, hDescriptor);

	//Raptor Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = RaptorTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(RaptorTex.Get(), &srvDesc, hDescriptor);

	//Desert Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = DesertTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(DesertTex.Get(), &srvDesc, hDescriptor);


	////Title Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = TitleBackgroundTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(TitleBackgroundTex.Get(), &srvDesc, hDescriptor);

	//Title Prompt Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = TitlePromptTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(TitlePromptTex.Get(), &srvDesc, hDescriptor);

	//Play text Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = MenuPlayTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(MenuPlayTex.Get(), &srvDesc, hDescriptor);

	//Quit Text Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = MenuQuitTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(MenuQuitTex.Get(), &srvDesc, hDescriptor);

	//Arrow text Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = MenuArrowTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(MenuArrowTex.Get(), &srvDesc, hDescriptor);

	//PausePrompt Text Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = PausePromptTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(PausePromptTex.Get(), &srvDesc, hDescriptor);

	//Setting Arrow Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = SilverTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(SilverTex.Get(), &srvDesc, hDescriptor);

	//Setting Label Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = Blue->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(Blue.Get(), &srvDesc, hDescriptor);

	//Setting Return Texture Descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = Sky->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(Sky.Get(), &srvDesc, hDescriptor);

	//Pause DisplayDescriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = Planet->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(Planet.Get(), &srvDesc, hDescriptor);

}

void Game::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};
	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_1");



	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

}

void Game::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	//GeometryGenerator::MeshData box = geoGen.CreateQuad(0.0f, 0.0f, 20.0f, 10.0f, 10.0f);
	GeometryGenerator::MeshData box = geoGen.CreateBox(1, 0, 1, 1);
	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = 0;
	boxSubmesh.BaseVertexLocation = 0;


	std::vector<Vertex> vertices(box.Vertices.size());

	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices = box.GetIndices16();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void Game::BuildConeGeometry()
{
	GeometryGenerator geoGen;
	//GeometryGenerator::MeshData box = geoGen.CreateQuad(0.0f, 0.0f, 20.0f, 10.0f, 10.0f);
	GeometryGenerator::MeshData box = geoGen.CreateCone(1, 1, 15, 15);
	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = 0;
	boxSubmesh.BaseVertexLocation = 0;


	std::vector<Vertex> vertices(box.Vertices.size());

	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices = box.GetIndices16();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "ConeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["Cone"] = boxSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void Game::BuildCylinderGeometry()
{
	GeometryGenerator geoGen;
	//GeometryGenerator::MeshData box = geoGen.CreateQuad(0.0f, 0.0f, 20.0f, 10.0f, 10.0f);
	GeometryGenerator::MeshData box = geoGen.CreateCylinder(1, 1,1, 15, 15);
	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = 0;
	boxSubmesh.BaseVertexLocation = 0;


	std::vector<Vertex> vertices(box.Vertices.size());

	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices = box.GetIndices16();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "CylinderGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["Cylinder"] = boxSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void Game::BuildQubeGeometry()
{
	GeometryGenerator geoGen;
	//GeometryGenerator::MeshData box = geoGen.CreateQuad(0.0f, 0.0f, 20.0f, 10.0f, 10.0f);
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.5, 0.5, 0.5, 0.5);
	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = 0;
	boxSubmesh.BaseVertexLocation = 0;


	std::vector<Vertex> vertices(box.Vertices.size());

	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices = box.GetIndices16();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "QubeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["Qube"] = boxSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void Game::BuildSphereGeometry()
{
	GeometryGenerator geoGen;
	//GeometryGenerator::MeshData box = geoGen.CreateQuad(0.0f, 0.0f, 20.0f, 10.0f, 10.0f);
	GeometryGenerator::MeshData box = geoGen.CreateSphere(1, 15, 15);
	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = 0;
	boxSubmesh.BaseVertexLocation = 0;


	std::vector<Vertex> vertices(box.Vertices.size());

	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices = box.GetIndices16();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "SphereGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["Sphere"] = boxSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void Game::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));


	// step 8
	// PSO for alpha tested objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
	alphaTestedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["alphaTestedPS"]->GetBufferPointer()),
		mShaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&mPSOs["alphaTested"])));
}

void Game::ClearFrameResources()
{
	mFrameResources.clear();
}

//void Game::BuildFrameResources()
//{
//	for (int i = 0; i < gNumFrameResources; ++i)
//	{
//		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
//			1, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
//	}
//}
void Game::BuildFrameResources(int numRenderItems)
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)numRenderItems, (UINT)mMaterials.size()));
	}
}
//step13
void Game::BuildMaterials()
{
	int index = 0;

	auto Eagle = std::make_unique<Material>();
	Eagle->Name = "Eagle";
	Eagle->MatCBIndex = index;
	Eagle->DiffuseSrvHeapIndex = index++;
	Eagle->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Eagle->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Eagle->Roughness = 0.2f;

	mMaterials["Eagle"] = std::move(Eagle);

	auto Raptor = std::make_unique<Material>();
	Raptor->Name = "Raptor";
	Raptor->MatCBIndex = index;
	Raptor->DiffuseSrvHeapIndex = index++;
	Raptor->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Raptor->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Raptor->Roughness = 0.2f;

	mMaterials["Raptor"] = std::move(Raptor);

	auto Desert = std::make_unique<Material>();
	Desert->Name = "Desert";
	Desert->MatCBIndex = index;
	Desert->DiffuseSrvHeapIndex = index++;
	Desert->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Desert->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Desert->Roughness = 0.2f;

	mMaterials["Desert"] = std::move(Desert);


	// title screen material
	auto TitleBackground = std::make_unique<Material>();
	TitleBackground->Name = "TitleBackground";
	TitleBackground->MatCBIndex = index;
	TitleBackground->DiffuseSrvHeapIndex = index++;
	TitleBackground->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	TitleBackground->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	TitleBackground->Roughness = 0.2f;

	mMaterials["TitleBackground"] = std::move(TitleBackground);

	// titlePrompt material
	auto TitleScreenPrompt = std::make_unique<Material>();
	TitleScreenPrompt->Name = "TitlePrompt";
	TitleScreenPrompt->MatCBIndex = index;
	TitleScreenPrompt->DiffuseSrvHeapIndex = index++;
	TitleScreenPrompt->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	TitleScreenPrompt->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	TitleScreenPrompt->Roughness = 0.2f;

	mMaterials["TitlePrompt"] = std::move(TitleScreenPrompt);

	// menu play prompt material
	auto MenuPlay = std::make_unique<Material>();
	MenuPlay->Name = "MenuPlay";
	MenuPlay->MatCBIndex = index;
	MenuPlay->DiffuseSrvHeapIndex = index++;
	MenuPlay->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	MenuPlay->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	MenuPlay->Roughness = 0.2f;

	mMaterials["MenuPlay"] = std::move(MenuPlay);

	//// menu quit material
	auto MenuQuit = std::make_unique<Material>();
	MenuQuit->Name = "MenuQuit";
	MenuQuit->MatCBIndex = index;
	MenuQuit->DiffuseSrvHeapIndex = index++;
	MenuQuit->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	MenuQuit->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	MenuQuit->Roughness = 0.2f;

	mMaterials["MenuQuit"] = std::move(MenuQuit);

	// title prompt material
	auto MenuArrow = std::make_unique<Material>();
	MenuArrow->Name = "Airplane";
	MenuArrow->MatCBIndex = index;
	MenuArrow->DiffuseSrvHeapIndex = index++;
	MenuArrow->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	MenuArrow->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	MenuArrow->Roughness = 0.2f;

	mMaterials["Airplane"] = std::move(MenuArrow);

	// Pause prompt material
	auto PausePrompt = std::make_unique<Material>();
	PausePrompt->Name = "PausePrompt";
	PausePrompt->MatCBIndex = index;
	PausePrompt->DiffuseSrvHeapIndex = index++;
	PausePrompt->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	PausePrompt->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	PausePrompt->Roughness = 0.2f;

	mMaterials["PausePrompt"] = std::move(PausePrompt);

	//SilverTex material
	auto SilverTex = std::make_unique<Material>();
	SilverTex->Name = "Silver";
	SilverTex->MatCBIndex = index;
	SilverTex->DiffuseSrvHeapIndex = index++;
	SilverTex->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SilverTex->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	SilverTex->Roughness = 0.2f;

	mMaterials["Silver"] = std::move(SilverTex);

	// Blue material
	auto Blue = std::make_unique<Material>();
	Blue->Name = "Blue";
	Blue->MatCBIndex = index;
	Blue->DiffuseSrvHeapIndex = index++;
	Blue->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Blue->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Blue->Roughness = 0.2f;

	mMaterials["Blue"] = std::move(Blue);

	// title prompt material
	auto Sky = std::make_unique<Material>();
	Sky->Name = "Sky";
	Sky->MatCBIndex = index;
	Sky->DiffuseSrvHeapIndex = index++;
	Sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Sky->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Sky->Roughness = 0.2f;

	mMaterials["Sky"] = std::move(Sky);


	auto Planet = std::make_unique<Material>();
	Planet->Name = "Planet";
	Planet->MatCBIndex = index;
	Planet->DiffuseSrvHeapIndex = index++;
	Planet->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Planet->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	Planet->Roughness = 0.9f;

	mMaterials["Planet"] = std::move(Planet);

}

void Game::BuildRenderItems()
{
}

void Game::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		//step18
		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

		//step19
		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

//step21
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Game::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegisters
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}