#version 330 core 

uniform sampler2D uTexture0;

in vec2 vTexCoord;
in vec4 vColor;

layout(location = 0) out vec4 ResultColor;

void main() {

#if 0
	vec4 f = textureGather(uTexture0, vTexCoord, 3);
	
	float hr0 = abs(f.y - f.x);
	float hr1 = abs(f.w - f.z);
	float vr0 = abs(f.z - f.x);
	float vr1 = abs(f.w - f.y);

	float maximum = max(max(hr0, hr1), max(vr0, vr1));
#else 
	vec4 f = texture(uTexture0, vTexCoord);
#endif 
	ResultColor.rgb = vColor.rgb;
	ResultColor.a = vColor.a*f.a;//(step(.75, .25*(f.x+f.y+f.z+f.w)) - step(.875, maximum));
}
