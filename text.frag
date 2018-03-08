#version 330

// Fragment shader for the text renderer, uses the alpha texture to determine color
in vec2 ex_texcoord;

uniform sampler2D tex;

uniform vec4 color;

void main(void) {
	// For now, we don't multiply by the color, but in the future we will
	gl_FragColor = vec4(color.xyz, color.w*texture2D(tex, ex_texcoord).r);
}
