// shadertype=glsl
#include "common/common.glsl"
layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec3 TexCoords;

layout(location = 0, binding = 0) uniform sampler2D uni_equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec2 uv = SampleSphericalMap(normalize(TexCoords));
	vec3 color = texture(uni_equirectangularMap, uv).rgb;
	FragColor = vec4(color, 1.0);
}