#version 460 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2DArray image_trace;
layout(rgba32f, binding = 1) uniform image2DRect image_screen;
layout(r32f, binding = 2) uniform image2DRect image_depth;
layout(r32ui, binding = 3) uniform uimage2DRect image_stencil;

void main() {
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 0), vec4(0.0, 0.0, 0.0, 0.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 1), vec4(0.0, 0.0, 0.0, 0.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 2), vec4(0.0, 0.0, 0.0, 0.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 3), vec4(0.0, 0.0, 0.0, 0.0));
    imageStore(image_screen, ivec2(gl_WorkGroupID.xy), vec4(0.0, 0.0, 0.0, 0.0));
    float f_inf = uintBitsToFloat(0x7F800000);
    imageStore(image_depth, ivec2(gl_WorkGroupID.xy), vec4(f_inf, 0.0, 0.0, 0.0));
    imageStore(image_stencil, ivec2(gl_WorkGroupID.xy), uvec4(0, 0, 0, 0));
}