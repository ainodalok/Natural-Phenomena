//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 14) uniform int SCATTERING_SAMPLES;

layout(location = 0) out vec3 colour;

vec3 multipleScatteringIntegrand(vec4 RMuMuSNu, bool intersectsGround, float travelledDistance)
{
	float r0 = sqrt(RMuMuSNu.x * RMuMuSNu.x + 2.0f * RMuMuSNu.x * RMuMuSNu.y * travelledDistance + travelledDistance * travelledDistance);
	r0 = clamp(r0, Rg, Rt);
	float muS0 = (RMuMuSNu.x * RMuMuSNu.z + travelledDistance * RMuMuSNu.w) / r0;
	muS0 = clamp(muS0, -1.0f, 1.0f);
	float mu0 = (RMuMuSNu.x * RMuMuSNu.y + travelledDistance) / r0;
	mu0 = clamp(mu0, -1.0f, 1.0f);
	return getInterpolatedInscattering(r0, mu0, muS0, RMuMuSNu.w, intersectsGround, scatteringJDeltaTex).rgb
		* getTransmittance(RMuMuSNu.x, RMuMuSNu.y, travelledDistance, intersectsGround);
}

void main()
{
	bool intersectsGround;
	float d;
	vec4 RMuMuSNu = getInscatteringRMuMuSNuFromSTLayer(intersectsGround, d);
	float segmentLength = d / SCATTERING_SAMPLES;

	for (int i = 0; i <= SCATTERING_SAMPLES; i++)
	{
		float travelledDistance = i * segmentLength;
		vec3 rmDelta = multipleScatteringIntegrand(RMuMuSNu, intersectsGround, travelledDistance) * segmentLength;
		if (i == 0 || i == SCATTERING_SAMPLES)
			rmDelta = rmDelta * 0.5f;
		colour += rmDelta;
	}
}