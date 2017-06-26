#include <stdio.h>
#include <errno.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "linmath.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "untitled_obj.h"

#define PI180 .01745329251994329576f
#define DEG2RAD(X) (X * PI180)

const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;

SDL_Window*	  window;
SDL_GLContext context;

char* __load_file(const char* path) {
	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "fopen \"%s\" failed: %d %s\n", path, errno, strerror(errno));
		exit(1);
	}

	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	rewind(file);

	char *data = (char*)calloc(length + 1, sizeof(char));
	fread(data, 1, length, file);
	fclose(file);

	return data;
}

GLuint load_texture(const char* path) {
	GLuint texture = 0;
	glGenTextures(1, &texture);
	
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	int width, height, chans;
	unsigned char* data = stbi_load(path, &width, &height, &chans, 0);
	
	if (!data) {
		fprintf(stderr, "stbi_load failed to load \"%s\"\n", path);
		abort();
	}
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	stbi_image_free(data);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return texture;
}

GLuint __make_shader(GLenum type, const char* src) {
	const char* src_data = __load_file(src);

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src_data, NULL);
	glCompileShader(shader);

	free((char*)src_data);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		GLchar *info = (GLchar*)calloc(length, sizeof(GLchar));
		glGetShaderInfoLog(shader, length, NULL, info);
		fprintf(stderr, "glCompileShader failed:\n%s\n", info);

		free(info);
		exit(-1);
	}

	return shader;
}

GLuint __make_program(GLuint vert, GLuint frag) {
	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		GLchar *info = calloc(length, sizeof(GLchar));
		glGetProgramInfoLog(program, length, NULL, info);
		fprintf(stderr, "glLinkProgram failed: %s\n", info);

		free(info);
		exit(-1);
	}

	glDetachShader(program, vert);
	glDetachShader(program, frag);
	glDeleteShader(vert);
	glDeleteShader(frag);

	return program;
}

GLuint load_shader(const char* vert, const char* frag) {
	return __make_program(__make_shader(GL_VERTEX_SHADER,		vert),
			   __make_shader(GL_FRAGMENT_SHADER, frag));
}

void cleanup() {
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	printf("Goodbye!\n");
}

int main(int argc, const char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Failed to initalize SDL!\n");
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("im not gay",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	if (!window) {
		fprintf(stderr, "Failed to create SDL window!\n");
		return -1;
	}

	context = SDL_GL_CreateContext(window);
	if (!context) {
		fprintf(stderr, "Failed to create OpenGL context!\n");
		return -1;
	}

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initalize GLEW!\n");
		return -1;
	}

	printf("Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version:  %s\n", glGetString(GL_VERSION));
	printf("GLSL:     %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	atexit(cleanup);

	mat4x4 p, v, m;
	mat4x4_perspective(p, 45.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, .1f, 1000.f);
	vec3 eye = { 0.f, 0.f, 3.f };
	vec3 cen = { 0.f, 0.f, 0.0f };
	vec3 up  = { 0.f, 1.f, 0.f };
	mat4x4_look_at(v, eye, cen, up);
	mat4x4_identity(m);

	GLuint texture = load_texture("earth.jpg");

	GLuint shader = load_shader("test.vert.glsl", "test.frag.glsl");

	GLuint p_loc = glGetUniformLocation(shader, "projection");
	GLuint v_loc = glGetUniformLocation(shader, "view");
	GLuint m_loc = glGetUniformLocation(shader, "model");

	unsigned int VBO_v, VBO_vt, VBO_vn, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO_v);
	glGenBuffers(1, &VBO_vt);
	glGenBuffers(1, &VBO_vn);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_v);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Sphere_vertices), Sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	glBindBuffer(GL_ARRAY_BUFFER, VBO_vn);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Sphere_normals), Sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_vt);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Sphere_texcoords), Sphere_texcoords, GL_STATIC_DRAW);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	glDeleteBuffers(1, &VBO_v);
	glDeleteBuffers(1, &VBO_vn);
	glDeleteBuffers(1, &VBO_vt);

	Uint32 old_time, current_time = SDL_GetTicks();
	float delta;

	SDL_bool running = SDL_TRUE;
	const Uint8* keys;
	SDL_Event e;
	while (running) {
		old_time = current_time;
		current_time = SDL_GetTicks();
		delta = (float)(current_time - old_time) / 1000.0f;

		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT:
					running = SDL_FALSE;
					break;
			}
		}

		keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_GetScancodeFromKey(SDLK_ESCAPE)])
			running = SDL_FALSE;

		mat4x4_rotate_Y(m, m, DEG2RAD(5.f) * delta);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 0);

		glUniformMatrix4fv(p_loc, 1, GL_FALSE, &p[0][0]);
		glUniformMatrix4fv(v_loc, 1, GL_FALSE, &v[0][0]);
		glUniformMatrix4fv(m_loc, 1, GL_FALSE, &m[0][0]);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, Sphere_num_vertices);
		glBindVertexArray(0);

		glUseProgram(0);

		SDL_GL_SwapWindow(window);
	}

	glDeleteProgram(shader);
	glDeleteVertexArrays(1, &VAO);

	return 0;
}
