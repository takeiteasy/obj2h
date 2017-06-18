#version 330 core
layout (location = 0) in vec3 positions;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 tex_coords;

uniform mat4 model, view, projection;

out vec2 o_tex_coords;

void main() {
  gl_Position = projection * view * vec4(positions, 1.0f);
  o_tex_coords = tex_coords;
}