#version 460 core

#define PI (3.141592653589793)

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform layout(rgba32f, binding = 0) image2DArray ray;

uniform ivec2 screenSize;
uniform vec2 viewSize;
uniform float screenRadius;
uniform vec3 cameraOrigin;
uniform vec3 cameraDirection;
uniform float cameraRoll;

void main() {
    /* This part is dyanmic and depends on the current work group */
    vec2 relCoord = vec2(gl_WorkGroupID.xy) / vec2(screenSize);
    vec2 rectCoord = relCoord * viewSize * 2.0 - viewSize;
    vec3 flatCoord = vec3(rectCoord, 0.0);
    vec3 origin = vec3(0.0, 0.0, screenRadius);
    vec3 direction = normalize(flatCoord - origin);

    imageStore(ray, ivec3(gl_WorkGroupID.xy, 0), vec4(cameraOrigin, 1.0));
    imageStore(ray, ivec3(gl_WorkGroupID.xy, 1), vec4(direction, 1.0));
}
