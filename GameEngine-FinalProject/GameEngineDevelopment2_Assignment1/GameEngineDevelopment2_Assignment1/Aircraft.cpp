/*
-------------------------------------------------------------------------
//Final Assignment
// author : Negar Saeidi - 101261396  and  NeginSaeidi - 101261395
// Aircraft.cpp, builds an aircraft render item
-------------------------------------------------------------------------*/
#include "Aircraft.hpp"
#include "Game.hpp"

/// <summary>
   /// Aircraft constructor, sets the sprite based on the type of the aircraft
 /// </summary>
Aircraft::Aircraft(BodyType type, AircraftType aircraftType, State* state)
	: Entity(state)
	, mType(type)
	,mAircraftType(aircraftType)
{
	switch (mType)
	{
	case Aircraft::Body:
		mSprite = "Silver";
		mGeometry = "CylinderGeo";
		mShape = "Cylinder";
		break;
	case Aircraft::Head:
		mSprite = "Raptor";
		mGeometry = "ConeGeo";
		mShape = "Cone";
		break;
	case Aircraft::Tale:
		mSprite = "Blue";
		mGeometry = "ConeGeo";
		mShape = "Cone";
		break;
	case Aircraft::LeftWing:
		mSprite = "Raptor";
		mGeometry = "QubeGeo";
		mShape = "Qube";
		break;
	case Aircraft::RightWing:
		mSprite = "Raptor";
		mGeometry = "QubeGeo";
		mShape = "Qube";
		break;
	default:
		break;
	}
}

void Aircraft::drawCurrent() const
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
 * Builds an aircraft render item and adds it to the mAllRenderItems in the game class
 *
 * @return void
 */
void Aircraft::buildCurrent()
{
	
	auto render = std::make_unique<RenderItem>();
	renderer = render.get();

	renderer->World = getTransform();
	renderer->ObjCBIndex = mState->mAllRitems.size();
	renderer->Mat = mState->getContext()->game->getMaterilas()[mSprite].get();
	renderer->Geo = mState->getContext()->game->getGeometries()[mGeometry].get();
	renderer->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	renderer->IndexCount = renderer->Geo->DrawArgs[mShape].IndexCount;
	renderer->StartIndexLocation = renderer->Geo->DrawArgs[mShape].StartIndexLocation;
	renderer->BaseVertexLocation = renderer->Geo->DrawArgs[mShape].BaseVertexLocation;
	//mAircraftRitem = render.get();

	mState->mAllRitems.push_back(std::move(render));


}

unsigned int Aircraft::getCategory() const
{
	switch (mAircraftType)
	{
	case Eagle:
		return Category::PlayerAircraft;

	default:
		return Category::EnemyAircraft;
	}
}
/**
 * Creates all parts of aircraft. Each part is an aircraft with different shapes 
 * And they are attached to this aircraft
 *
 *
 * @return void
 */
void Aircraft::CreateAirplane()
{
	std::unique_ptr<Aircraft> aircraft1(new Aircraft(Aircraft::Head, Aircraft::Raptor, mState));
	mAircraft[0] = aircraft1.get();
	mAircraft[0]->setPosition(0, 0.0, 0.6);
	mAircraft[0]->setScale(1, 1, 1);
	mAircraft[0]->setWorldRotation(0 / 2, 0, 0);
	//	mPlayerAircraft[1]->setVelocity(1, 0.0001, 0);
	this->attachChild(std::move(aircraft1));

	std::unique_ptr<Aircraft> aircraft2(new Aircraft(Aircraft::Tale, Aircraft::Raptor, mState));
	mAircraft[1] = aircraft2.get();
	mAircraft[1]->setPosition(0, 0, -0.15);
	mAircraft[1]->setScale(1, 1, 1);
	mAircraft[1]->setWorldRotation(0 / 2, 0, 0);
	//	mPlayerAircraft[2]->setVelocity(1, 0.0001, 0);
	this->attachChild(std::move(aircraft2));

	std::unique_ptr<Aircraft> aircraft3(new Aircraft(Aircraft::LeftWing, Aircraft::Raptor, mState));
	mAircraft[2] = aircraft3.get();
	mAircraft[2]->setPosition(-0.2, 0.0, 0.0);
	mAircraft[2]->setScale(1, 1, 1);
	mAircraft[2]->setWorldRotation(0, 0, 0);
	//	mPlayerAircraft[3]->setVelocity(1, 0.0001, 0);
	this->attachChild(std::move(aircraft3));

	std::unique_ptr<Aircraft> aircraft4(new Aircraft(Aircraft::LeftWing, Aircraft::Raptor, mState));
	mAircraft[3] = aircraft4.get();
	mAircraft[3]->setPosition(0.2, 0.0, 0.0);
	mAircraft[3]->setScale(1, 1, 1);
	mAircraft[3]->setWorldRotation(0, 0, 0);
	//	mPlayerAircraft[4]->setVelocity(1, 0.0001, 0);
	this->attachChild(std::move(aircraft4));
}

