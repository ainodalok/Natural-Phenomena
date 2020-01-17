layout(location = 4) uniform vec3 rBeta;
layout(location = 5) uniform vec3 mBetaExt;
layout(location = 6) uniform float rH;
layout(location = 7) uniform float mH;
layout(location = 8) uniform int TRANSMITTANCE_SAMPLES;

layout(location = 0) out vec3 color;

//Only precompute transmittance to top atmosphere boundaray, since transmittance for rays that intersect ground can be
//calculated by using reversed rays as presented in Bruneton 2017 implementation
vec3 getTransmittanceRMuFromUV()
{
	//Adjusted uv so that transmittance values range would be mapped to a texture range [0.5/n; 1.0 - 0.5/n] instead of [0.0; 1.0], 
	//since fragment shader always looks up at values in the center of the texel, hence sampling outside gives extrapolated values 
	//gl_FragCoord / T - fragment coord converted to range [0.5/n; 1.0 - 0.5/n]
	//0.5 / T - center of the first fragment converted to [0.5/n; 1.0 - 0.5/n] fragment coords  
	//gl_FragCoord / T - 0.5 / T - conversion to range [0.0; 1.0 - 1.0/n]
	//Last step is a normalization to [0.0; 1.0] range
	float aU = (gl_FragCoord.x / T_W - 0.5 / T_W) / (1.0 - 1.0 / T_W);
	float aV = (gl_FragCoord.y / T_H - 0.5 / T_H) / (1.0 - 1.0 / T_H);

	//Actual mapping
	//Distance to horizon from top atmosphere boundary
	float H = sqrt(Rt * Rt - Rg * Rg);
	//Rho - distance to horizon from current r, calculated from MU_R formula
	float rho = aU * H;
	//Getting r from rho formula
	float r = sqrt(rho * rho + Rg * Rg);

	//Minimum distance to atmosphere boundary
	float dMin = Rt - r;
	//Maximum distance to Rt (distance to horizon from current r + distance to top atmosphere boundary from horizon)
	float dMax = rho + H;
	//Distance to top atmosphere boundary derived from mapping formula MU_MU = (d - d_min) / (d_max - d_min)
	float d = aV * (dMax - dMin) + dMin;

	//Mu derived from distance to top atmosphere boundary formula  d = -r * mu + sqrt(r * r * mu * mu - r * r + Rt * Rt)
	//In case distance to top atmosphere boundary is 0, avoid division by 0 by setting mu to 1, 
	//since distance to top atmosphere boundary equals to 0 only when r = Rt and mu = 1 (looking up)
	//Clamp cosine as sometimes it is just a bit outside of [-1; 1] boundaries most likely due to precision error
	float mu = d == 0 ? 1.0 : clamp((-r * r + Rt * Rt - d * d) / (2 * d * r), -1.0, 1.0);
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
	vec3 RMuD = getTransmittanceRMuFromUV();
	//Distance to the atmosphere top (r is always inside atmosphere, mu is a cosine of view-zenith angle that describes non-ground-intersecting ray)
	float r = RMuD.x;
	float mu = RMuD.y;
	float d = RMuD.z;
	color = exp(-(rBeta * opticalDepth(rH, r, mu, d) + mBetaExt * opticalDepth(mH, r, mu, d)));
}