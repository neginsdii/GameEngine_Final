/*
-------------------------------------------------------------------------
//Final Assignment
// author : Negar Saeidi - 101261396  and  NeginSaeidi - 101261395
// SpriteNode.cpp
// Builds a sprite render item
-------------------------------------------------------------------------*/
#include "SpriteNode.hpp"
#include "Game.hpp"
SpriteNode::SpriteNode(State* state, std::string spriteName)
	: Entity(state)
	,mSprite(spriteName)
{
}

void SpriteNode::drawCurrent() const
{
	
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mState->getContext()->game->GetFrameResource()->ObjectCB->Resource();
	auto matCB = mState->getContext()->game->GetFrameResource()->MaterialCB->Resource();

	if (renderer != nullptr)
	{
		mState->getContext()->game->getcommandList()->IASetVertexBuffers(0, 1, &renderer->Geo->VertexBufferView());
		mState->getContext()->game->getcommandList()->IASetIndexBuffer(&renderer->Geo->IndexBufferView());
		mState->getContext()->game->getcommandList()->IASetPrimitiveTopology(renderer->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mState->getContext()->game->getSrvDesvriptionHeap()->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(renderer->Mat->DiffuseSrvHeapIndex, mState->getContext()->game->GetCbvSrvDescriptorSize());

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + (UINT64)renderer->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + (UINT64)renderer->Mat->MatCBIndex * matCBByteSize;

		mState->getContext()->game->getcommandList()->SetGraphicsRootDescriptorTable(0, tex);
		mState->getContext()->game->getcommandList()->SetGraphicsRootConstantBufferView(1, objCBAddress);
		mState->getContext()->game->getcommandList()->SetGraphicsRootConstantBufferView(3, matCBAddress);

		mState->getContext()->game->getcommandList()->DrawIndexedInstanced(renderer->IndexCount, 1, renderer->StartIndexLocation, renderer->BaseVertexLocation, 0);
	}
}
/**
 * Builds a sprite render item and adds it to the mAllRenderItems in the game class
 * 
 * @return void
 */
void SpriteNode::buildCurrent()
{
	auto render = std::make_unique<RenderItem>();
	renderer = render.get();
	renderer->World = getTransform();
	XMStoreFloat4x4(&renderer->TexTransform,XMMatrixScaling(1,1,1));
	renderer->ObjCBIndex = mState->mAllRitems.size();
	renderer->Mat = mState->getContext()->game->getMaterilas()[mSprite].get();
	renderer->Geo = mState->getContext()->game->getGeometries()["boxGeo"].get();
	renderer->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	renderer->IndexCount = renderer->Geo->DrawArgs["box"].IndexCount;
	renderer->StartIndexLocation = renderer->Geo->DrawArgs["box"].StartIndexLocation;
	renderer->BaseVertexLocation = renderer->Geo->DrawArgs["box"].BaseVertexLocation;
	mState->mAllRitems.push_back(std::move(render));

}
