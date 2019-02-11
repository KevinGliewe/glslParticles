#version 430

in vec3 pos;

out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 p = vec4(pos, 1.0);
    gl_Position = (projection * view * model * p).xyzw;
}
