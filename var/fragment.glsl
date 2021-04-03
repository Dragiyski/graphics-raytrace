#version 460 core

out vec4 fragmentColor;

uniform sampler2DRect screen;

void main() {
    fragmentColor = texture(screen, gl_FragCoord.xy);
}
