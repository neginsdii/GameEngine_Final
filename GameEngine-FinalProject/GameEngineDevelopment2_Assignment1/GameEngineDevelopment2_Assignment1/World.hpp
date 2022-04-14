#pragma once

#include "SceneNode.hpp"
#include "Aircraft.hpp"
#include "SpriteNode.hpp"
#include "Planet.h"
#include "Command.hpp"
#include "CommandQueue.hpp"

class State;
class World
{
public:
    World(State* state);
	void                                       update(const GameTimer& gt);
	void                                       draw();
	void                                       buildScene();
	CommandQueue&                              getCommandQueue();
	~World();
private:

	enum  Layer
	{
		Background,
		Air,
		LayerCount
	};

private:

	State*                                             mState;
	SceneNode*                                        mSceneGraph;
	std::array<SceneNode*, 3>                mSceneLayers;
	XMFLOAT4                                          mWorldBounds;
    XMFLOAT2                                          mSpawnPostion;
	float                                             mScrollSpeed;
	Aircraft*                                         mPlayerAircraft;
	SpriteNode*                                       mBackground[15][15];
	Planet*											Planets[15];

	CommandQueue                                      mCommandQueue;



#pragma region step 3
	void								adaptPlayerPosition();
	void								adaptPlayerVelocity();
#pragma endregion


};
