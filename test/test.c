#include <stdio.h>
#include <errno.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "linmath.h"
#include "untitled_obj.h"

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

	mat4x4 proj, view;
	mat4x4_perspective(proj, 45.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, .1f, 1000.f);
	vec3 eye = { 0.f, 0.f, 3.f };
	vec3 cen = { 0.f, 0.f, 0.0f };
	vec3 up  = { 0.f, 1.f, 0.f };
	mat4x4_look_at(view, eye, cen, up);

	GLuint shader = load_shader("test.vert.glsl", "test.frag.glsl");

	GLuint proj_loc = glGetUniformLocation(shader, "projection");
	GLuint view_loc = glGetUniformLocation(shader, "view");

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Sphere_vertices), Sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	SDL_bool running = SDL_TRUE;
	const Uint8* keys;
	SDL_Event e;
	while (running) {
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

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader);

		glUniformMatrix4fv(proj_loc, 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(view_loc, 1, GL_FALSE, &view[0][0]);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, Sphere_num_vertices);
		glBindVertexArray(0);

		glUseProgram(0);

		SDL_GL_SwapWindow(window);
	}

	glDeleteProgram(shader);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	return 0;
}
