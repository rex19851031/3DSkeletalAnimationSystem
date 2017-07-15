#include "Link.hpp"
#include "GameCode\PointMass.hpp"

namespace Henry
{ 

Link::Link(PointMass* which1, PointMass* which2, float restingDist, float stiff, float _tearSensitivity, bool drawMe)
{
	p1 = which1; // when you set one object to another, it's pretty much a reference. 
	p2 = which2; // Anything that'll happen to p1 or p2 in here will happen to the paticles in our ArrayList

	restingDistance = restingDist;
	stiffness = stiff;
	drawThis = drawMe;

	tearSensitivity = _tearSensitivity;
}


Link::~Link()
{
}


void Link::Solve()
{
	// calculate the distance between the two PointMasss
	Vec3f positionDifference = p1->currentPosition - p2->currentPosition;
	float distance = positionDifference.length();

	// find the difference, or the ratio of how far along the restingDistance the actual distance is.
	float difference = (restingDistance - distance) / distance;

	// if the distance is more than TearSensitivity, the cloth tears
// 	if (distance > tearSensitivity)
// 		p1->removeLink(this);

	// Inverse the mass quantities
	float im1 = 1.0f / p1->mass;
	float im2 = 1.0f / p2->mass;
	float scalarP1 = (im1 / (im1 + im2)) * stiffness;
	float scalarP2 = stiffness - scalarP1;

	// Push/pull based on mass
	// heavier objects will be pushed/pulled less than attached light objects
	p1->currentPosition += positionDifference * scalarP1 * difference;
	p2->currentPosition += positionDifference * scalarP2 * difference;
}


}