//Special GLSL language integration VS extension directive
//! #version 460

#define T 0
#define E 1
#define S 2
#define DELTA_J 3
#define DELTA_E 4
#define DELTA_SR 5
#define DELTA_SM 6
#define PERLIN_WORLEY 7
#define COUNT 8

#define PI 3.1415926538f

layout(location = 0) uniform float Rg;
layout(location = 1) uniform float Rt;
layout(location = 2) uniform int T_W;
layout(location = 3) uniform int T_H;
layout(location = 4) uniform int E_W;
layout(location = 5) uniform int E_H;
layout(location = 6) uniform int MU_R;		//S_LAYERS
layout(location = 7) uniform int MU_MU;		//S_H
layout(location = 8) uniform int MU_MU_S;	//| 
											//|* = S_W
layout(location = 9) uniform int MU_NU;		//|
layout(location = 10) uniform float sunAngularRadius;
//The smallest possible angle for which scattering precomputation yields negligible results
//Necessary so that more memory could be allocated for result precision
layout(location = 11) uniform float muSmin;
layout(location = 12) uniform float g;
layout(location = 13) uniform float solarIrradiance;

layout(binding = T) uniform sampler2D transmittanceTex;
layout(binding = DELTA_E) uniform sampler2D irradianceDeltaTex;
layout(binding = DELTA_SR) uniform sampler3D scatteringRDeltaTex;
layout(binding = DELTA_SM) uniform sampler3D scatteringMDeltaTex;
layout(binding = E) uniform sampler2D irradianceTex;
layout(binding = DELTA_J) uniform sampler3D scatteringJDeltaTex;
layout(binding = S) uniform sampler3D scatteringSTex;
layout(binding = PERLIN_WORLEY) uniform sampler3D perlinWorleyTex;

//Constant solar ir/-radiance, no wavelength luminance precomputation
float solarRadiance = solarIrradiance / (PI * sunAngularRadius * sunAngularRadius);

#ifdef FRAGMENT

////////////////////////////////////////////////////////////////////TRANSMITTANCE FUNCTIONS/////////////////////////////////////////////////////////////////

vec2 getTransmittanceSTFromRMu(float r, float mu)
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
	float aS = rho / H;
	//Improved formula
	//Taken from Bruneton's 2017 implementation which added -dMin to both dividend and divisor, which sticks lower boundary of aV range to 0.0, 
	//otherwise it may be bigger depending on the dMin, which in turn dismisses a part of the texture not filling it with data
	float aT = (d - dMin) / (dMax - dMin);

	//Adjusted uv so that transmittance values range would be mapped to a texture range [0.5/n; 1.0 - 0.5/n] instead of [0.0; 1.0], 
	//since fragment shader always looks up at values in the center of the texel, hence sampling outside gives extrapolated values
	precise float s = 0.5 / T_W + aS * (1.0 - 1.0 / T_W);
	precise float t = 0.5 / T_H + aT * (1.0 - 1.0 / T_H);
	return vec2(s, t);
	//return vec2(s, rho);
}

//Assumes ray does not intersect ground, otherwise undefined (probably returns clamped values)
vec3 getTransmittanceToTopAtmosphereBoundary(float r, float mu) 
{
	vec2 st = getTransmittanceSTFromRMu(r, mu);
	return texture(transmittanceTex, st).rgb;
	//return vec3(st.y, 0.0, 0.0);
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

//Overload with position (x) instead of height (r)
vec3 getTransmittance(vec3 x, float mu, float d, bool intersectsGround)
{
	return getTransmittance(length(x), mu, d, intersectsGround);
}

//Using calculations to top atmosphere boundary calculates approximated transmission to sun as an integrand
//over sun disk
//Fraction varies from 0 when sun is completely below the horizon to 1 when above
//Which is approximated by
//muS < cos(horizonZenithAngle) - sunAngularRadius * sin(horizonZenithAngle) -- below horizon
//muS > cos(horizonZenithAngle) + sunAngularRadius * sin(horizonZenithAngle) -- above horizon
//and a hermite interpolation to calculate the fraction in between
vec3 getTransmittanceToSun(float r, float muS)
{
	float sinHorZenAngle = Rg / r;
	//Sum of sin/cos squares formula
	float cosHorZenAngle = -sqrt(max(1.0f - sinHorZenAngle * sinHorZenAngle, 0.0f));
	return getTransmittanceToTopAtmosphereBoundary(r, muS) * smoothstep(-sinHorZenAngle * sunAngularRadius,
																	   sinHorZenAngle * sunAngularRadius,
																	   muS - cosHorZenAngle);
}

//////////////////////////////////////////////////////////////////////IRRADIANCE FUNCTIONS//////////////////////////////////////////////////////////////////

vec2 getIrradianceRMuSFromST()
{
	//Same idea as in getTransmittanceRMuFromST()
	float aS = (gl_FragCoord.x - 0.5f) / (E_W - 1.0f);
	float aT = (gl_FragCoord.y - 0.5f) / (E_H - 1.0f);
	
	vec2 RMuS;
	RMuS.x = clamp(Rg + aS * (Rt - Rg), Rg, Rt);
	RMuS.y = clamp(2.0f * aT- 1.0f, -1.0f, 1.0f);

	return RMuS;
}

vec2 getIrradianceSTFromRMuS(float r, float muS)
{
	float aS = (r - Rg) / (Rt - Rg);
	float aT = (muS + 1.0f) / 2.0f;

	precise float s = 0.5 / E_W + aS * (1.0 - 1.0 / E_W);
	precise float t = 0.5 / E_H + aT * (1.0 - 1.0 / E_H);
	return vec2(s, t);
}

vec3 getIrradiance(float r, float muS, sampler2D irradianceTexture)
{
	vec2 st = getIrradianceSTFromRMuS(r, muS);
	return texture(irradianceTexture, st).rgb;
}

/////////////////////////////////////////////////////////////////////INSCATTERING FUNCTIONS//////////////////////////////////////////////////////////////////

vec4 getInscatteringRMuMuSNuFromSTLayer(out bool intersectsGround, out float d)
{
	float aQ = float(gl_Layer) / (MU_R - 1.0f);
	float H = sqrt(Rt * Rt - Rg * Rg);
	float rho = aQ * H;
	float r = sqrt(rho * rho + Rg * Rg);
	r = clamp(r, Rg, Rt);

	float mu;
	//Getting value of Mu when ray intersects bottom boundary(ground)
	if (gl_FragCoord.y < MU_MU * 0.5f)
	{
		float dmin = r - Rg;
		float dmax = rho;
		//Two parts of the texture are considered as separate texture, hence MU_MU mapping for each half is normalized
		float aP = (0.5 * MU_MU - gl_FragCoord.y - 0.5f) / (0.5 * MU_MU - 1.0f);
		d = dmin + (dmax - dmin) * aP;
		//Looking straight down
		if (d == 0.0f)
			mu = -1.0f;
		else
			//Inverse d formula from mu
			mu = -(rho * rho + d * d) / (2.0f * r * d);
		intersectsGround = true;
	}
	//Getting value of Mu when ray intersects top boundary(atmosphere)
	else
	{
		intersectsGround = false;
		float dmin  = Rt - r;
		float dmax = rho + H;
		//Two parts of the texture are considered as separate texture, hence MU_MU mapping for each half is normalized
		float aP = (gl_FragCoord.y - 0.5 * MU_MU - 0.5f) / (0.5 * MU_MU - 1.0f);
		d = dmin + (dmax - dmin) * aP;
		//Looking straight up
		if (d == 0.0f)
			mu = 1.0f;
		else
			//Inverse d formula from mu
			mu = (-r * r - d * d + Rt * Rt) / (2.0f * r * d);
	}
	mu = clamp(mu, -1.0f, 1.0f);

	float aT = (mod(gl_FragCoord.x, MU_MU_S) - 0.5f) / (MU_MU_S - 1.0f);
	float muS;
	//Similar idea as with mu and transmittance with increased precision at horizon, with configurable ad-hoc planet parameter
	//Instead of hard-coded one for Earth as suggested in paper
	//Shortest distance to top boundary from the bottom boundary
	float dSmin = Rt - Rg;
	//Longest distance to top boundary from the bottom boundary
	float dSmax = H;
	//This formula is taken from Brunteon2017
	float A = -2.0f * muSmin * Rg / (dSmax - dSmin);
	//Distance to top atmosphere boundary for muS ray to max distance to top atmosphere boundary (cf. inverse mapping)
	//This formula would be enough to increase precision close to the horizon, but it would not cutoff non-significant scattering values
	//and would require more complex texture halving than in mu, hence usage of A
	float a = (A - aT * A) / (1.0 + aT * A);
	//This is where cutoff and increased precision mapping happens
	//min(a, A) is actually a muMuS coordinate which is used to determine d from which muS is easily derived
	float dS = dSmin + min(a, A) * (dSmax - dSmin);
	//If so happens that Rt == Rg then d would be equal to 0, then just set muS to always be 1 (looking up) as it doesn't really matter,
	//since there is no Atmospheric Scattering
	if (dS == 0)
		muS = 1.0f;
	else
		//d formula, with r = Rg solved for mu
		muS = (H * H - dS * dS) / (2.0f * Rg * dS);
	muS = clamp(muS, -1.0f, 1.0f);

	float aS = floor(gl_FragCoord.x / MU_MU_S) / (MU_NU - 1.0f);
	float nu = aS * 2.0f - 1.0f;
	//As suggested in Bruneton2017, nu must be clamped to min and max angle between mu and muS since it's value depends on both of these
	//Formulas used:
	//sinA * sinA + cosA * cosA = 1
	//cosA-B = cosA * cosB - sinA * sinB
	//cosA+B = cosA * cosB + sinA * sinB
	nu = clamp(nu, mu * muS - sqrt((1.0f - mu * mu) * (1.0f - muS * muS)), mu * muS + sqrt((1.0f - mu * mu) * (1.0f - muS * muS)));

	//return vec4(r, mu, muS, nu);
	return vec4(r, mu, muS, nu);
}

vec4 getInscatteringS0S1TPFromRMuMuSNu(float r, float mu, float muS, float nu, bool intersectsGround, out float lerp)
{
	float rho = sqrt(r * r - Rg * Rg);
	float H = sqrt(Rt * Rt - Rg * Rg);
	float q = rho / H;

	float p;
	if(intersectsGround)
	{
		float dmin = r - Rg;
		float dmax = rho;
		//Same value as if d == dmin
		if(dmin == dmax)
			p = 0.5f - 0.5f / MU_MU;
			//d = dmin;
		else
		{
			float RMu = r * mu;
			float d = -RMu - sqrt(RMu * RMu - rho * rho);
			float aP = (d - dmin) / (dmax - dmin);
			p = 0.5f - 0.5f * aP + (aP - 0.5f) / MU_MU;
		}
	}
	else
	{
		float dmin = Rt - r;
		float dmax = rho + H;
		if (dmax == dmin)
		{
			p = 0.5f + 0.5f / MU_MU;
		}
		else
		{
			float RMu = r * mu;
			float d = -RMu + sqrt(RMu * RMu - r * r + Rt * Rt);
			float aP = (d - dmin) / (dmax - dmin);
			p = 0.5f + 0.5f * aP - (aP - 0.5f) / MU_MU;
		}
	}

	//Distance to top atmosphere boundary from bottom atmosphere boundary, sun direction
	float dS = -Rg * muS + sqrt(Rg * Rg * muS * muS + H * H);
	float dSmin = Rt - Rg;
	float dSmax = H;
	float a = (dS - dSmin) / (dSmax - dSmin);
	//Cutoff value
	float A = -2.0f * muSmin * Rg / (dSmax - dSmin);
	float t = 0.5f / MU_MU_S;
	if (a < A)
	{
		float aT = (1.0f - a / A) / (1.0f + a);
		t += aT * (1.0f - 1.0f / MU_MU_S);
	}

	float s = (nu + 1.0f) / 2.0f;

	//4th dimension interpolation of nu coord
	float nuTexCoord = s * (MU_NU - 1.0f);
	float nuTexCoordSpecificTexel = floor(nuTexCoord);
	lerp = nuTexCoord - nuTexCoordSpecificTexel;
	float s0 = (nuTexCoordSpecificTexel + t) / MU_NU;
	float s1 = s0 + 1.0f / MU_NU;

	return vec4(s0, s1, p, q);
}

vec4 getInterpolatedInscattering(float r, float mu, float muS, float nu, bool intersectsGround, sampler3D inscatteringTexture)
{
	float lerp;
	vec4 S0S1TLayer = getInscatteringS0S1TPFromRMuMuSNu(r, mu, muS, nu, intersectsGround, lerp);
	return texture(inscatteringTexture, S0S1TLayer.xzw) * (1.0f - lerp) + 
			texture(inscatteringTexture, S0S1TLayer.yzw) * lerp;
	//return vec4(vec3(lerp), 1.0);
}

//Eq. 2
float phaseRay(float cosTheta)
{
	return (3.0f / (16.0f * PI)) * (1.0f + cosTheta * cosTheta);
}

//Eq. 4
float phaseMie(float cosTheta)
{
	return 3.0f / 
				(8.0f * PI) * 
			(1.0f - g * g) * (1.0 + cosTheta * cosTheta) / 
				(pow(1.0f + g * g - 2.0f * g * cosTheta, 3.0 / 2.0) * (2.0f + g * g));
}

/////////////////////////////////CLOUDS/////////////////////////////

float remap(float x, float oldMin, float oldMax, float newMin, float newMax)
{
    return newMin + (((x - oldMin) / (oldMax - oldMin)) * (newMax - newMin));
}

float worleyFBM(float x, float y, float z)
{
    return x * 0.625f + y * 0.25f + z * 0.125f;
}

#endif