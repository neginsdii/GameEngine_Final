#pragma once
#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "../../Common/Camera.h"
#include "FrameResource.h"
#include "Category.hpp"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")


struct RenderItem
{
	RenderItem() = default;

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

class State;
struct  Command;

class SceneNode
{
public :
	typedef std::unique_ptr<SceneNode> Ptr;
public:
	SceneNode(State* state);

	void                    attachChild(Ptr child);
	Ptr                     detachChild(const SceneNode& node);
	void                    ClearChildren();
	void                    update(const GameTimer& gt);
	void                    draw() const;
	void                    build();
	XMFLOAT3                getWorldPosition() const;
	void                    setPosition(float x, float y, float z);
	XMFLOAT3                getWorldRotation() const;
	void                    setWorldRotation(float x, float y, float z);
	XMFLOAT3                getWorldScale() const;
	void                    setScale(float x, float y, float z);

	XMFLOAT4X4              getWorldTransform() const;
	XMFLOAT4X4              getTransform() const;

	void                    onCommand(const Command& command, const GameTimer& gt);
	virtual unsigned int    getCategory() const;

	void                    move(float x, float y, float z);
	
private:
	virtual void            updateCurrent(const GameTimer& gt);
	void                    updateChildren(const GameTimer& gt);
	virtual void            drawCurrent() const;
	void                    drawChildren() const;
	virtual void            buildCurrent();
	void                    buildChildren();


protected:
	State*                   mState;
	RenderItem*             renderer;
private:
	XMFLOAT3                mWorldPosition;
	XMFLOAT3                mWorlRotation;
	XMFLOAT3                mWorldScale;
	std::vector<Ptr>        mChildren;
	SceneNode*              mParent;
};
