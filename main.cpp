#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
 
#define print(x) std::cout << #x << std::flush

#define Width 640
#define Height 480

GLuint tex = 0;

void test_shader(GLuint shader) {

	GLint result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if(result == GL_FALSE) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(shader, length, &length, message);
        std::cout 
            << "\nWARNING: Failed to compile \n" 
            << message
            << std::endl;
    }
}

void window_setup(GLFWwindow *window) {
	glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		glfwTerminate();
        return;
	}
}

void draw_setup() {
	
	int w, h, channels;
	stbi_set_flip_vertically_on_load(true); 
	unsigned char *data = stbi_load("test.jpg", &w, &h, &channels, 0);

	GLuint tex_output;
	glGenTextures(1, &tex_output);

	tex = tex_output;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	static const float vertex_data[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // lb
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,	// rb
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,	// rt
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f	// lt
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0};

    const char* vertex_shader_code =
	"#version 460\n"
	"layout(location = 0) in vec3 pos;\n"
	"layout(location = 1) in vec2 in_tex_coord;\n"
	"out vec2 tex_coord;\n"
	"void main()\n"
	"{\n"
	"	tex_coord = in_tex_coord;\n"
	"	gl_Position.xyz = pos;\n"
	"	gl_Position.w = 1.0;\n"
	"}\n";

    const char* fragment_shader_code =
	"#version 460\n"
	"in vec2 tex_coord;\n"
	"out vec4 color;\n"
	"uniform int w;\n"
	"uniform int h;\n"
	"uniform sampler2D out_tex;\n"
	"layout(rgba32f, binding = 0) uniform image2D img_output;\n"
	"void main()\n"
	"{\n"
	"	color = texture(out_tex, tex_coord);\n"
	"}\n";

    GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * 5 * sizeof(float), vertex_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(3 * sizeof(float)));

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_code, nullptr);
    glCompileShader(vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_code, nullptr);
	glCompileShader(fragment_shader);

	test_shader(fragment_shader);
	test_shader(vertex_shader); 

	GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
	glValidateProgram(program);
 	glUseProgram(program);

	GLint location_w = glGetUniformLocation(program, "w");
	glUniform1i(location_w, Width);
	GLint location_h = glGetUniformLocation(program, "h");
	glUniform1i(location_h, Height);
}

void compute_setup() {
    
	const char *cs_shader_code =
	"#version 460\n"
	"layout(local_size_x = 1, local_size_y = 1) in;\n"
	"layout(rgba32f, binding = 0) uniform image2D img_output;\n"
	"uniform int w;\n"
	"uniform int h;\n"
	"void main()\n"
	"{\n"
	"	ivec2 pos = ivec2(gl_GlobalInvocationID.xy);\n"
	"	ivec2 new_pos = pos - ivec2(1,0);\n"
	"	if(new_pos.x < 0) new_pos.x = w - 1;\n"
	"	vec4 pixel = imageLoad(img_output, new_pos);\n"
	"	imageStore(img_output, pos, pixel);\n"
	"}\n";

	while(!tex) {
		
	}

	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	GLuint cs_shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(cs_shader, 1, &cs_shader_code, NULL);
	glCompileShader(cs_shader);

	test_shader(cs_shader);

	GLuint program = glCreateProgram();
	glAttachShader(program, cs_shader);
	glLinkProgram(program);
	glUseProgram(program);

		GLint location_w = glGetUniformLocation(program, "w");
	glUniform1i(location_w, Width);
	GLint location_h = glGetUniformLocation(program, "h");
	glUniform1i(location_h, Height);
}

void start_draw_thread(GLFWwindow *window, bool *finished) {

	window_setup(window);
	draw_setup();

	while (!glfwWindowShouldClose(window) && !*finished)
    {   
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glfwSwapBuffers(window);
    }

	*finished = true;
}

void start_compute_thread(GLFWwindow *window, bool *finished) {

	window_setup(window);
	compute_setup();

	GLuint own_tex = 0;

	while (!glfwWindowShouldClose(window) && !*finished)
    {   
		glDispatchCompute((GLuint) Width, (GLuint) Height, 1);
        glfwSwapBuffers(window);
    }

	*finished = true;
}

int main(void)
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	GLFWwindow *draw_window = glfwCreateWindow(Width, Height, "Draw", nullptr, nullptr);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	GLFWwindow *compute_window = glfwCreateWindow(Width, Height, "Compute", nullptr, draw_window);

	if (!draw_window || !compute_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

	bool draw_finished = false, compute_finished = false;

	std::thread draw_thread(start_draw_thread, draw_window, &draw_finished);
	std::thread compute_thread(start_compute_thread, compute_window, &compute_finished);

	while(!draw_finished && !compute_finished) {
		glfwPollEvents();
	}

	draw_finished = true;
	compute_finished = true;

    glfwDestroyWindow(compute_window);
    glfwDestroyWindow(draw_window);

    compute_thread.join();
    draw_thread.join();

    glfwTerminate();
}
