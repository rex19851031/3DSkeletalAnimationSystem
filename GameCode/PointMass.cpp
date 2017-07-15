#include "PointMass.hpp"


namespace Henry
{

PointMass::PointMass(Vec3f position, RGBA _color) : color(_color)
{
	currentPosition = position; 
	lastPosition = position; 
	acceleration = Vec3f(0.0f, 0.0f, 0.0f);
	bounceCounter = 0;
}


PointMass::~PointMass()
{
}


void PointMass::updatePhysics(float deltaSeconds)
{
	Vec3f gravity = Vec3f(0.0f, 0.0f, -980.f);
	this->applyForce(gravity * mass);

	Vec3f velocity = currentPosition - lastPosition;
	velocity *= 0.99f;
	float timeSquare = deltaSeconds * deltaSeconds;
	Vec3f nextPosition = currentPosition + velocity + acceleration * timeSquare * 0.5f;

	lastPosition = currentPosition;
	currentPosition = nextPosition;
	acceleration = Vec3f(0.0f, 0.0f, 0.0f);
}


void PointMass::applyForce(Vec3f force)
{
	acceleration.x += force.x / mass;
	acceleration.y += force.y / mass;
	acceleration.z += force.z / mass;
}


void PointMass::solveConstraints()
{
	for (size_t index = 0; index < links.size(); ++index)
	{
		Link* currentLink = links[index];
		currentLink->Solve();
	}

	//boundary constraints, currently hard coded z = 0 as boundary
	if (currentPosition.z < 1.0f && bounceCounter <= 10)
	{
		currentPosition.z = 2 * 1.0f - currentPosition.z;
		++bounceCounter;
	}

	if (bounceCounter >= 10)
	{
		currentPosition.z = 1.0f;
	}
}


void PointMass::attachTo(PointMass* P, float restingDist, float stiff, bool drawLink)
{
	attachTo(P, restingDist, stiff, 30, drawLink);
}


void PointMass::attachTo(PointMass* P, float restingDist, float stiff, float tearSensitivity)
{
	attachTo(P, restingDist, stiff, tearSensitivity, true);
}


void PointMass::attachTo(PointMass* P, float restingDist, float stiff, float tearSensitivity, bool drawLink) {
	Link* lnk = new Link(this, P, restingDist, stiff, tearSensitivity, drawLink);
	links.push_back(lnk);
}


void PointMass::removeLink(Link* lnk)
{
	std::vector<Link*>::iterator it = links.begin();
	while (it != links.end())
	{
		Link* l = *it;
		if (l == lnk)
		{
			it = links.erase(it);
			break;
		}
		else
			++it;
	}
}

}