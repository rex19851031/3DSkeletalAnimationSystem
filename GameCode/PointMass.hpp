#pragma once
#include <vector>

#include "Engine\Math\Vec3.hpp"
#include "Engine\Core\VertexStruct.hpp"

#include "GameCode\Link.hpp"

#ifndef POINTMASS_HPP 
#define POINTMASS_HPP

namespace Henry
{ 

class PointMass
{
public:
	PointMass(Vec3f position, RGBA _color);
	~PointMass();
	void updatePhysics(float deltaSeconds);
	void applyForce(Vec3f force);
	void solveConstraints();
	void attachTo(PointMass* P, float restingDist, float stiff, bool drawLink);
	void attachTo(PointMass* P, float restingDist, float stiff, float tearSensitivity);
	void attachTo(PointMass* P, float restingDist, float stiff, float tearSensitivity, bool drawLink);
	void removeLink(Link* lnk);

public:
	Vec3f lastPosition;
	Vec3f currentPosition;
	Vec3f acceleration;
	float mass;
	float damping;
	std::vector<Link*> links;
	RGBA color;

private:
	int bounceCounter;
};

}

#endif