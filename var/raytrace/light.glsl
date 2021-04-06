#version 460 core

#define PI (3.141592653589793)

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2DArray image_trace;
layout(r32ui, binding = 1) uniform uimage2DRect image_trace_index;
layout(rgba32f, binding = 2) uniform image2DRect image_screen;

uniform vec3 lightPosition;
uniform vec3 lightColor;
const vec4 material = vec4(0.15, 0.6, 0.25, 8.0);

void main() {
     uint id = imageLoad(image_trace_index, ivec2(gl_WorkGroupID.xy)).x;
     if (id == 0) {
//        imageStore(image_screen, ivec2(gl_WorkGroupID.xy), imageLoad(image_trace, ivec3(gl_WorkGroupID.xy, 1)));
        return;
    }
    vec3 materialColor = imageLoad(image_trace, ivec3(gl_WorkGroupID.xy, 0)).xyz;
    vec3 N = imageLoad(image_trace, ivec3(gl_WorkGroupID.xy, 1)).xyz;
    vec3 hitPoint = imageLoad(image_trace, ivec3(gl_WorkGroupID.xy, 2)).xyz;
    vec3 V = imageLoad(image_trace, ivec3(gl_WorkGroupID.xy, 3)).xyz;

    vec3 L = normalize(lightPosition - hitPoint);
    vec3 R = reflect(-L, N);
    vec3 color = material.x * materialColor;
    color += max(0.0, dot(L, N)) * material.y * materialColor * lightColor;
    color += pow(max(0.0, dot(R, V)), material.w) * material.z * lightColor;
    imageStore(image_screen, ivec2(gl_WorkGroupID.xy), vec4(color, 1.0));
}
