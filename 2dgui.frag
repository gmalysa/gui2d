#version 330

// Really simple fragment shader just textures it
in vec3 ex_tex;

uniform sampler2D tex;

void main(void) {
	vec4 texSample = texture2D(tex, ex_tex.xy);
	gl_FragColor = vec4(texSample.rgb, ex_tex.z * texSample.a);
}
