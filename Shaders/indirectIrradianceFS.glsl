//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 0) out vec3 colour;

layout(location = 14) uniform int order;
layout(location = 15) uniform int IRRADIANCE_SPHERICAL_SAMPLES;

void main()
{
	colour = vec3(0.0f);
	vec2 RMuS = getIrradianceRMuSFromST();
	vec3 s = vec3(0.0f, RMuS.y, 0.0f);
	s.x = sqrt(max(1.0f - s.y * s.y, 0.0f));

	float dAngle = PI / IRRADIANCE_SPHERICAL_SAMPLES;
	//Integrating over hemisphere, 90 degree slices
	for (int i = 0; i < IRRADIANCE_SPHERICAL_SAMPLES / 2; i++)
	{
		float phi = (i + 0.5f) * dAngle;
		for (int u = 0; u < 2 * IRRADIANCE_SPHERICAL_SAMPLES; u++)
		{
			float theta = (u + 0.5f) * dAngle;
			vec3 omega = vec3(cos(theta) * sin(phi), cos(phi), sin(theta) * sin(phi));
			float dOmega = dAngle * dAngle * sin(phi);
			float nu = dot(s, omega);
			vec3 irradiance = getInterpolatedInscattering(RMuS.x, cos(phi), RMuS.y, nu, false, scatteringRDeltaTex).rgb;
			if (order == 1)
                irradiance = irradiance * phaseRay(nu) + getInterpolatedInscattering(RMuS.x, cos(phi), RMuS.y, nu, false, scatteringMDeltaTex).rgb * phaseMie(nu);
			colour += irradiance * cos(phi) * dOmega;
		}
	}
}