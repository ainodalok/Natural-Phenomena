#version 460

#define T 0
#define E 1
#define S 2
#define DELTA_J 3
#define DELTA_E 4
#define DELTA_SR 5
#define DELTA_SM 6
#define COUNT 7

layout(location = 0) uniform float Rg;
layout(location = 1) uniform float Rt;
layout(location = 2) uniform int T_W;
layout(location = 3) uniform int T_H;

layout(binding = T) uniform sampler2D transmittanceTex;

vec2 getTransmittanceUvFromRMu(float r, float mu) 
{
	//Distance to horizon from top atmosphere boundary
	float H = sqrt(Rt * Rt - Rg * Rg);
	//Rho - distance to horizon from current r
	float rho = sqrt(r * r - Rg * Rg);/////////////////BUGGED when r == Rg true, workaround - precision qualifier for u and v
	//Both quadratic equation solution of sphere intersection and geometrically calculated distance from point at r to the atmosphere boundary along the ray
	float d = -r * mu + sqrt(r * r * mu * mu - r * r + Rt * Rt);
	//Minimum distance to atmosphere boundary
	float dMin = Rt - r;
	//Maximum distance to Rt (distance to horizon from current r + distance to top atmosphere boundary from horizon)
	float dMax = rho + H;
	
	//U_R and U_Mu
	float aU = rho / H;
	float aV = (d - dMin) / (dMax - dMin);

	//Adjusted uv so that transmittance values range would be mapped to a texture range [0.5/n; 1.0 - 0.5/n] instead of [0.0; 1.0], 
	//since fragment shader always looks up at values in the center of the texel, hence sampling outside gives extrapolated values
	precise float u = 0.5 / T_W + aU * (1.0 - 1.0 / T_W);
	precise float v = 0.5 / T_H + aV * (1.0 - 1.0 / T_H);
	return vec2(u, v);
	//return vec2(u, rho);
}

//Assumes ray does not intersect ground, otherwise undefined (probably returns clamped values)
vec3 getTransmittanceToTopAtmosphereBoundary(float r, float mu) 
{
	vec2 uv = getTransmittanceUvFromRMu(r, mu);
	return texture(transmittanceTex, uv).rgb;
	//return vec3(uv.y, 0.0, 0.0);
}

//Assumes point at radius r is inside atmosphere
vec3 getTransmittance(float r, float mu, float d, bool intersectsGround)
{
	//Distance from the center of the planet to the point at the ray end
	//Cosine law, mu is a cos of outer angle, hence it is inverted
	float negRMu = r * (-mu);
	float r0 = sqrt(d * d + r * r - 2.0 * negRMu * d);
	//Clamp radius so that it would be bound by atmosphere boundaries
	//float r0Clamped = clamp(r0, Rg, Rt); //Clamp function returning slightly out of bounds values
	float r0Clamped = r0;
	if (r0Clamped < Rg)
		r0Clamped = Rg;
	if (r0Clamped > Rt)
		r0Clamped = Rt;
	float mu0 = -(negRMu - d) / r0Clamped;

	//a, b - arbitrary points, t - point at top atmosphere boundary
	//Tab = Tat / Tbt 
	//min in case division by 0
	//if mu ray intersects ground then reverse direction	
	if (intersectsGround)
	{
		vec3 transmittanceFromR0ToTopAtmosphereBoundary = getTransmittanceToTopAtmosphereBoundary(r0Clamped, -mu0);
		vec3 transmittanceFromaRtoTopAtmosphereBoundary = getTransmittanceToTopAtmosphereBoundary(r, -mu);
		return min(transmittanceFromR0ToTopAtmosphereBoundary / transmittanceFromaRtoTopAtmosphereBoundary, vec3(1.0));
		//return transmittanceFromR0ToTopAtmosphereBoundary;
		//return vec3(r0Clamped - Rg, 0.0, 0.0);
	}
	else
	{
		vec3 transmittanceFromR0ToTopAtmosphereBoundary = getTransmittanceToTopAtmosphereBoundary(r0Clamped, mu0);
		vec3 transmittanceFromaRtoTopAtmosphereBoundary = getTransmittanceToTopAtmosphereBoundary(r, mu);
		return min(transmittanceFromaRtoTopAtmosphereBoundary / transmittanceFromR0ToTopAtmosphereBoundary, vec3(1.0));
	}
}