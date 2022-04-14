#pragma once
//#pragma region step 7
#include "State.hpp"

#include "../../Common/d3dApp.h"


class TitleState : public State
{
public:
	TitleState(StateStack* stack, Context* context);

	virtual void		draw();
	virtual bool		update(const GameTimer& gt);
	virtual bool		handleEvent(WPARAM btnState);


private:
	virtual void        CreateScene() override;
};
//#pragma endregion
