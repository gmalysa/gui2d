#version 330

// Really simple vertex shader for 2D items

// 3D coordinate input
in vec3 in_vert;
in vec3 in_tex;

out vec3 ex_tex;

void main(void) {
	gl_Position = vec4(in_vert.xy, in_vert.z, 1);
	ex_tex = in_tex;
}
