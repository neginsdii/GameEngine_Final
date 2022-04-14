/*
-------------------------------------------------------------------------
//Final Assignment
// author : Negar Saeidi - 101261396  and  NeginSaeidi - 101261395
//State.cpp
*/
#include "State.hpp"
#include "StateStack.hpp"
/**
 * Constructor(Initializes the variables of the class)
 *@param Player* player, Game* game
 * @return None
 */

State::Context::Context( Player* player, Game* game)
	: player(player),
	game(game)
{
}
/**
 *Second Constructor(Initializes the variables of the class)
 *@paramStateStack* stack, Context* context
 * @return None
 */
State::State(StateStack* stack, Context* context)
	: mStack(stack)
	, mContext(context)
	, mSceneGraph(new SceneNode(this))
{
}

State::~State()
{
}
/**
 * pushes the new state to the stack
 *@param States::ID stateID
 * @return void
 */
void State::requestStackPush(States::ID stateID)
{
	mStack->pushState(stateID);
}
/**
 * pops the last State from the stack
 *@param none
 * @return void
 */
void State::requestStackPop()
{
	mStack->popState();
}
/**
 * clears the state stack
 *@param none
 * @return void
 */
void State::requestStateClear()
{
	mStack->clearStates();
}
/**
 * returns the context
 *@param none
 * @return State::Context*
 */
State::Context* State::getContext() const
{
	return mContext;
}
