#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;
layout(location = 2) in vec3 inCol;

out vec2 chTex;
out vec4 chCol;

uniform vec2 uOffset;

void main()
{
	vec2 pos = inPos + uOffset;
	gl_Position = vec4(pos, 0.0, 1.0);

	chTex = inTex;
	chCol = vec4(inCol, 1.0);
}