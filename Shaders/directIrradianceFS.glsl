//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 0) out vec3 color;

void main()
{
	vec2 RMuS = getIrradianceRMuSFromST();
	//Approximate sun contribution taking in account Radiative view factor, simplified formula was provided by Bruneton2017 implementation
	//alpha ~ cos(pi/2 - alpha)
	if (RMuS.y < -sunAngularRadius)
		color = vec3(0.0f);
	else if (RMuS.y > sunAngularRadius)
		color = solarIrradiance * getTransmittanceToTopAtmosphereBoundary(RMuS.x, RMuS.y) * RMuS.y;
    else
		color = solarIrradiance * getTransmittanceToTopAtmosphereBoundary(RMuS.x, RMuS.y) * (RMuS.y + sunAngularRadius) * (RMuS.y + sunAngularRadius) / (4.0f * sunAngularRadius);
}