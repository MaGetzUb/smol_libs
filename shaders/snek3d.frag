#version 330 core

in vec3 vPos;
in vec4 vCol;
flat in int vIndex;

uniform float uTime;

layout(location = 0) out vec4 ResultColor;

void main() {

	vec3 ddx = dFdx(vPos);
	vec3 ddy = dFdy(vPos);

	vec3 n = normalize(cross(ddx, ddy));

	const vec3 l = normalize(vec3(0.5, 1.5, 1.0));

	vec3 eye = -normalize(vPos);

	ResultColor = vec4(vCol.rgb*.25 + (vCol.rgb*.75 * clamp(dot(n, l), 0., 1.) + vec3(0., 0., .5)*dot(n, -l)), 1.);
}