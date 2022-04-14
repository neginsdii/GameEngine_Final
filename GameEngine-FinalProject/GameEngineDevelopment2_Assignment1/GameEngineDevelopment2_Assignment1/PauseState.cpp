/*
-------------------------------------------------------------------------
//Final Assignment
// author : Negar Saeidi - 101261396  and  NeginSaeidi - 101261395
//PauseState.cpp
//Creates the pause screen of the game
*/
#pragma region step 3
#include "PauseState.hpp"
#include "Game.hpp"
#include "GameState.hpp"
#include "State.hpp"
/**
 * Constructor(Initializes the variables of the class
 *@param StateStack* stack, Context* context
 * @return None
 */
PauseState::PauseState(StateStack* stack, Context* context)
	:State(stack, context)
{
	CreateScene();
}
/**
 * Draws the pause screen
 *@param None
 * @return void
 */
void PauseState::draw()
{
	((GameState*)((*mStack->GetStateStack())[0].get()))->mPauseSceneGraph->draw();
}
/**
 * updates the scene accordingly
 *@param const GameTimer& gt
 * @return bool
 */
bool PauseState::update(const GameTimer& gt)
{
	((GameState*)((*mStack->GetStateStack())[0].get()))->mPauseSceneGraph->update(gt);
	return false;
}
/**
 * handles any input by player(if Q pressed -> Resume the game)
 *@param WPARAM btnState
 * @return bool
 */

bool PauseState::handleEvent(WPARAM btnState)
{
	if (btnState == 'Q')
	{
		requestStackPop();
	}
	else if (btnState == VK_BACK)
	{
		requestStateClear();
		requestStackPush(States::Menu);
	}
	return false;
}

void PauseState::CreateScene()
{

}
#pragma endregion
