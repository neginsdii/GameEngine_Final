#pragma once
#include "World.hpp"
#include "Player.hpp"
#include "StateIdentifier.hpp"
#include "TitleState.hpp"
#include "GameState.hpp"
#include "MenuState.hpp"
#include "PauseState.hpp"
#include "StateStack.hpp"

class StateStack;
enum class RenderLayer : int
{
    Opaque = 0,
    AlphaTested,
    Count
};
class Game: public D3DApp
{
public:
    Game(HINSTANCE hInstance);
    Game(const Game& rhs) = delete;
    Game& operator=(const Game& rhs) = delete;
    ~Game();
    void OnKeyboardInput(WPARAM btnState);
    virtual bool Initialize()override;
    void ClearFrameResources();
    void BuildFrameResources(int numberOfRenderItems);
private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;
    void RegisterStates();
    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
  
  
   // void ProcessInput(WPARAM btnState);

    void UpdateCamera(const GameTimer& gt);
    void AnimateMaterials(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateMaterialCBs(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);

    void LoadTextures();
    void BuildRootSignature();
    void BuildDescriptorHeaps();
    void BuildShadersAndInputLayout();
    void BuildShapeGeometry();
    void BuildConeGeometry();
    void BuildCylinderGeometry();
    void BuildQubeGeometry();
    void BuildSphereGeometry();
    void BuildPSOs();
 
    void BuildRenderItems();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

   
private:

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    UINT mCbvSrvDescriptorSize = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    // List of all the render items.
    std::vector<std::unique_ptr<RenderItem>> mAllRitems;
    ComPtr<ID3D12PipelineState> mOpaquePSO = nullptr;
    // Render items divided by PSO.

    PassConstants mMainPassCB;

 /*   XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f * XM_PI;
    float mPhi = 0.2f * XM_PI;
    float mRadius = 15.0f;*/

    POINT mLastMousePos;
  //  World mWorld;
    Player mPlayer;
    StateStack mStateStack;
public:
    Camera mCamera;

    std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];
    std::vector<RenderItem*>& getRenderItems() { return *mRitemLayer; }
    std::unordered_map<std::string, std::unique_ptr<Material>>& getMaterilas() { return mMaterials; }
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& getGeometries() { return mGeometries; }
    ID3D12GraphicsCommandList* getcommandList() { return mCommandList.Get(); }
    ID3D12DescriptorHeap* getSrvDesvriptionHeap() { return mSrvDescriptorHeap.Get(); }
    UINT GetCbvSrvDescriptorSize() { return mCbvSrvDescriptorSize; }
    FrameResource* GetFrameResource() { return mCurrFrameResource; }
    void BuildMaterials();

};
