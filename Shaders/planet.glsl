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
layout(location = 25) uniform float cloudOpacity;

float bottomLayer = Rg + 2000.0f;
float topLayer = Rg + 12000.0f;

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

bool intersectsCloudLayer(bool intersectsGround, float r, float mu, out float distanceToLayer, out float distanceInsideLayer)
{
	float RMu = r * mu;
	float bottomDiscrim = RMu * RMu - r * r + bottomLayer * bottomLayer;
	float topDiscrim = RMu * RMu - r * r + topLayer * topLayer;
	distanceToLayer = 0.0f;
	distanceInsideLayer = 0.0f;

	//Above low cloud layer boundary
	if (r > bottomLayer)
	{
		//Above top cloud layer boundary
		if (r > topLayer)
		{
			//Intersecting both cloud layer boundaries
			if ((bottomDiscrim >= 0.0f && mu < 0.0f) || intersectsGround)
			{
				float distanceToBottomLayer = -RMu - sqrt(bottomDiscrim);
				float distanceToTopLayer = -RMu - sqrt(topDiscrim);
				distanceInsideLayer = distanceToBottomLayer - distanceToTopLayer;
				distanceToLayer = distanceToTopLayer;
				return true;
			}
			//Intersecting only top cloud layer boundary
			else if (topDiscrim >= 0.0f && mu < 0.0f)
			{
				distanceInsideLayer = sqrt(topDiscrim) * 2.0f;
				distanceToLayer = -RMu - sqrt(topDiscrim);
				return true;
			}
		}
		//Under top cloud layer boundary, but above low cloud layer boundary
		else
		{
			//Intersecting low cloud layer boundary
			if ((bottomDiscrim >= 0.0f && mu < 0.0f) || intersectsGround) 
			{
				distanceInsideLayer = -RMu - sqrt(bottomDiscrim);
				return true;
			}
			//Intersecting top cloud layer boundary
			else
			{
				distanceInsideLayer = -RMu + sqrt(topDiscrim);
				return true;
			}
		}
	}
	//Under both cloud layer and looking at the sky(intersceting both)
	else if (!intersectsGround)
	{
		float distanceToBottomLayer = -RMu + sqrt(bottomDiscrim);
		float distanceToTopLayer = -RMu + sqrt(topDiscrim);
		distanceInsideLayer = distanceToTopLayer - distanceToBottomLayer;
		distanceToLayer = distanceToBottomLayer;
		return true;
	}
	return false;
}

float cloudIntegral(vec3 xCamera, vec3 fragRay, float distanceToLayer, float distanceInsideLayer)
{
	float cameraOpacity = clamp((1.0f - (length(xCamera) - bottomLayer) / (topLayer - bottomLayer)) * 2, 0.0f, 1.0f);
	if (cameraOpacity > 0.01f)
	{
		vec3 xStart = xCamera + fragRay * distanceToLayer;

		int steps = 200;
		float densityMultiplier = 0.0004f;
		//2k to 12k height - 0.00008f x1, good results at 0.00002f
		float cloudFrequency = 0.00001f;

		float coverageFrequency = 6.0f;

		float detailFrequency = 0.0002f;

		float cutOffDensity = 4.605f / densityMultiplier;
		float stepSize = distanceInsideLayer / steps;

		float distanceTravelled = 0.0f;
		float cloudDensity = 0.0f;

		for (int i = 0; i <= steps; i++)
		{
			vec3 position = xStart + distanceTravelled * fragRay;
			float gradient = clamp((1.0f - (length(position) - bottomLayer) / (topLayer - bottomLayer)) * 2, 0.0f, 1.0f);

			float perlinWorley = texture(perlinWorleyTex, vec3(position.zxy * cloudFrequency)).x * gradient;
			//Local coverage
			float cloudDensityD = clamp(remap(perlinWorley, 0.60f, 1.0f, 0.0f, 1.0f), 0.0f, 1.0f);
			
			//Adding detail if there is something to add to
			if (cloudDensityD > 0.0f)
				cloudDensityD = max(cloudDensityD - (1.0f - texture(perlinWorleyTex, vec3(position.zxy * detailFrequency + 0.6f)).x) * (1.0 - cloudDensityD) * 0.3, 0.0f);

			cloudDensityD *= stepSize * gradient * cameraOpacity;

			if (i == 0 || i == steps)
				cloudDensityD *= 0.5f;

			cloudDensity += cloudDensityD;
			if (cloudDensity >= cutOffDensity)
				return exp((-cloudDensity * densityMultiplier));

			distanceTravelled += stepSize;
		}
		return exp(-cloudDensity * densityMultiplier);
	}
	else
		return 1.0f;
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
	
	vec3 result = vec3(0.0f);

	//If looking into atmosphere or inside atmosphere
	if (r <= Rt)
	{
		float d = RMu * RMu - r * r + Rg * Rg;
		bool intersectsGround = false;		

		vec4 mixedInscattering = getInterpolatedInscattering(r, mu, muS, nu, intersectsGround, scatteringSTex);
		vec3 singleMie = extrapolateMie(mixedInscattering);	

		vec3 inscatter;
		vec3 transmittance;

		if ((d >= 0) && (mu < 0.0))	//Looking at the ground
		{
			intersectsGround = true;
			d = -RMu - sqrt(d);
			transmittance = getTransmittance(r, mu, d, intersectsGround);	
			//If intersection with something on the ground, do shadow calculations
			//Eq. 13
				//End of the ray on the ground surface
				//vec3 x0 = xBound + fragRay * d;
				//float r0 = length(x0);
				//float mu0 = -(-RMu - d) / r0;
				//float RMu0 = r0 * mu0;
				//inscatterR -= getInterpolatedInscattering(r0, mu0, muS, nu, intersectsGround, scatteringRDeltaTex).rgb * transmittance;
				//inscatterM -= getInterpolatedInscattering(r0, mu0, muS, nu, intersectsGround, scatteringMDeltaTex).rgb * transmittance;
			inscatter = max(mixedInscattering.rgb * phaseRay(nu) + singleMie * phaseMie(nu), 0.0f);
			result = transmittance * surfaceAlbedo / PI * groundLight(xBound, fragRay, d) + inscatter;
		} 
		else	//Looking at the sky
		{
			d = -RMu + sqrt(RMu * RMu - r * r + Rt * Rt);
			inscatter = max(mixedInscattering.rgb * phaseRay(nu) + singleMie * phaseMie(nu), 0.0f);
			transmittance = getTransmittance(r, mu, d, intersectsGround);

			//colour = vec4(getTransmittance(r, mu, d, intersectsGround) * vec3(directSunlight(fragRay)) + inscatter, 1.0f);
			result = transmittance * vec3(directSunlight(fragRay)) + inscatter;
		}

		float distanceInsideLayer;
		float distanceToLayer;
		bool intersectsCloudLayer = intersectsCloudLayer(intersectsGround, r, mu, distanceToLayer, distanceInsideLayer);
		if (cloudOpacity > 0.1f)
		{
			if (intersectsCloudLayer)
			{
				float cloudTransmittance = cloudIntegral(vec3(xBound.x, xBound.y, xBound.z), fragRay, distanceToLayer, distanceInsideLayer);
				float lengthInscatter;
				vec3 normalizedInscatter;
				if (cloudTransmittance > 0.01f)
				{
					lengthInscatter = length(inscatter);
					normalizedInscatter = inscatter / lengthInscatter;
				}
				result = result * cloudTransmittance + (1.0f - cloudTransmittance) * normalizedInscatter * lengthInscatter * cloudOpacity;
				//result = vec3(cloudTransmittance);
			}
		}
	}
	else	//Could possibly see only sun while not looking into atmosphere
		result = vec3(directSunlight(fragRay)); //Ideal transmittance

	colour = exponentialToneMappingWithBakedInGamma(vec4(result, 1.0f));
	
	/* //Clouds in a box test
	float dim = 100.0f;

	vec3 xG = vec3(x.x, x.y - 1.5f - Rg, x.z) / 10000.0f;
	vec3 invRay = 1.0f / fragRay;
	vec3 t0 = (vec3(0.0f) - xG)*invRay;
	vec3 t1 = (vec3(dim*20, dim , dim * 20) - xG)*invRay;
	vec3 tmin = min(t0,t1); 
	vec3 tmax = max(t0,t1);
	float firstInt = max(tmin.x, max(tmin.y, tmin.z));
	float lastInt = min(tmax.x, min(tmax.y, tmax.z));
	if (firstInt <= lastInt && lastInt >= 0.0f)
	{
		firstInt = max(0.0f, firstInt);
		float transmittance = cloudIntegral(xG, fragRay, firstInt, lastInt-firstInt);
		colour = vec4(vec3((1.0f - transmittance) + transmittance * vec3(0.3f, 0.3f, 0.0f)), 1.0f);
	}
	else
		colour = vec4(vec3(0.3f, 0.3f, 0.0f), 1.0f);
	*/
}
#endif
