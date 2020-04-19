//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 0) out vec4 colour;

void main()
{
	vec3 stp = vec3(gl_FragCoord.xy, float(gl_Layer)) / vec3(MU_MU_S * MU_NU, MU_MU, MU_R - 1.0f);
	bool intersectsGround;
	float d;
	vec4 RMuMuSNu = getInscatteringRMuMuSNuFromSTLayer(intersectsGround, d);
	//Phase factor omitted from resulting scattering texture
	//Not overwriting single scattering Mie contribution in alpha channel
	colour = vec4(texture(scatteringRDeltaTex, stp).rgb / phaseRay(RMuMuSNu.w), 0.0f);
}