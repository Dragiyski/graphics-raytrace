#version 460 core

precision highp float;

layout (location = 0) in vec2 position_in;

void main() {
    gl_Position = vec4(position_in, 0.0, 1.0);
}