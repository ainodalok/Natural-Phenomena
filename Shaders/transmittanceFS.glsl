//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 14) uniform vec3 rBeta;
layout(location = 15) uniform vec3 mBetaExt;
layout(location = 16) uniform float rH;
layout(location = 17) uniform float mH;
layout(location = 18) uniform int TRANSMITTANCE_SAMPLES;

layout(location = 0) out vec3 color;

//Only precompute transmittance to top atmosphere boundaray, since transmittance for rays that intersect ground can be
//calculated by using reversed rays as presented in Bruneton 2017 implementation
vec3 getTransmittanceRMuFromST()
{
	//Adjusted uv so that transmittance values range would be mapped to a texture range [0.5/n; 1.0 - 0.5/n] instead of [0.0; 1.0], 
	//since fragment shader always looks up at values in the center of the texel, hence sampling outside gives extrapolated values 
	//gl_FragCoord - 0.5 - fragment coord converted to range [0.0; n - 1]
	// /(n - 1) - normalization to [0.0; 1.0] range
	float aS = (gl_FragCoord.x - 0.5f) / (T_W - 1.0f);
	float aT = (gl_FragCoord.y - 0.5f) / (T_H - 1.0f);

	//Actual mapping
	//Distance to horizon from top atmosphere boundary
	float H = sqrt(Rt * Rt - Rg * Rg);
	//Rho - distance to horizon from current r, calculated from MU_R formula
	float rho = aS * H;
	//Getting r from rho formula
	float r = clamp(sqrt(rho * rho + Rg * Rg), Rg, Rt);

	//Minimum distance to atmosphere boundary
	float dMin = Rt - r;
	//Maximum distance to Rt (distance to horizon from current r + distance to top atmosphere boundary from horizon)
	float dMax = rho + H;
	//Distance to top atmosphere boundary derived from mapping formula MU_MU = (d - d_min) / (d_max - d_min)
	float d = clamp(aT * (dMax - dMin) + dMin, dMin, dMax);

	//Mu derived from distance to top atmosphere boundary formula  d = -r * mu + sqrt(r * r * mu * mu - r * r + Rt * Rt)
	//In case distance to top atmosphere boundary is 0, avoid division by 0 by setting mu to 1, 
	//since distance to top atmosphere boundary equals to 0 only when r = Rt and mu = 1 (looking up)
	//Clamp cosine as sometimes it is just a bit outside of [-1; 1] boundaries most likely due to precision error
	float mu = d == 0.0f ? 1.0f : clamp((-r * r + Rt * Rt - d * d) / (2 * d * r), -1.0f, 1.0f);
	return vec3(r, mu, d);
}

float opticalDepth(float H, float r, float mu, float d)
{
	float result = 0.0;

	float segmentLength = d / float(TRANSMITTANCE_SAMPLES);
	for (int i = 0; i <= TRANSMITTANCE_SAMPLES; i++) 
	{
		float travelledDistance = i * segmentLength;
		//Square root gives length (magnitude) of segment (vector) from center to point at distance travelDistance along vector r*mu
		float h = sqrt(r * r + travelledDistance * travelledDistance + 2.0f * travelledDistance * r * mu) - Rg;
		//exp is always [0,1] since h and H are both positive, hence exp argument is negative
		float currentOpticalDepthByTravelledDistance = exp(-h / H) * segmentLength;
		//Trapezoidal rule as in new implementation, old implementation had more variables, saved previous values to process each trapezoid separately
		if (i == 0 || i == TRANSMITTANCE_SAMPLES)
			currentOpticalDepthByTravelledDistance = currentOpticalDepthByTravelledDistance * 0.5;

		result += currentOpticalDepthByTravelledDistance;
	}
	return result;
}

void main()
{
	vec3 RMuD = getTransmittanceRMuFromST();
	//Distance to the atmosphere top (r is always inside atmosphere, mu is a cosine of view-zenith angle that describes non-ground-intersecting ray)
	float r = RMuD.x;
	float mu = RMuD.y;
	float d = RMuD.z;
	color = exp(-(rBeta * opticalDepth(rH, r, mu, d) + mBetaExt * opticalDepth(mH, r, mu, d)));
}