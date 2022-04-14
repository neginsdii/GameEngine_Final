#ifndef BOOK_GAMESTATE_HPP
#define BOOK_GAMESTATE_HPP

#include "State.hpp"
#include "World.hpp"
#include "Player.hpp"

#include "SpriteNode.hpp"


class GameState : public State
{
public:
	GameState(StateStack* stack, Context* context);
	void               ProcessInput();
	virtual void		draw();
	virtual bool			update(const GameTimer& gt);
	virtual bool			handleEvent(WPARAM btnState);
public: 
	SceneNode* mPauseSceneGraph;

private:
	World				mWorld;

	virtual void CreateScene() override;
};

#endif // BOOK_GAMESTATE_HPP