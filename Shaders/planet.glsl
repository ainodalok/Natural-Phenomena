layout(location = 4) uniform float sunAngularRadius;
layout(location = 5) uniform vec3 s;

#ifdef VERTEX
layout(location = 0) out vec3 ray;
layout(location = 1) out vec2 uv;

void main()
{
	uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(uv * 2.0f + -1.0f, 1.0f, 1.0f);
	ray = vec3(gl_Position.xyz);
}
#endif

#ifdef FRAGMENT
layout(location = 0) in vec3 ray;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 color;

float directSunlight(vec3 v)
{
	if (dot(v, s) > cos(sunAngularRadius))
		return //Smooth out the edges of the solar disc
			//pow((dot(v, s) - cos(sunAngularRadius)) / (1.0 - cos(sunAngularRadius)), 3.0) *
			// *1.5 / (M_... constant solar radiance, not physically accurate but does job better than old implementation
			1.5 / (3.1415926538 * sunAngularRadius * sunAngularRadius);
	else
		return 0.0;
}

void main()
{
	vec3 fragRay = normalize(ray);
	//r and mu must be clamped in case outside atmosphere
	float r = Rg + 1;
	float mu = dot(vec3(0.0, 1.0, 0.0), fragRay);
	//currently assume inside atmosphere
	//check for either first ground or top atmosphere intersection
	float RMu = r * mu;
	float d = RMu * RMu - r * r + Rg * Rg;
	bool intersectsGround = false;
	if ((d >= 0) && (mu < 0.0))
	{
		intersectsGround = true;
		d = -RMu - sqrt(d);
		color = vec4(getTransmittance(r, mu, d, intersectsGround), 1.0);
	}
	else
	{
		d = -RMu + sqrt(RMu * RMu - r * r + Rt * Rt);
		color = vec4(getTransmittance(r, mu, d, intersectsGround), 1.0);
	}
}
#endif