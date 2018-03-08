#version 330

// Vertex shader for the text renderer, unpacks the vec4 data type used to conserve memory

// Input is packed, xy are position, zw are texture coordinates 
in vec2 in_vert;
in vec2 in_tex;

// Output to next step is just the texture coordinate data
out vec2 ex_texcoord;

// Allow us to specify the z coordinate with a uniform (to determine stacking order)
uniform float un_z;

void main(void) {
	gl_Position = vec4(in_vert, un_z/32767, 1);
	ex_texcoord = in_tex;
}
