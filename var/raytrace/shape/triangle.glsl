#version 460 core

#define PI (3.141592653589793)

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2DArray image_ray;
layout(rgba32f, binding = 1) uniform image2DArray image_trace;

struct Vertex {
    vec3 location;
    // Per-vertex normal is useful for computation of normal map
    // it is intepolated alongside the position of the triangle;
    vec3 normal;
    vec2 uv;
};

uniform Vertex triangle[3];
// This is the triangle normal (i.e. the normal of the plane the triangle lies in);
uniform vec4 plane;

vec3 triangleInterpolate(vec3 point) {
    return normalize(
        vec3(
            length(point - triangle[0].location),
            length(point - triangle[1].location),
            length(point - triangle[2].location)
        )
    );
}

vec3 interpolate3(vec3 item0, vec3 item1, vec3 item2, vec3 coordinates) {
    return coordinates.x * item0 + coordinates.y * item1 + coordinates * item2;
}

void main() {
    vec3 rayOrigin = imageLoad(image_ray, ivec3(gl_WorkGroupID.xy, 0)).xyz;
    vec3 rayDirection = imageLoad(image_ray, ivec3(gl_WorkGroupID.xy, 1)).xyz;

    // Step 1: Compute the intersection between the triangle's plane and the ray:
    // A plane is pre-computed on the CPU (for now, although it is possible to run additional compute shader for that).
    // A plane formula is a * x + b * y + c * z = d
    // Where N = (a, b, c) is the normal to the plane
    // and X = (x, y, z) is a point from that plane
    // and d is used to get concrete plane, as there are infinite number of planes with the same normal
    // This can be rewritten as dot(N, X) = d
    // If X belongs to the ray it is also true that X = O + t * D for some distance t.
    float ND = dot(plane.xyz, rayDirection);
    float t = (plane.w - dot(plane.xyz, rayOrigin)) / ND;

    // In case the ray is parallel to the plane, we won't find any intersection point.
    if (isinf(t) || isnan(t) || t < 0.0) {
        return;
    }

    // Now x is an intersection point to the plane.
    vec3 x = rayOrigin + t * rayDirection;

    // But it might not be an intersection to the triangle
    vec3 triangleCoords = vec3(
        dot(cross(triangle[1].location - triangle[0].location, x - triangle[0].location), plane.xyz),
        dot(cross(triangle[2].location - triangle[1].location, x - triangle[1].location), plane.xyz),
        dot(cross(triangle[0].location - triangle[2].location, x - triangle[2].location), plane.xyz)
    );

    if (triangleCoords.x < 0.0 || triangleCoords.y < 0.0 || triangleCoords.z < 0.0) {
        return;
    }

    triangleCoords = normalize(triangleCoords);
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 0), vec4(1.0, 1.0, 1.0, 1.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 1), vec4(interpolate3(triangle[0].normal, triangle[1].normal, triangle[2].normal, triangleCoords), 1.0));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 2), vec4(x, t));
    imageStore(image_trace, ivec3(gl_WorkGroupID.xy, 3), vec4(-rayDirection, 1.0));
}