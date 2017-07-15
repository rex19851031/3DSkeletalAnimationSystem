#pragma once

#ifndef LINK_HPP 
#define LINK_HPP

namespace Henry
{ 

class PointMass;

class Link
{
public:
	Link(PointMass* which1, PointMass* which2, float restingDist, float stiff, float _tearSensitivity, bool drawMe);
	~Link();
	void Solve();

public:
	bool drawThis;
	float restingDistance;
	float stiffness;
	float tearSensitivity;
	PointMass* p1;
	PointMass* p2;
};

}

#endif