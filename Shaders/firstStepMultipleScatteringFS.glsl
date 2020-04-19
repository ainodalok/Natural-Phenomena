//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 0) out vec3 colour;

layout(location = 14) uniform int SCATTERING_SPHERICAL_SAMPLES;
layout(location = 15) uniform vec3 surfaceAlbedo;
layout(location = 16) uniform int order;
layout(location = 17) uniform vec3 rBeta;
layout(location = 18) uniform vec3 mBeta;
layout(location = 19) uniform float rH;
layout(location = 20) uniform float mH;

void main()
{
	colour = vec3(0.0f);
	bool fragRayIntersectsGround;
	float fragRayD;
	vec4 RMuMuSNu = getInscatteringRMuMuSNuFromSTLayer(fragRayIntersectsGround, fragRayD);

	//Z is not considered as s will be rotated accordingly so that nu constraint would be satisfied
	//Y equals to Mu since v is a unit vector
	//X is derived from unit vector length formula
	vec3 v = vec3(0.0f, RMuMuSNu.y, 0.0f);
	v.x = sqrt(max(0.0f, 1.0f - v.y * v.y));
	//v = normalize(v);
	//Same idea but Z is calculated as X before was
	//and X is calculated in cases when v is not pointing straight upwards or downwards
	vec3 s = vec3(0.0f, RMuMuSNu.z, 0.0f);
	//X is derived from dot product formula v.x * s.x + 
	//										v.y * s.y + 
	//										v.z * s.z = 
	//										|v| * |s| * nu
	//v.z = 0
	//nu, s.y, v.y and v.x are all known
	if (v.x != 0.0f)
		s.x = (RMuMuSNu.w - v.y * s.y) / v.x;
	s.z = sqrt(max(0.0f, 1.0f - s.x * s.x - s.y * s.y));
	// Step for integration of half a circle
	float dAngle = PI / SCATTERING_SPHERICAL_SAMPLES;

	for (int i = 0; i < SCATTERING_SPHERICAL_SAMPLES; i++)
	{
		float phi = (i + 0.5) * dAngle;
		float phiMu = cos(phi);
		float RPhiMu = RMuMuSNu.x * phiMu;

		vec3 transmittance = vec3(0.0f);
		//Discriminant
		float d = RPhiMu * RPhiMu - RMuMuSNu.x * RMuMuSNu.x + Rg * Rg;
		bool intersectsGround = false;
		if ((d >= 0) && (phiMu < 0.0))	//Looking at the ground
		{
			intersectsGround = true;
			//Distance to the ground
			d = -RPhiMu - sqrt(d);
			transmittance = getTransmittance(RMuMuSNu.x, phiMu, d, intersectsGround);
		}

		//Integrating full circle for each phi
		for (int u = 0; u < SCATTERING_SPHERICAL_SAMPLES * 2; u++)
		{
			float theta = (u + 0.5f) * dAngle;
			vec3 omega = vec3(cos(theta) * sin(phi), phiMu, sin(theta) * sin(phi));
			//Spherical coordinates integration
			float dOmega = dAngle * dAngle * sin(phi);
			float nu = dot(s, omega);
			//If current calculated order is 2, then delta texture store 1st order, 
			//in which case Rayleigh and Mie scattering are separated and without phase terms
			vec3 scattering = getInterpolatedInscattering(RMuMuSNu.x, phiMu, RMuMuSNu.z, nu, intersectsGround, scatteringRDeltaTex).rgb;
			
			if (order == 1) 
				scattering = phaseRay(nu) * scattering + 
					phaseMie(nu) * getInterpolatedInscattering(RMuMuSNu.x, phiMu, RMuMuSNu.z, nu, intersectsGround, scatteringMDeltaTex).rgb;

			vec3 irradiance = vec3(0.0f);
			if (intersectsGround)
			{
				vec3 normal = normalize(vec3(0.0f, RMuMuSNu.x, 0.0f) + omega * d);
				float omegaMuS = dot(normal, s);
				irradiance = transmittance * surfaceAlbedo / PI * getIrradiance(Rg, omegaMuS, irradianceDeltaTex);
			}

			float phaseCosTheta = dot(v, omega);
			colour += (scattering + irradiance) * 
					(rBeta * exp(-(RMuMuSNu.x - Rg) / rH) * phaseRay(phaseCosTheta) + mBeta * exp(-(RMuMuSNu.x - Rg) / mH) * phaseMie(phaseCosTheta)) * 
					dOmega;
		}
	}
}