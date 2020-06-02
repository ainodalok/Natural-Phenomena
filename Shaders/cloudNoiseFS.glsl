//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 14) uniform int CLOUD_W;
layout(location = 15) uniform int CLOUD_H;
layout(location = 16) uniform int CLOUD_D;

layout(location = 0) out float perlin_worley;

//Inigo Quilez 2017 Integer based (sinless) Hash
vec3 hash(vec3 cell)
{
    uvec3 uintCell = floatBitsToUint(cell);
	const uint k = 1103515245U;
    uintCell = ((uintCell>>8U)^uintCell.yzx)*k;
    uintCell = ((uintCell>>8U)^uintCell.yzx)*k;
    uintCell = ((uintCell>>8U)^uintCell.yzx)*k;
    
    return vec3(uintCell)*(1.0f/float(0xffffffffU));
}

float perlin(vec3 p, float freq)
{
    p *= freq;
    vec3 flooredST = floor(p);
    vec3 fractionST = p - flooredST;

    //Perlin interpolator, smootherstep
    vec3 alpha = fractionST * fractionST * fractionST * (fractionST * (fractionST * 6.0f - 15.0f) + 10.0f);

    //Cube points
    //  g_________h
    //  |\   \   |\
    //  | \c__\__|_\d
    //  |  |  |  |  |
    // e|__|__|__|f |
    //   \ | \|   \ |
    //   a\|__\____\|b
    vec3 a = vec3(0.0f, 0.0f, 0.0f);    
    vec3 b = vec3(1.0f, 0.0f, 0.0f);
    vec3 c = vec3(0.0f, 1.0f, 0.0f);
    vec3 d = vec3(1.0f, 1.0f, 0.0f);
    vec3 e = vec3(0.0f, 0.0f, 1.0f);  
    vec3 f = vec3(1.0f, 0.0f, 1.0f);    
    vec3 g = vec3(0.0f, 1.0f, 1.0f);    
    vec3 h = vec3(1.0f, 1.0f, 1.0f);
    
    //Gradients for each tile that has cube point in its center
    vec3 aG = hash(mod(flooredST + a, freq)) * 2.0f - 1.0f;
    vec3 bG = hash(mod(flooredST + b, freq)) * 2.0f - 1.0f;
    vec3 cG = hash(mod(flooredST + c, freq)) * 2.0f - 1.0f;
    vec3 dG = hash(mod(flooredST + d, freq)) * 2.0f - 1.0f;
    vec3 eG = hash(mod(flooredST + e, freq)) * 2.0f - 1.0f;
    vec3 fG = hash(mod(flooredST + f, freq)) * 2.0f - 1.0f;
    vec3 gG = hash(mod(flooredST + g, freq)) * 2.0f - 1.0f;
    vec3 hG = hash(mod(flooredST + h, freq)) * 2.0f - 1.0f;

    //Dot product between gradient vector and a vector to a point in a tile for each corner of a cube
    float aDot = dot(aG, fractionST - a);
    float bDot = dot(bG, fractionST - b);
    float cDot = dot(cG, fractionST - c);
    float dDot = dot(dG, fractionST - d);
    float eDot = dot(eG, fractionST - e);
    float fDot = dot(fG, fractionST - f);
    float gDot = dot(gG, fractionST - g);
    float hDot = dot(hG, fractionST - h);

    //Trilinear interpolation
    //Plane interpolation
    float ab = mix(aDot, bDot, alpha.x);
    float cd = mix(cDot, dDot, alpha.x);
    float ef = mix(eDot, fDot, alpha.x);
    float gh = mix(gDot, hDot, alpha.x);

    //Line interpolation
    float abef = mix(ab, ef, alpha.z);
    float cdgh = mix(cd, gh, alpha.z);

    //Point interpolation
    float abefcdgh = mix(abef, cdgh, alpha.y);

    return abefcdgh; 
}

float perlinFBM(vec3 p, float freq, int octaves)
{
    float amp = 1.0f;
    float ampSum = 0.0f;
    float noise = 0.0f;
    for (int i = 0; i < octaves; i++)
    {
        noise += perlin(p, freq) * amp;
        ampSum += amp;
        freq *= 2.0f;
        amp *= 0.5f;
    }
    return noise / ampSum;
}

float worley(vec3 p, float freq)
{
    p *= freq;
    vec3 flooredST = floor(p);
    vec3 fractionST = p - flooredST;

    float minDist =1.0f;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; z++)
            {
                vec3 neighborCell = vec3(x, y, z);
                vec3 cell = flooredST + neighborCell;

                cell = mod(freq + cell, freq);

                vec3 point = hash(cell);
                vec3 vectorBetween = neighborCell + point - fractionST;
                float d = length(vectorBetween);
                minDist = min(minDist, d);
            }
        }
     }  
     return  1.0 - minDist;
}

//From book of shaders
void main()
{
    vec3 stp = vec3(gl_FragCoord.xy, float(gl_Layer)) / vec3(CLOUD_W, CLOUD_H, CLOUD_D - 1.0f);

    float worley4 = worley(stp, 4);
    float worley8 = worley(stp, 8);
    float worley16 = worley(stp, 16);
    float worley32 = worley(stp, 32);
    float worley64 = worley(stp, 64);

    float lowFreqWorleyFBM = worleyFBM(worley4, worley8, worley16);
    float medFreqWorleyFBM = worleyFBM(worley8, worley16, worley32);
    float highFreqWorleyFBM = worleyFBM(worley16, worley32, worley64);
    
    float lowFreqPerlinFBM = perlinFBM(stp, 4.0f, 7);
    //Different shape, not as described in GPU Pro 7 book, using low frequency Worley FBM as old low makes shapes wispy instead of billowy
    perlin_worley = remap(lowFreqPerlinFBM, 0.0f, 1.0f, lowFreqWorleyFBM, 1.0f);
    //Add shape details
    perlin_worley = remap(perlin_worley, -(worleyFBM(lowFreqWorleyFBM, medFreqWorleyFBM, highFreqWorleyFBM) - 1.0f), 1.0f, 0.0f, 1.0f);
    //Normalize
    perlin_worley = clamp(remap(perlin_worley, -0.9f, 1.0f, 0.0f, 1.0f), 0.0f, 1.0f);
}