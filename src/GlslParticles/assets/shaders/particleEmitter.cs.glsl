#version 430

// Particle position
layout(std140, binding = 0) buffer Pos {
   vec4 vPos[];
};

// Particle velocity
layout(std140, binding = 1) buffer Vel {
    vec4 vVel[];
};

// Particle Time To Live [sec]
layout(std140, binding = 2) buffer TTL {
   float ttl[];
};

layout (local_size_x = 16, local_size_y = 16) in;

// Time interval [sec]
uniform float dt = 0.02;
// Gravity
uniform vec3 g = vec3(0, -3.81, 0);
// Velocity the particle spawn
uniform vec3 velSpawn = vec3(0, 1, 0);
// Initial TTL value whe particle spawns
uniform float lifeTime = 1.0;
// Seed to feed into the rand function
uniform float randseed = 0.0;

// Collision shape sphere
const float sphereRadius = 0.2;
const vec3 spherePos = vec3(0.1, -0.3, 0);
const float sphereBouncyness = 0.3;

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

    // Check remaining lifetime of particle
    if(t < 0.0) {
        // Respawn
        t += lifeTime;

        // Random X Axis offset
        float x_offs = rand(vec2(randseed, float(idx))) - 0.5;
        // Random Y Axis offset
        float y_offs = rand(vec2(randseed, float(idx + 1))) - 0.5;

        p = vec4(0, 0, 0, 1.0);
        v = vec4(velSpawn, 1.0) + vec4(x_offs, 0 ,y_offs , 0);
    } else {
        // Gravity
        v += vec4(g * dt, 1.0);
        
        // Move Particle according to velocity
        p += v * dt;

        // Sphere collision
        vec3 s_p = p.xyz - spherePos; // spere -> particle
        if(length(s_p) <= sphereRadius) {
            // collision normal vector
            vec3 normal = normalize(s_p);
            // Reflect particle velocity using collision normal
            v = vec4(reflect(v.xyz, normal) * sphereBouncyness, 1.0);
            // Move paricle outside of the sphere
            p = vec4(normal * (sphereRadius + 0.001) + spherePos, 1.0);
        }
    }

    vPos[idx] = p;
    vVel[idx] = v;
    ttl[idx] = t;
}