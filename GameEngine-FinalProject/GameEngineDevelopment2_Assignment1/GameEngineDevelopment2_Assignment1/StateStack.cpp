/*
-------------------------------------------------------------------------
//Final Assignment
// author : Negar Saeidi - 101261396  and  NeginSaeidi - 101261395
//StateStack.cpp

*/
#include "StateStack.hpp"
#include <cassert>
/**
 * Constructor(Initializes the variables of the class
 *@param StateStack* stack, Context* context
 * @return None
 */
StateStack::StateStack(State::Context context)
	: mStack()
	, mPendingList()
	, mContext(context)
	, mFactories()
{
}
/**
 *  Iterate from top to bottom, stop as soon as update() returns false
 *@param const GameTimer& gt
 * @return void
 */
void StateStack::update(const GameTimer& gt)
{

	for (auto itr = mStack.rbegin(); itr != mStack.rend(); ++itr)
	{
		if (!(*itr)->update(gt))
			break;
	}

	applyPendingChanges();
}
/**
 *Draw all active states from bottom to top
 *@param None
 * @return void
 */
void StateStack::draw()
{
	
	for (State::Ptr& state : mStack)
		state->draw();
}
/**
 *  Iterate from top to bottom, stop as soon as handleEvent() returns false
 *@param WPARAM btnState
 * @return bool
 */
void StateStack::handleEvent(WPARAM btnState)
{

	for (auto itr = mStack.rbegin(); itr != mStack.rend(); ++itr)
	{
		if (!(*itr)->handleEvent(btnState))
			break;
	}

	applyPendingChanges();
}
/**
 * pushes the next state to the pendingList
 *@param States::ID stateID
 * @return void
 */
void StateStack::pushState(States::ID stateID)
{
	mPendingList.push_back(PendingChange(Push, stateID));
}
/**
 * pops the last state from the pendingList
 *@param States::ID stateID
 * @return void
 */
void StateStack::popState()
{
	mPendingList.push_back(PendingChange(Pop));
}
/**
 * clears the pendingList
 *@param 
 * @return void
 */
void StateStack::clearStates()
{
	mPendingList.push_back(PendingChange(Clear));
}


/**
 * Checks to see if stack is empty
 *@param none
 * @return bool
 */
bool StateStack::isEmpty() const
{
	return mStack.empty();
}
/**
 * returns the stack
 *@param none
 * @return std::vector<State::Ptr>*
 */
std::vector<State::Ptr>* StateStack::GetStateStack()
{
	return &mStack;
}/**
 * creates the state
 *@param States::ID stateID
 * @return State::Ptr
 */

State::Ptr StateStack::createState(States::ID stateID)
{
	auto found = mFactories.find(stateID);
	assert(found != mFactories.end());

	return found->second();
}

/**
 * applies changes exsiting in the changelist
 *@param none
 * @return void
 */

void StateStack::applyPendingChanges()
{
	for (PendingChange change : mPendingList)
	{
		switch (change.action)
		{
		case Push:
			mStack.push_back(createState(change.stateID));
			break;

		case Pop:
			mStack.pop_back();
			break;

		case Clear:
			mStack.clear();
			break;
		}
	}

	mPendingList.clear();
}

StateStack::PendingChange::PendingChange(Action action, States::ID stateID)
	: action(action)
	, stateID(stateID)
{
}