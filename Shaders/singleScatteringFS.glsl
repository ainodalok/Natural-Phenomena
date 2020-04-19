//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 14) uniform int SCATTERING_SAMPLES;
layout(location = 15) uniform vec3 rBeta;
layout(location = 16) uniform vec3 mBeta;
layout(location = 17) uniform float rH;
layout(location = 18) uniform float mH;

layout(location = 0) out vec3 colourR;
layout(location = 1) out vec3 colourM;

void singleScatteringIntegrand(vec4 RMuMuSNu, bool intersectsGround, float travelledDistance, out vec3 rDelta, out vec3 mDelta)
{
	//r and muS at the end of the travelledDistance
	float r0 = clamp(sqrt(RMuMuSNu.x * RMuMuSNu.x + 2.0f * RMuMuSNu.x * RMuMuSNu.y * travelledDistance + travelledDistance * travelledDistance), Rg, Rt);
	float muS0 = clamp((RMuMuSNu.x * RMuMuSNu.z + travelledDistance * RMuMuSNu.w) / r0, -1.0f, 1.0f);
	vec3 transmittanceToSun = getTransmittance(RMuMuSNu.x, RMuMuSNu.y, travelledDistance, intersectsGround) * getTransmittanceToSun(r0, muS0);
	//Square root gives length (magnitude) of segment (vector) from center to point at distance travelDistance along vector r*mu
	float h = r0 - Rg;
	rDelta = transmittanceToSun * exp(-h / rH);
	mDelta = transmittanceToSun * exp(-h / mH);
}

void main()
{
	bool intersectsGround;
	float d;
	vec4 RMuMuSNu = getInscatteringRMuMuSNuFromSTLayer(intersectsGround, d);
	float segmentLength = d / SCATTERING_SAMPLES;
	vec3 rSum = vec3(0.0f);
	vec3 mSum = vec3(0.0f);
	for (int i = 0; i <= SCATTERING_SAMPLES; i++)
	{
		float travelledDistance = i * segmentLength;
		vec3 rDelta, mDelta;
		singleScatteringIntegrand(RMuMuSNu, intersectsGround, travelledDistance, rDelta, mDelta);
		if (i == 0 || i == SCATTERING_SAMPLES)
		{
			rDelta = rDelta * 0.5;
			mDelta = mDelta * 0.5;
		}
		rSum +=rDelta;
		mSum +=mDelta;
	}

	colourR = rSum * segmentLength * solarIrradiance * rBeta;
	colourM = mSum * segmentLength * solarIrradiance * mBeta;
}