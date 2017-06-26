#version 330 core

in vec2 o_tex_coords;
uniform sampler2D texture_diffuse;

out vec4 color;

void main () {
  color = texture(texture_diffuse, o_tex_coords);
}
