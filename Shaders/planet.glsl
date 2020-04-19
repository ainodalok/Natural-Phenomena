//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 14) uniform vec3 s;
layout(location = 15) uniform float width;
layout(location = 16) uniform float height;
layout(location = 17) uniform float fov_x;
layout(location = 18) uniform mat3 viewMatrix;
layout(location = 19) uniform vec3 x;
layout(location = 20) uniform vec3 surfaceAlbedo;
layout(location = 21) uniform vec3 rBeta;
layout(location = 22) uniform vec3 mBeta;
layout(location = 23) uniform float exposure;
layout(location = 24) uniform vec3 whitePoint;

#ifdef VERTEX
layout(location = 0) out vec3 ray;
layout(location = 1) out vec2 st;

void main()
{
	st = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(st * 2.0f + -1.0f, -1.0f, 1.0f);
	ray = viewMatrix * vec3(gl_Position.x * tan(radians(fov_x) / 2.0f), gl_Position.y * tan(radians(fov_x) / 2.0f) / (width/ height), gl_Position.z);
}
#endif

#ifdef FRAGMENT
layout(location = 0) in vec3 ray;
layout(location = 1) in vec2 st;

layout(location = 0) out vec4 colour;

//L0
float directSunlight(vec3 nu)
{
	if (dot(nu, s) > cos(sunAngularRadius))
		return //Smooth out the edges of the solar disc
			(dot(nu, s) - cos(sunAngularRadius)) / (1.0f - cos(sunAngularRadius)) *
			//1.5 / (3.141... constant solar radiance, not physically accurate but does job better than old implementation
			solarRadiance;
	else
		return 0.0;
}

//R[L0] Lambertian reflection with sun as a source
vec3 groundLight(vec3 x, vec3 v, float d)
{
	vec3 x0 = x + v * d;
	//In our spherical planet case r0 is always equal to Rg
	float r0 = length(x0);
	//Clamp to ground level
	if (r0 < Rg)
		r0 = Rg;
	//Normal
	vec3 n = x0 / r0;
	//Sun-zenith cosine 
	float muS0 = dot(n, s);

	vec3 indirectIrradiance = getIrradiance(r0, muS0, irradianceTex); 
	//Approximation of irradiance for non-horizontal surfaces, map cosine between zenith and surface normal to [0, 1] range
	// * (dot(n, surfaceNormal) + 1.0f) / 2.0f;
	vec3 directIrradiance = getTransmittanceToSun(r0, muS0) * max(muS0, 0.0) * solarIrradiance;

	return directIrradiance + indirectIrradiance;
}

vec4 exponentialToneMappingWithBakedInGamma(vec4 colour)
{
	return vec4(pow(vec3(1.0f) - exp(-vec3(colour.xyz) * exposure / whitePoint), vec3(1.0f / 2.2f)), 1.0f);
}

vec3 extrapolateMie(vec4 mixedInscattering)
{
	//Red channel of Rayleigh scattering empty - there is no Mie scattering
	if (mixedInscattering.r == 0.0)
		return vec3(0.0f);
	return mixedInscattering.rgb * 
			mixedInscattering.a / mixedInscattering.r *
			rBeta.r / mBeta.r *
			mBeta / rBeta;
}

void main()
{
	vec3 fragRay = normalize(ray);

	//r and mu must be clamped in case outside atmosphere
	vec3 xBound = x;
	vec3 normal = normalize(xBound);
	float r = length(xBound);
	float mu = dot(normal, fragRay);
	
	float RMu = r * mu;
	//Outside of atmosphere
	if (r > Rt)
	{
		//Check for intersection with atmosphere
		float dA = RMu * RMu - r * r + Rt * Rt;
		if ((dA >= 0) && (mu < 0.0))
		{
			dA = -RMu - sqrt(dA);
			//Move r and xBound to atmosphere boundary and recalculate mu
			xBound += fragRay * dA;
			r = Rt;
			mu = -(-RMu - dA) / r;
			RMu = r * mu;
			normal = normalize(xBound);
		}
	}

	float muS = dot(normal, s);
	float nu = dot(fragRay, s);
	
	//If looking into atmosphere or inside atmosphere
	if (r <= Rt)
	{
		float d = RMu * RMu - r * r + Rg * Rg;
		bool intersectsGround = false;		
		vec4 mixedInscattering = getInterpolatedInscattering(r, mu, muS, nu, intersectsGround, scatteringSTex);
		vec3 singleMie = extrapolateMie(mixedInscattering);	
		
		if ((d >= 0) && (mu < 0.0))	//Looking at the ground
		{
			intersectsGround = true;
			d = -RMu - sqrt(d);
			vec3 transmittance = getTransmittance(r, mu, d, intersectsGround);	
			//If intersection with something on the ground, do shadow calculations
			//Eq. 13
				//End of the ray on the ground surface
				//vec3 x0 = xBound + fragRay * d;
				//float r0 = length(x0);
				//float mu0 = -(-RMu - d) / r0;
				//float RMu0 = r0 * mu0;
				//inscatterR -= getInterpolatedInscattering(r0, mu0, muS, nu, intersectsGround, scatteringRDeltaTex).rgb * transmittance;
				//inscatterM -= getInterpolatedInscattering(r0, mu0, muS, nu, intersectsGround, scatteringMDeltaTex).rgb * transmittance;
			vec3 inscatter = max(mixedInscattering.rgb * phaseRay(nu) + singleMie * phaseMie(nu), 0.0f);
			colour = vec4(transmittance * surfaceAlbedo / PI * groundLight(xBound, fragRay, d) + inscatter, 1.0f);
		} 
		else	//Looking at the sky
		{
			d = -RMu + sqrt(RMu * RMu - r * r + Rt * Rt);
			vec3 inscatter = max(mixedInscattering.rgb * phaseRay(nu) + singleMie * phaseMie(nu), 0.0f);
			colour = vec4(getTransmittance(r, mu, d, intersectsGround) * vec3(directSunlight(fragRay)) + inscatter, 1.0f);
		}
	}
	else	//Only could possibly see sun while not looking into atmosphere
		colour = vec4(vec3(1.0) * directSunlight(fragRay), 1.0); //Ideal transmittance

	colour = exponentialToneMappingWithBakedInGamma(colour);
}
#endif
