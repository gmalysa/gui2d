#version 330

// Really simple untextured 2D quad shader

// 3D coordinate input, should have a fixed Z for 2d
in vec3 in_vert;
in vec4 in_color;

out vec4 ex_color;

void main(void) {
	gl_Position = vec4(in_vert.xy, in_vert.z, 1);
	ex_color = in_color;
}
