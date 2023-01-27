// CS4300 Assignment 1
// Turtle graphics using OpenGL and C++ created by Rafael Matthew Susanto Kwan
// The structure is based on HelloOpenGL1 by Amit Shesh

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;

#define PI 3.14159265

static const char *vertex_shader_text =
    "#version 330\n"
    "in vec2 vPosition;\n"
    "in vec3 vColor;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 modelview;\n"
    "out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projection * modelview * vec4(vPosition,0.0,1.0);\n"
    "    outColor = vec4(vColor,1);\n"
    "}\n";

static const char *fragment_shader_text =
    "#version 330\n"
    "in vec4 outColor;\n"
    "out vec4 fColor;\n"
    "void main()\n"
    "{\n"
    "    fColor = outColor;\n"
    "}\n";

static void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

struct State
{
    float x;
    float y;
    float Vx;
    float Vy;
};

vector<State> vertexData;
vector<State> stack;
float bounds[4];

void turtle(string filename)
{
    ifstream ifs(filename);
    if (!ifs.good())
    {
        fprintf(stderr, "Error: %s\n", "File does not exist.");
        exit(EXIT_SUCCESS);
    }
    int i = 0;
    string line;
    State current;

    while (getline(ifs, line))
    {
        string task;
        float tempVx, tempVy, num;
        istringstream iss(line);
        if (i < 1)
        {
            // get bounds and initiate current state
            iss >> bounds[0] >> bounds[1] >> bounds[2] >> bounds[3];
            current.x = 0;
            current.y = 0;
            current.Vx = 10000000; // the vector that draws all the images, when the value is higher, the image is more visible
            current.Vy = 0;
        }
        else
        {
            // commands and their numbers
            iss >> task >> num;
            if (task == "push")
            {
                stack.push_back(current);
            }
            else if (task == "pop")
            {
                current = stack[stack.size() - 1];
                stack.pop_back();
            }
            else if (task == "turn")
            {
                tempVx = (current.Vx - current.x) * cos(num * PI / 180) - (current.Vy - current.y) * sin(num * PI / 180);
                tempVy = (current.Vx - current.x) * sin(num * PI / 180) + (current.Vy - current.y) * cos(num * PI / 180);
                current.Vx = tempVx + current.x;
                current.Vy = tempVy + current.y;
            }
            else if (task == "trace")
            {
                vertexData.push_back(current);
                current.x = current.x + num * cos(atan2(current.Vy - current.y, current.Vx - current.x));
                current.y = current.y + num * sin(atan2(current.Vy - current.y, current.Vx - current.x));
                vertexData.push_back(current);
            }
            else if (task == "move")
            {
                current.x = current.x + num * cos(atan2(current.Vy - current.y, current.Vx - current.x));
                current.y = current.y + num * sin(atan2(current.Vy - current.y, current.Vx - current.x));
            }
            else
            {
                continue;
            }
        }
        i++;
    }

    // // vertex Data checking
    // for (auto &a : vertexData)
    // {
    //     cout << a.x << " "
    //          << a.y << " "
    //          << a.Vx << " "
    //          << a.Vy << endl;
    // }

    ifs.close();
}

// the command-line option checker is modified from stackoverflow by @iain

char *getOption(char **begin, char **end, const string &option)
{
    char **itr = find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool optionExists(char **begin, char **end, const string &option)
{
    return find(begin, end, option) != end;
}

int main(int argc, char *argv[])
{
    GLFWwindow *window;
    GLuint vertex_array, vertex_buffer, vertex_shader, fragment_shader, program;
    GLint modelview_location, projection_location, vpos_location;

    glfwSetErrorCallback(error_callback);

    if (optionExists(argv, argv + argc, "-i"))
    {
        char *input = getOption(argv, argv + argc, "-i");
        turtle(input);
    }
    else
    {
        cout << "write the input file name using option -i";
    }

    float *vertexDataAsFloats = new float[vertexData.size() * 2];

    int i = 0, j = 0;
    while (j < vertexData.size())
    {
        vertexDataAsFloats[i] = vertexData[j].x;
        vertexDataAsFloats[i + 1] = vertexData[j].y;
        i += 2;
        j++;
    }

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(500, 500, "Turtle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glUseProgram(program);

    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    // 5 floats per vertex (x,y,r,g,b) * 6 vertices
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * 2 * sizeof(float), vertexDataAsFloats, GL_STATIC_DRAW);

    modelview_location = glGetUniformLocation(program, "modelview");
    // printf("modelview: %d\n", modelview_location);
    projection_location = glGetUniformLocation(program, "projection");
    // printf("projection: %d\n", projection_location);
    vpos_location = glGetAttribLocation(program, "vPosition");
    // printf("vPosition: %d\n", vpos_location);

    float ratio;
    int width, height;
    glm::mat4 p, m;

    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);

    // prepare the projection matrix for orthographic projection
    p = glm::ortho(bounds[0], bounds[1], bounds[2], bounds[3]);
    // send projection matrix to GPU
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(p));

    // the vertex positions are 2 at a time
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), (void *)0);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        m = glm::mat4(1.0);
        // send modelview matrix to GPU
        glUniformMatrix4fv(modelview_location, 1, GL_FALSE, glm::value_ptr(m));

        glDrawArrays(GL_LINES, 0, vertexData.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    delete[] vertexDataAsFloats;
    exit(EXIT_SUCCESS);
}
