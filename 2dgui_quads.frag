#version 330

// Simple quad fragment shader just colors
in vec4 ex_color;

void main(void) {
	gl_FragColor = ex_color;
}
