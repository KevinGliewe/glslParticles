#version 430

uniform sampler2D tex1;

out vec4 colour;

void main() {
  colour = texture(tex1, gl_PointCoord);
  colour.w = colour.w / 2.0;
}
