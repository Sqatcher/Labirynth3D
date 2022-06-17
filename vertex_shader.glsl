#version 330 

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 incol;


uniform mat4 MVP;

out vec4 outcol2;

void main()
{
    gl_Position = MVP*vec4(aPos.x, aPos.y, aPos.z, 1.0);
    outcol2=vec4(incol, 1.0f);
}