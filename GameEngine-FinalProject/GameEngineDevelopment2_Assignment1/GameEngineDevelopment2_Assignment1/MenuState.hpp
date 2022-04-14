#ifndef BOOK_MENUSTATE_HPP
#define BOOK_MENUSTATE_HPP

#include "State.hpp"
#include "SpriteNode.hpp"
#include  "../../Common/d3dApp.h" 



class MenuState : public State
{
public:
	MenuState(StateStack* stack, Context* context);

	virtual void			draw();
	virtual bool			update(const GameTimer& gt);
	virtual bool			handleEvent(WPARAM btnState);

	void					updateOptionText();


private:
	enum OptionNames
	{
		Play,
		Exit,
	};


private:
	SpriteNode* mBackgroundSprite;
	SpriteNode* mPlay;
	SpriteNode* mQuit;
	SpriteNode* mArrow;

	std::vector<SpriteNode*> mOptions;
	std::size_t mOptionIndex;

	virtual void CreateScene() override;
};

#endif // BOOK_MENUSTATE_HPP
