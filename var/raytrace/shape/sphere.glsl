#version 460 core

#define PI (3.141592653589793)

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2DArray image_ray;
layout(rgba32f, binding = 1) uniform image2DArray image_trace;
layout(r32ui, binding = 2) uniform uimage2DRect image_trace_index;

uniform uint id;
uniform vec3 position;
uniform float radius;
uniform vec3 color;

void main() {
    vec3 rayOrigin = imageLoad(image_ray, ivec3(gl_WorkGroupID.xy, 0)).xyz;
    vec3 rayDirection = imageLoad(image_ray, ivec3(gl_WorkGroupID.xy, 1)).xyz;

    vec3 s = rayOrigin - position;
    float a = dot(rayDirection, rayDirection);
    float b = 2.0 * dot(rayDirection, s);
    float c = dot(s, s) - radius * radius;
    float D = b * b - 4.0 * a * c;
    if (D < 0.0) {
        return;
    }
    float x = (-b - sqrt(D)) / (2.0 * a);
    if (x < 0.0) {
        x = (-b + sqrt(D)) / (2.0 * a);
        if (x < 0.0) {
            return;
        }
    }
    vec3 hitPoint = rayOrigin + x * rayDirection;
    vec3 normal = normalize(hitPoint - position);
    if(dot(rayDirection, normal) > 0.0) {
        normal = -normal;
    }
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 0), vec4(color, 1.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 1), vec4(normal, 1.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 2), vec4(hitPoint, x));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 3), vec4(-rayDirection, 1.0));
    imageStore(image_trace_index, ivec2(gl_WorkGroupID.xy), uvec4(id, 0, 0, 0));
}
