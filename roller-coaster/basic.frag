#version 330 core

in vec2 chTex;
in vec4 chCol;
out vec4 outCol;


uniform sampler2D uTex;
uniform bool useTexture;
void main()
{
	if (useTexture) {
        outCol = texture(uTex, chTex);
    } else {
        outCol = chCol;
    }
}