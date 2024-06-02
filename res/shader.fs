#version 330 core

precision highp float;

in vec2 fragTexCoord;
out vec4 FragColor;

#define MANDELBROT 0

uniform ivec2 resolution = ivec2(1280, 720);
uniform vec2 location = vec2(0.0, 0.0);
uniform vec2 mousePos = vec2(0.0, 0.0);
uniform int fractalType = MANDELBROT;
uniform int isJuliaModeEnabled = 0;
uniform int iterations = 200;
uniform float zoom = 2.0;

vec2 compsquare(vec2 z)
{
    float temp = z.x;
    z.x = z.x * z.x - z.y * z.y;
    z.y = 2.0 * temp * z.y;       
    return z;
}

int fractal(vec2 offset) {
    vec2 uv = (gl_FragCoord.xy + offset) / vec2(resolution);
    float ratio = float(resolution.x) / float(resolution.y);
    uv.x *= ratio;
    uv -= vec2(0.5 * ratio, 0.5);
    uv *= zoom;
    uv += location;
    uv.y *= -1.0;
    vec2 z = vec2(0.0);
    if (isJuliaModeEnabled == 1) {
        z = uv;
        uv = mousePos;
    }
    //calculate iterationts until it escapes
    for (int iters = 0; iters < iterations; ++iters)
    {
        z = compsquare(z) + uv;
        if (dot(z, z) > 4.0) return iters;
    }
    return iterations;
}

void main()
{
    vec3 fragColor = vec3(float(fractal(vec2(0.0,0))) / float(iterations));
    fragColor += vec3(float(fractal(vec2(0.5,0))) / float(iterations));
    fragColor += vec3(float(fractal(vec2(0,0.5))) / float(iterations));
    fragColor += vec3(float(fractal(vec2(0.5,0.5))) / float(iterations));
    fragColor /= 4.0;
    FragColor = vec4(fragColor, 1.0);
}