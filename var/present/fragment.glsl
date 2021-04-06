#version 460 core

out vec4 fragmentColor;

uniform layout(rgba32f, binding = 0) image2DRect screen;

void main() {
    fragmentColor = imageLoad(screen, ivec2(gl_FragCoord.xy));
}
