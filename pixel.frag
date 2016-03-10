#version 450

uniform sampler2D srcTexture;
in vec2 texCoord;
out vec4 out_Color;

void main(void)
{
	vec4 color = texture(srcTexture, texCoord);
	out_Color = color;//vec4(0, 1, 0, 1);
}
