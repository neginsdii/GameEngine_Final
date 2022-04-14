/*
-------------------------------------------------------------------------
//Final Assignment
// author : Negar Saeidi - 101261396  and  NeginSaeidi - 101261395
//MenuState.cpp
//Creates the Menu screen of the game
*/
#include "MenuState.hpp"
#include "Game.hpp"
/**
 * Constructor(Initializes the variables of the class
 *@param StateStack* stack, Context* context
 * @return None
 */
MenuState::MenuState(StateStack* stack, Context* context)
	: State(stack, context)
	, mOptionIndex(0)
	, mBackgroundSprite(nullptr)
	, mPlay(nullptr)
	, mQuit(nullptr)
	/*, mArrow(nullptr)*/
{
	
	CreateScene();
	
	mArrow->setPosition(-0.9, -6, -3.4);
}
/**
 * Draws the menu screen
 *@param None
 * @return void
 */
void MenuState::draw()
{
	mSceneGraph->draw();
}
/**
 * updates the scene accordingly
 *@param const GameTimer& gt
 * @return bool
 */
bool MenuState::update(const GameTimer& gt)
{
	mSceneGraph->update(gt);



	return true;
	
}
/**
 * handles any input by player(if the first button is selected -> go to gameState, if the second one is selected quit the app)
 *@param WPARAM btnState
 * @return bool
 */

bool MenuState::handleEvent(WPARAM btnState)
{
	if (btnState == VK_RETURN)
	{
		if (mOptionIndex == Play)
		{
			requestStackPop();
			requestStackPush(States::Game);
		}
		else if (mOptionIndex == Exit)
		{
			// The exit option was chosen, by removing itself, the stack will be empty, and the game will know it is time to close.
			requestStackPop();
		}
	}
	else if (btnState == VK_UP)
	{
		// Decrement and wrap-around
		if (mOptionIndex > 0)
			mOptionIndex--;
		else
			mOptionIndex = mOptions.size() - 1;

		updateOptionText();
	}
	else if (btnState == VK_DOWN)
	{
		// Increment and wrap-around
		if (mOptionIndex < mOptions.size() - 1)
			mOptionIndex++;
		else
			mOptionIndex = 0;

		updateOptionText();
	}
	else
	{
		return false;
	}

	return true;
}

void MenuState::updateOptionText()
{
	if (mOptions.empty())
		return;
	if(mOptionIndex==0)
	{ 
		mArrow->setPosition(-0.9, -6, -3.4);

	}
	else
	{
		mArrow->setPosition(-0.9, -6, -1.8);
	}

}
/**
 *Creates the Menu panel( background, title, buttons, selector)
 *@param None
 * @return void
 */
void MenuState::CreateScene()
{
	getContext()->game->BuildMaterials();

	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(this));
	backgroundSprite->setPosition(0, -5.5, -3.1);
	backgroundSprite->setScale(12, 4, 8.5);
	backgroundSprite->setWorldRotation(XMConvertToRadians(150), 0, 0);
	backgroundSprite->setVelocity(0, 0, 0);
	mSceneGraph->attachChild(std::move(backgroundSprite));

	std::unique_ptr<SpriteNode> PlaySprite(new SpriteNode(this, "MenuPlay"));
	mPlay = PlaySprite.get();
	mPlay->setPosition(0, -6, -3.4);
	mPlay->setScale(6 ,1, 5);
	mPlay->setWorldRotation(XMConvertToRadians(150), 0, 0);
	mPlay->setVelocity(0, 0, 0);
    mSceneGraph->attachChild(std::move(PlaySprite));
    mOptions.push_back(mPlay);

	std::unique_ptr<SpriteNode> QuitSprite(new SpriteNode(this, "MenuQuit"));
	mQuit = QuitSprite.get();
	mQuit->setPosition(0, -6, -0.4);
	mQuit->setScale(6, 1, 5);
	mQuit->setWorldRotation(XMConvertToRadians(150), 0, 0);

	mQuit->setVelocity(0, 0, 0);
	mSceneGraph->attachChild(std::move(QuitSprite));
	mOptions.push_back(mQuit);

	std::unique_ptr<SpriteNode> menuArrowSprite(new SpriteNode(this, "Airplane"));
	mArrow= menuArrowSprite.get();
	mArrow->setPosition(0, -6, -3.4);
	mArrow->setScale(0.5f, 1.0f, 0.5f);
	mArrow->setWorldRotation(XMConvertToRadians(150), 0, 0);

	mArrow->setVelocity(0, 0, 0);
	mSceneGraph->attachChild(std::move(menuArrowSprite));

	mSceneGraph->build();
	getContext()->game->ClearFrameResources();
	getContext()->game->BuildFrameResources(mAllRitems.size());

	
	


	



}
