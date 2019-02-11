#version 430

layout(std140, binding = 0) buffer Pos {
   vec4 vPos[];
};

layout(std140, binding = 1) buffer Vel {
    vec4 vVel[];
};

layout(std140, binding = 2) buffer TTL {
   float ttl[];
};

layout (local_size_x = 16, local_size_y = 16) in;

uniform float dt;// = 0.02;
uniform vec3 g;// = vec3(0, -9.81, 0);
uniform vec3 vSpawn;// = vec3(0, 1, 0);
uniform float tSpawn;// = 1.0;

void main() {
    uint idx = gl_GlobalInvocationID.x;

    /*vec4 p = vPos[idx];
    vec4 v = vVel[idx];
    float t = ttl[idx];

    t -= dt;

    // Respawn
    if(t < 0.0) {
        t += tSpawn;
        p = vec4(0, 0, 0, 1.0);
        v = vec4(vSpawn, 1.0);
    } else {
        v += vec4(g * dt, 1.0);
        p += v * dt;
    }

    vPos[idx] = p;
    vVel[idx] = v;
    ttl[idx] = t;*/
}