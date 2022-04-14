/*
-------------------------------------------------------------------------
//Final Assignment 
// author : Negar Saeidi - 101261396  and  NeginSaeidi - 101261395
// World.cpp
-------------------------------------------------------------------------*/
#define NOMINMAX

#include "World.hpp"
#include "State.hpp"
#include <stdlib.h>
#include <time.h>
World::World(State* state)
	: mState(state)
	, mSceneGraph(new SceneNode(state))
	, mPlayerAircraft(nullptr)
	, mSceneLayers()
	, mWorldBounds(-5.9f, 5.9f, -2.1f, 10.f)
	, mSpawnPostion(0.0f, 0.0f)
	, mScrollSpeed(-1.f)
{
	srand((unsigned)time(NULL));
}
/**
 * Updates the scene graph, calls the update function of eahc scene node
 *@param const GameTimer& gt
 * @return void
 */
void World::update(const GameTimer& gt)
{
	for (int i = 0; i < 15; i++)
	{
		for (int j = 0; j < 15; j++)
		{
			mBackground[i][j]->move(0.f, 0, mScrollSpeed * gt.DeltaTime());
		}
	}

	for (int i = 0; i < 15; i++)
	{
		
			Planets[i]->move(0.f, 0, (mScrollSpeed - 0.5) * gt.DeltaTime());
		
	}

	mPlayerAircraft->setVelocity(0.0f, 0.0f, 0.0f);
	adaptPlayerPosition();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph->onCommand(mCommandQueue.pop(), gt);

	//adaptPlayerVelocity();

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph->update(gt);

}

void World::draw()
{
	mSceneGraph->draw();
}
/**
 * builds the game scene, creates scene nodes and adds the m to the scene graph
 *@param void
 * @return void
 */
void World::buildScene()
{
	mSceneGraph->ClearChildren();

	std::unique_ptr<Aircraft> aircraft(new Aircraft(Aircraft::Body,Aircraft::Eagle, mState));
	mPlayerAircraft = aircraft.get();
	mPlayerAircraft->setPosition(0, 0.0, 0.0);
	mPlayerAircraft->setScale(0.15, 0.8, 0.15);
	mPlayerAircraft->setWorldRotation(XM_PI / 2, 0, 0);
	mPlayerAircraft->setVelocity(1, 0.0001, 0);
	mSceneGraph->attachChild(std::move(aircraft));
	mPlayerAircraft->CreateAirplane();


	std::unique_ptr<Aircraft> enemy1(new Aircraft(Aircraft::Body, Aircraft::Raptor, mState));
	auto raptor = enemy1.get();
	raptor->setPosition(-1, 0.0, -1.0);
	raptor->setScale(1, 1, 1);
	raptor->setWorldRotation(0, 0, 0);
	mPlayerAircraft->attachChild(std::move(enemy1));
	raptor->CreateAirplane();


	std::unique_ptr<Aircraft> enemy2(new Aircraft(Aircraft::Body, Aircraft::Raptor, mState));
	auto raptor1 = enemy2.get();
	raptor1->setPosition(1, 0.0, -1.0);
	raptor1->setScale(1, 1, 1);
	raptor1->setWorldRotation(0, 0, 0);
	mPlayerAircraft->attachChild(std::move(enemy2));
	raptor1->CreateAirplane();

	for (int i = 0; i < 15; i++)
	{
		for (int j = 0; j < 15; j++)
		{

			std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(mState));
			mBackground[i][j] = backgroundSprite.get();
			mBackground[i][j]->setPosition(-15+j*15, 0, -10+i * 10);
			mBackground[i][j]->setScale(15, 1, 10);
			mSceneGraph->attachChild(std::move(backgroundSprite));
		}
	}
	
	for (int i = 0; i < 15; i++)
	{
			float x = rand() % 15 - rand() % 10;
			std::unique_ptr<Planet> planet(new Planet(mState));
			Planets[i] = planet.get();
			Planets[i]->setPosition( x, 1, -10 + i * 10);
			Planets[i]->setScale(0.5, 0.5, 0.5);
			mSceneGraph->attachChild(std::move(planet));
		
	}

	mSceneGraph->build();

}

CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}
World::~World()
{
}
#pragma region step 4

void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border

	XMFLOAT3 position = mPlayerAircraft->getWorldPosition();
	position.x = std::max(position.x, mWorldBounds.x);
	position.x = std::min(position.x, mWorldBounds.y);
	position.y = std::max(position.y, mWorldBounds.z);
	position.y = std::min(position.y, mWorldBounds.w);
	mPlayerAircraft->setPosition(position.x, position.y, position.z);
	

}

void World::adaptPlayerVelocity()
{
	XMFLOAT3 velocity = mPlayerAircraft->getVelocity();

	// If moving diagonally, reduce velocity (to have always same velocity)
	if (velocity.x != 0.f && velocity.y != 0.f && velocity.z != 0)
		mPlayerAircraft->setVelocity(velocity.x / std::sqrt(2.f), velocity.y / std::sqrt(2.f), velocity.z / std::sqrt(2.f));

	// Add scrolling velocity
//	mPlayerAircraft->accelerate(0.f, mScrollSpeed);
}

#pragma endregion

