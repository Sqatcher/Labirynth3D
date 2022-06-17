#version 330

in vec4 outcol2;

void main()
{
	gl_FragColor  = vec4(outcol2.x, outcol2.y,outcol2.z, 0.1f);
}
