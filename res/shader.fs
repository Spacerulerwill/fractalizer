#version 330 core

precision highp float;

in vec2 fragTexCoord;
out vec4 FragColor;

// Fractal types
const int FRACTAL_TYPE_MANDELBROT = 0;
const int FRACTAL_TYPE_BURNING_SHIP = 1;
const int FRACTAL_TYPE_TRICORN = 2;

// Render mode
const int RENDER_MODE_FRACTAL = 0;
const int RENDER_MODE_JULIA_SET = 1;
const int RENDER_MODE_FRACTAL_WITH_JULIA_SET_OVERLAY = 2;

// Uniforms
uniform ivec2 resolution;
uniform vec2 fractalCenterArgand;
uniform float fractalZoom;
uniform vec2 juliaSetCenterArgand;
uniform float juliaSetZoom;
uniform vec2 juliaSetMousePosArgand;
uniform int fractalType;
uniform int renderMode;
uniform int iterations;

// Globals
float aspectRatio;

vec2 compsquare(vec2 z)
{
    float temp = z.x;
    z.x = z.x * z.x - z.y * z.y;
    z.y = 2.0 * temp * z.y;       
    return z;
}

int fractal(vec2 offset) {
    // Adjust for aspect ratio, move to center, apply zoom and flip vertically
    vec2 z = vec2(0.0);
    vec2 c = (gl_FragCoord.xy + offset) / vec2(resolution);
    c.x *= aspectRatio;
    c -= vec2(0.5 * aspectRatio, 0.5);
    c *= fractalZoom;
    c += fractalCenterArgand;
    c.y *= -1.0;
    //calculate iterationts until it escapes
    for (int iters = 0; iters < iterations; ++iters)
    {
        z = compsquare(z) + c;
        if (dot(z, z) > 4.0) return iters;
    }
    return iterations;
}

vec4 fractal_antialias() {
    vec3 fragColor = vec3(float(fractal(vec2(0,0))) / float(iterations));
    fragColor += vec3(float(fractal(vec2(0.5,0))) / float(iterations));
    fragColor += vec3(float(fractal(vec2(0,0.5))) / float(iterations));
    fragColor += vec3(float(fractal(vec2(0.5,0.5))) / float(iterations));
    fragColor /= 4.0;
    return vec4(fragColor, 1.0);
}

int julia_set(vec2 offset) {
    // Adjust for aspect ratio, move to center, apply zoom and flip vertically
    vec2 z = (gl_FragCoord.xy + offset) / vec2(resolution);
    z.x *= aspectRatio;
    z -= vec2(0.5 * aspectRatio, 0.5);
    z *= juliaSetZoom;
    z += juliaSetCenterArgand;
    z.y *= -1.0;
    //calculate iterationts until it escapes
    for (int iters = 0; iters < iterations; ++iters)
    {
        z = compsquare(z) + juliaSetMousePosArgand;
        if (dot(z, z) > 4.0) return iters;
    }
    return iterations;
}

vec4 julia_set_antialias() {
    vec3 fragColor = vec3(float(julia_set(vec2(0.0,0))) / float(iterations));
    fragColor += vec3(float(julia_set(vec2(0.5,0))) / float(iterations));
    fragColor += vec3(float(julia_set(vec2(0,0.5))) / float(iterations));
    fragColor += vec3(float(julia_set(vec2(0.5,0.5))) / float(iterations));
    fragColor /= 4.0;
    return vec4(fragColor, 1.0);
}


void main()
{
    aspectRatio = float(resolution.x) / float(resolution.y);

    switch (renderMode) {
    case RENDER_MODE_FRACTAL:
        FragColor = fractal_antialias();
        break;
    case RENDER_MODE_JULIA_SET:
        FragColor = julia_set_antialias();\
        break;
    case RENDER_MODE_FRACTAL_WITH_JULIA_SET_OVERLAY:
        FragColor = mix(fractal_antialias(), julia_set_antialias(), 0.75);
        break;
    default:
        FragColor = vec4(0.5);
        break;
    }
}