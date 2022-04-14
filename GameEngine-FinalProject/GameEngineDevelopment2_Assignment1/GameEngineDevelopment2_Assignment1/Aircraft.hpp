#pragma once
#include "Entity.hpp"
#include <string>

class Aircraft : public Entity
{
public:
	enum BodyType
	{
		Body,
		Head,
		Tale,
		LeftWing,
		RightWing

	};

	enum AircraftType
	{
		Eagle,
		Raptor
	};
	

public:
	Aircraft(BodyType type, AircraftType aircraftType, State* state);
	virtual unsigned int getCategory() const;
	void CreateAirplane();
private:
	virtual void       drawCurrent() const;
	virtual void       buildCurrent();


private:
	BodyType          mType;
	AircraftType	  mAircraftType;
	std::string       mSprite;
	
	std::string       mGeometry;
	std::string       mShape;
	
	Aircraft*		mAircraft[4];

};