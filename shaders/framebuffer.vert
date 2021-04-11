#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 app_pos;

void main() {
    gl_Position = vec4(app_pos, 1.0);
}