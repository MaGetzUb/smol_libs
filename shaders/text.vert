#version 330 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

uniform mat4 uMVP;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
	
	vTexCoord = aTexCoord;
	vColor = aColor;
	gl_Position = uMVP * vec4(aPosition, 0., 1.);
}
