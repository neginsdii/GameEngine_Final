#pragma once
#include "Entity.hpp"


class SpriteNode : public Entity
{
public:
	SpriteNode(State* state, std::string spriteName = "Sky");
	

private:

	virtual void       drawCurrent() const;
	virtual void       buildCurrent();
	std::string        mSprite;
};