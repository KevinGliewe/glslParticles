#version 430

uniform sampler2D tex0;
in vec2 Vertex_UV;

out vec4 colour;

void main() {
  colour = texture(tex0, Vertex_UV);
  colour.w = colour.w / 2.0;
}
