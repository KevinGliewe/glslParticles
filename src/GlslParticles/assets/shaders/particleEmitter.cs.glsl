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

uniform float dt = 0.02;
uniform vec3 g = vec3(0, -9.81, 0);
uniform vec3 velSpawn = vec3(0, 1, 0);
uniform float lifeTime = 1.0;
uniform float randseed = 0.0;

// https://stackoverflow.com/questions/12964279/whats-the-origin-of-this-glsl-rand-one-liner
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    uint idx = gl_GlobalInvocationID.x;

    vec4 p = vPos[idx];
    vec4 v = vVel[idx];
    float t = ttl[idx];

    t -= dt;

    // Respawn
    if(t < 0.0) {
        t += lifeTime;
        float alpha = rand(vec2(randseed, float(idx)));
        float beta = rand(vec2(randseed, float(idx + 1)));
        p = vec4(0, 0, 0, 1.0);
        v = vec4(velSpawn, 1.0) + vec4(alpha, 0 ,beta, 0);
    } else {
        v += vec4(g * dt, 1.0);
        p += v * dt;
    }

    vPos[idx] = p;
    vVel[idx] = v;
    ttl[idx] = t;
}