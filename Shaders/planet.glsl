layout(location = 0) uniform float Rg;
layout(location = 1) uniform float Rt;
layout(location = 2) uniform int T_W;
layout(location = 3) uniform int T_H;
layout(location = 4) uniform float sunAngularRadius;
layout(location = 5) uniform vec3 s;
layout(binding = T) uniform sampler2D transmittanceTex;

#ifdef VERTEX

layout(location = 0) in vec2 position;
//layout(location = 1) in vec2 texCoordIn;

//out vec2 texCoord;
layout(location = 0) out vec3 ray;
layout(location = 1) out vec2 coords;

void main()
{
	//texCoord = texCoordIn;
	ray = vec3(position, 1.0);
	coords = (position.xy + 1) * 0.5;
	gl_Position = vec4(position, 1.0, 1.0);
}

#endif

#ifdef FRAGMENT

//in vec2 texCoord;
layout(location = 0) in vec3 ray;
layout(location = 1) in vec2 coords;

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
	float d = r * r * mu * mu - r * r + Rg * Rg;
	bool intersectsGround = false;
	if ((d >= 0) && (mu < 0.0))
	{
		intersectsGround = true;
		d = -r * mu - sqrt(d);
		color = vec4(getTransmittance(T_W, T_H, Rg, Rt, transmittanceTex, r, mu, d, intersectsGround), 1.0);
	}
	else
	{
		d = -r * mu + sqrt(r * r * mu * mu - r * r + Rt * Rt);
		color = vec4( getTransmittance(T_W, T_H, Rg, Rt, transmittanceTex, r, mu, d, intersectsGround), 1.0);
	}

	//color = texture2D(transmittanceTex, coords);
	float test1 = Rg * Rt;
	float test2 = Rg + 10000;

	color = vec4(Rg, Rt, Rg * Rg - Rt, 1.0);
}

#endif