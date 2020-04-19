#include "Atmosphere.h"

void Atmosphere::updateS(const float modifier)
{
	//Rotate sun direction by 2 degrees around x axis
	s = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, modifier * 0.05f).rotatedVector(s);
	s.normalize();
}

