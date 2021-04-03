#version 460 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform layout(rgba32f) image2DRect screen;

void main() {
    imageStore(screen, ivec2(gl_WorkGroupID.xy), vec4(0.5, 0.0, 0.0, 1.0));
}
