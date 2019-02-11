#version 430

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform float particle_size = 0.01; // Particle size

out vec2 Vertex_UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    mat4 mv = model * view;

    vec3 right = vec3(mv[0][0], mv[1][0], mv[2][0]);
    vec3 up = vec3(mv[0][1], mv[1][1], mv[2][1]);
    
    vec3 p = gl_in[0].gl_Position.xyz;
    
    vec3 va = p - (right + up) * particle_size;
    gl_Position = mv * vec4(va, 1.0);
    Vertex_UV = vec2(0.0, 0.0);
    EmitVertex();  
    
    vec3 vb = p - (right - up) * particle_size;
    gl_Position = mv * vec4(vb, 1.0);
    Vertex_UV = vec2(0.0, 1.0);
    EmitVertex();  

    vec3 vd = p + (right - up) * particle_size;
    gl_Position = mv * vec4(vd, 1.0);
    Vertex_UV = vec2(1.0, 0.0);
    EmitVertex();  

    vec3 vc = p + (right + up) * particle_size;
    gl_Position = mv * vec4(vc, 1.0);
    Vertex_UV = vec2(1.0, 1.0);
    EmitVertex();  
    
    EndPrimitive(); 
}