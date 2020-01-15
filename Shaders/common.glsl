#version 460

#define T 0
#define E 1
#define S 2
#define DELTA_J 3
#define DELTA_E 4
#define DELTA_SR 5
#define DELTA_SM 6
#define COUNT 7

vec2 getTransmittanceUvFromRMu(int aT_W, int aT_H, float aRg, float aRt, float r, float mu) 
{
	//Distance to horizon from top atmosphere boundary
	float H = sqrt(aRt * aRt - aRg * aRg);
	//Rho - distance to horizon from current r
	float rho = sqrt(r * r - aRg * aRg);/////////////////BUGGED when r == Rg true
	//Both quadratic equation solution of sphere intersection and geometrically calculated distance from point at r to the atmosphere boundary along the ray
	float d = -r * mu + sqrt(r * r * mu * mu - r * r + aRt * aRt);
	//Minimum distance to atmosphere boundary
	float dMin = aRt - r;
	//Maximum distance to aRt (distance to horizon from current r + distance to top atmosphere boundary from horizon)
	float dMax = rho + H;
	
	//U_R and U_Mu
	float aU = rho / H;
	float aV = (d - dMin) / (dMax - dMin);

	//Adjusted uv so that transmittance values range would be mapped to a texture range [0.5/n; 1.0 - 0.5/n] instead of [0.0; 1.0], 
	//since fragment shader always looks up at values in the center of the texel, hence sampling outside gives extrapolated values
	float u = 0.5 / aT_W + aU * (1.0 - 1.0 / aT_W);
	float v = 0.5 / aT_H + aV * (1.0 - 1.0 / aT_H);
	return vec2(u, v);
	//return vec2(u, rho);
}

//Assumes ray does not intersect ground, otherwise undefined (probably returns clamped values)
vec3 getTransmittanceToTopAtmosphereBoundary(int aT_W, int aT_H, float aRg, float aRt, sampler2D aTransmittanceTex, float r, float mu) 
{
	vec2 uv = getTransmittanceUvFromRMu(aT_W, aT_H, aRg, aRt, r, mu);
	return texture(aTransmittanceTex, uv).rgb;
	//return vec3(uv.y, 0.0, 0.0);
}

//Assumes point at radius r is inside atmosphere
vec3 getTransmittance(int aT_W, int aT_H, float aRg, float aRt, sampler2D aTransmittanceTex, float r, float mu, float d, bool intersectsGround)
{
	//Distance from the center of the planet to the point at the ray end
	//Cosine law, mu is a cos of outer angle, hence it is inverted
	float r0 = sqrt(d * d + r * r - 2.0 * r * (-mu) * d);
	//Clamp radius so that it would be bound by atmosphere boundaries
	float r0Clamped = min(max(r0, aRg), aRt);
	if (r0 > aRt)
		r0Clamped = aRt;
	if (r0 < aRg)
		r0Clamped = aRg;
	//Cosine law inversed for one of the adjacent sides adjacent to the angle of known cosine
	if (r0Clamped != r0)
	{
		//Distance must be also adjusted to boundary
		//Sharp angle case
		if (mu < 0)
			d = r * (-mu) - sqrt(r0Clamped * r0Clamped - r * r + r * r * (-mu) * (-mu));
		else
			d = r * (-mu) + sqrt(r0Clamped * r0Clamped - r * r + r * r * (-mu) * (-mu));
	}
	float mu0 = (d - r * (-mu)) / r0Clamped;

	//a, b - arbitrary points, t - point at top atmosphere boundary
	//Tab = Tat / Tbt 
	//min in case division by 0
	//if mu ray intersects ground then reverse direction	
	if (intersectsGround)
	{
		vec3 transmittanceFromR0ToTopAtmosphereBoundary = getTransmittanceToTopAtmosphereBoundary(aT_W, aT_H, aRg, aRt, aTransmittanceTex, r0Clamped, -mu0);
		vec3 transmittanceFromaRtoTopAtmosphereBoundary = getTransmittanceToTopAtmosphereBoundary(aT_W, aT_H, aRg, aRt, aTransmittanceTex, r, -mu);
		return min(transmittanceFromR0ToTopAtmosphereBoundary / transmittanceFromaRtoTopAtmosphereBoundary, vec3(1.0));
		//return transmittanceFromR0ToTopAtmosphereBoundary;
	}
	else
	{
		vec3 transmittanceFromR0ToTopAtmosphereBoundary = getTransmittanceToTopAtmosphereBoundary(aT_W, aT_H, aRg, aRt, aTransmittanceTex, r0Clamped, mu0);
		vec3 transmittanceFromaRtoTopAtmosphereBoundary = getTransmittanceToTopAtmosphereBoundary(aT_W, aT_H, aRg, aRt, aTransmittanceTex, r, mu);
		return min(transmittanceFromaRtoTopAtmosphereBoundary / transmittanceFromR0ToTopAtmosphereBoundary, vec3(1.0));
	}
}