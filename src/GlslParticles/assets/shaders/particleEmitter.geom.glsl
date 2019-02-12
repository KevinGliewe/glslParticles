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
    vec3 right = vec3(particle_size, 0, 0);
    vec3 up = vec3(0, particle_size, 0);
    
    vec3 center = (view * model * gl_in[0].gl_Position).xyz;
    
    vec3 va = center - (right + up);
    gl_Position = projection * vec4(va, 1.0);
    Vertex_UV = vec2(0.0, 1.0);
    EmitVertex();  
    
    vec3 vb = center - (right - up);
    gl_Position = projection * vec4(vb, 1.0);
    Vertex_UV = vec2(0.0, 0.0);
    EmitVertex();  

    vec3 vd = center + (right - up);
    gl_Position = projection * vec4(vd, 1.0);
    Vertex_UV = vec2(1.0, 1.0);
    EmitVertex();  

    vec3 vc = center + (right + up);
    gl_Position = projection * vec4(vc, 1.0);
    Vertex_UV = vec2(1.0, 0.0);
    EmitVertex();

    EndPrimitive(); 
}