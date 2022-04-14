#ifndef BOOK_STATE_HPP
#define BOOK_STATE_HPP

#include "StateIdentifier.hpp"

#include "../../Common/d3dApp.h"

#include "SpriteNode.hpp"
#include <memory>



class StateStack;
class Player;
class Game;
class State
{
public:
	typedef std::unique_ptr<State> Ptr;

	struct Context
	{
		Context(  Player* player, Game* game);

		Game* game;
		Player* player;
	};


public:
	State(StateStack* stack, Context* context);
	virtual				~State();

	virtual void		draw() = 0;
	virtual bool		update(const GameTimer& gt) = 0;
	virtual bool		handleEvent(WPARAM btnState) = 0;
	virtual void        CreateScene() = 0;

	Context* getContext() const;
protected:
	void				requestStackPush(States::ID stateID);
	void				requestStackPop();
	void				requestStateClear();


	SceneNode*          mSceneGraph;
	StateStack* mStack;
public:
	std::vector < std::unique_ptr<RenderItem>> mAllRitems;
private:

	Context*				mContext;

};

#endif // BOOK_STATE_HPP
