
#include "graphics.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const char *vertexShaderSource =
#include "vertex.gl"
;

const char *fragment_shader_source =
#include "frag.gl"
;

typedef unsigned int uint;

// globals for error handling
static int success;
static char info_log[512];

/* global opengl state */
static GLFWwindow * window = NULL;
static int num_bodies = 0;
static float * points = NULL;
static uint VAO = 0;
static uint VBO = 0;
static uint EBO = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLFWwindow * init(const char* win_name) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(600, 600, win_name, NULL, NULL);
	if (window == NULL) {
		puts("Failed to create GLFW window");
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		puts("Failed to initialize GLAD");
		return NULL;
	}
	return window;
}

void shader_compile(uint shader, const char* err_msg) {
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, info_log);
		info_log[511] = '\0';
		puts(err_msg);
		puts(info_log);
	}
}

void program_link(uint shaderProgram, const char * err_msg) {
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, info_log);
		info_log[511] = '\0';
		puts(err_msg);
		puts(info_log);
	}
}

void process_input(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


double get_simulation_time() {
	return glfwGetTime();
}

void render(Body * bodies) {

	for (int i=0; i<num_bodies; i++) {
		points[3*i] = (float) bodies[i].pos.x;
		points[3*i + 1] = (float) bodies[i].pos.y;
		points[3*i + 2] = (float) bodies[i].pos.z;
	}

	glClear(GL_COLOR_BUFFER_BIT);

	glBufferData(GL_ARRAY_BUFFER, sizeof(*points) * 3 * num_bodies, points, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_POINTS, 0, num_bodies);

	glfwPollEvents();
	glfwSwapBuffers(window);
}


int init_graphics(int num_bodies_arg) {
	num_bodies = num_bodies_arg;
	window = init("gravsim");
	if (!window) return -1;
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (vertexShader == 0) return -1;
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	shader_compile(vertexShader, "Vertex shader compilation error");

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (fragmentShader == 0) return -1;
	glShaderSource(fragmentShader, 1, &fragment_shader_source, NULL);
	shader_compile(fragmentShader, "Vertex shader compilation error");

	unsigned int shaderProgram = glCreateProgram();
	if (shaderProgram == 0) return -1;
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	program_link(shaderProgram, "Program linking error");
	glUseProgram(shaderProgram);

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	points = malloc(sizeof(*points) * num_bodies * 3);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	/* glGenBuffers(1, &EBO); */
	/* glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); */
	/* glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); */

	glClearColor(0.002f, 0.004f, 0.006f, 1.0f);
	/* glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); */
    return 0;
}

