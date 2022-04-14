#pragma once
#include "Entity.hpp"
#include <string>

class Planet : public Entity
{
public:


public:
	Planet(State* state);
	virtual unsigned int getCategory() const;

private:
	virtual void       drawCurrent() const;
	virtual void       buildCurrent();


private:

	std::string       mSprite;

	std::string       mGeometry;
	std::string       mShape;

};
