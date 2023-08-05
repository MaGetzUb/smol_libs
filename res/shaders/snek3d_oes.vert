#version 300 es
precision highp float;

layout(location = 0) in vec4 iPosScale;
layout(location = 1) in vec4 iColor;

layout(location = 2) in vec3 aPos;

uniform float uTime;

uniform mat4 uMVP;

out vec4 vCol;
out vec3 vPos;
flat out int vIndex;

void main() {
	vPos = aPos;
	vCol = iColor;
	vIndex = gl_InstanceID;
	//gl_Position = uMVP * vec4((aPos*iPosScale.w) + sin(vec3(uTime)*normalize(vec3(float(vIndex % 30), float((vIndex+10) % 30), float((vIndex+20) % 30)))) + iPosScale.xyz, 1.0);
	gl_Position = uMVP * vec4(aPos*iPosScale.w + iPosScale.xyz, 1.);
}