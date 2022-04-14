#pragma region step 2
#pragma once

#include "State.hpp"
#include "SpriteNode.hpp"
#include  "../../Common/d3dApp.h" 

class PauseState : public State
{
public:
	PauseState(StateStack* stack, Context* context);

	virtual void		draw();
	virtual bool		update(const GameTimer& gt);
	virtual bool		handleEvent(WPARAM btnState);


private:
	SpriteNode*			mBackgroundSprite;
	SpriteNode*         mPausedText;
	SpriteNode*         mInstructionText;

	virtual void CreateScene() override;
};

#pragma endregion