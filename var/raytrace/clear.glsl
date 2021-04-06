#version 460 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2DArray image_trace;
layout(rgba32f, binding = 1) uniform image2DRect image_screen;
layout(r32ui, binding = 2) uniform uimage2DRect image_trace_index;

void main() {
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 0), vec4(0.0, 0.0, 0.0, 0.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 1), vec4(0.0, 0.0, 0.0, 0.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 2), vec4(0.0, 0.0, 0.0, 0.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 3), vec4(0.0, 0.0, 0.0, 0.0));
    imageStore(image_screen, ivec2(gl_WorkGroupID.xy), vec4(0.0, 0.0, 0.0, 0.0));
    imageStore(image_trace_index, ivec2(gl_WorkGroupID.xy), uvec4(0, 0, 0, 0));
}