#version 330 core

in vec2 chTex;
in vec4 chCol;
out vec4 outCol;


uniform sampler2D uTex;
uniform bool useTexture;
uniform float uTransparency;
void main()
{
	if (useTexture) {
        vec4 texCol = texture(uTex, chTex);
        outCol = vec4(texCol.rgb, texCol.a * uTransparency);
    } else {
        outCol = vec4(chCol.rgb, chCol.a * uTransparency);
    }
}