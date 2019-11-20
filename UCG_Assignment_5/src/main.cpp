// This example is heavily based on the tutorial at https://open.gl

////////////////////////////////////////////////////////////////////////////////
// OpenGL Helpers to reduce the clutter
#include "helpers.h"
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
// Linear Algebra Library
#include <Eigen/Dense>
#include <Eigen/LU>
// Timer
#include <chrono>
#include <thread>
////////////////////////////////////////////////////////////////////////////////

// VertexBufferObject wrapper
VertexBufferObject VBO;
//VAO
VertexArrayObject VAO;
//OpenGL Program
Program program;

// Contains the vertex positions
Eigen::Matrix<float, 6, 30> V;

Eigen::Matrix<float, 3, 3> mat_Transform = Eigen::MatrixXf::Identity(3, 3);

Eigen::Matrix<float, 3, 3> mat_View = Eigen::MatrixXf::Identity(3, 3);

Eigen::Matrix<float, 2, 30> Key_frame_pos;

//store triangle coordinates for later calculatios
std::vector<Eigen::Vector2d> Triangles;

static int vert_count = 0;
static int num_Triangles = 0;

//key to enable/disable insert mode
bool Key_i = false;

bool triangle_selected = false;
int triangle_selected_index = -1;
float shift_x, current_x;
float shift_y, current_y;
Eigen::Vector3f color;
bool color_change = false;
bool mouse_move_flag = false;
bool apply_shader_translation = false;
int closer_vertex = -1;
int key_frames_count = 0;
int selected_triangle_animation = -1;
bool animation_on = false;

//forward declairations
void findselectedtriangle(double x, double y);
void removeselectedtriangle();

void init()
{
    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();

    V.resize(6, 30);
    VBO.update(V);

    //initialize view matrix
    Eigen::Vector3f camdirection = Eigen::Vector3f(0, 0, 1);
    Eigen::Vector3f up = Eigen::Vector3f(0.0, 1.0, 0.0);
    Eigen::Vector3f camrigh = up.cross(camdirection);
    Eigen::Vector3f camup = camdirection.cross(camrigh);

    mat_View.col(0) << camrigh[0],camrigh[1], 0.1;
    mat_View.col(1) << camup[0],camup[1], 0.1;
    mat_View.col(2) << camdirection[0],camdirection[1],1;

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    const GLchar* vertex_shader = R"(
        #version 150 core

        in vec3 position;
        uniform float shift_x;
        uniform float shift_y;
        uniform mat3 Translation;
        uniform mat3 viewMatrix;
        in vec3 triangleColor;
        out vec3 o_color;

        void main() {
            vec3 T_position = viewMatrix * Translation * position;
            gl_Position = vec4(T_position[0] - shift_x, T_position[1] - shift_y, 0.0, 1.0);
            o_color = triangleColor;
        }
    )";

    const GLchar* fragment_shader = R"(
        #version 150 core

        in vec3 o_color;
        out vec4 outColor;

        void main() {
            outColor = vec4(o_color, 0.0);
        }
    )";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader, fragment_shader, "outColor");
    program.bind();

    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    program.bindVertexAttribArray("position","triangleColor", VBO);
}

void draw_triangle(GLFWwindow* window)
{
    // Set the size of the viewport (canvas) to the size of the application window (framebuffer)
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glLineWidth(1);

    // Bind your VAO (not necessary if you have only one)
    VAO.bind();

    // Bind your program
    program.bind();

    // Clear the framebuffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (apply_shader_translation)
    {
        mat_Transform.col(0) << 2.0, 0, 0;
    }
    else
    {
        mat_Transform.col(0) << 1.0, 0, 0;
    }

    glUniformMatrix3fv(program.uniform("Translation"), 1, false, &mat_Transform(0,0));
    glUniformMatrix3fv(program.uniform("viewMatrix"), 1, false, &mat_View(0,0));

    // Draw a triangle
    if (vert_count != 0) {
        for (int i = 0; i <= num_Triangles; i++) {
            if ((i == triangle_selected_index) && triangle_selected)
            {
                V.col((i * 3) + 0) << V.coeff(0, (i * 3) + 0), V.coeff(1, (i * 3) + 0), V.coeff(2, (i * 3) + 0), 0.0, 0.0, 1.0;
                V.col((i * 3) + 1) << V.coeff(0, (i * 3) + 1), V.coeff(1, (i * 3) + 1), V.coeff(2, (i * 3) + 1), 0.0, 0.0, 1.0;
                V.col((i * 3) + 2) << V.coeff(0, (i * 3) + 2), V.coeff(1, (i * 3) + 2), V.coeff(2, (i * 3) + 2), 0.0, 0.0, 1.0;
                VBO.update(V);
                glUniform1f(program.uniform("shift_x"), 0.0);
                glUniform1f(program.uniform("shift_y"), 0.0);

            }
            else if ((i == triangle_selected_index) && color_change && (closer_vertex != -1))
            {
                V.col((i * 3) + closer_vertex) << V.coeff(0, (i * 3) + closer_vertex), V.coeff(1, (i * 3) + closer_vertex), V.coeff(1, (i * 3) + closer_vertex), color[0], color[1], color[2];
                VBO.update(V);
                glUniform1f(program.uniform("shift_x"), 0.0);
                glUniform1f(program.uniform("shift_y"), 0.0);
            }
            else
            {
                V.col((i * 3) + 0) << V.coeff(0, (i * 3) + 0), V.coeff(1, (i * 3) + 0), V.coeff(2, (i * 3) + 0), 1.0, 0.0, 0.0;
                V.col((i * 3) + 1) << V.coeff(0, (i * 3) + 1), V.coeff(1, (i * 3) + 1), V.coeff(2, (i * 3) + 1), 1.0, 0.0, 0.0;
                V.col((i * 3) + 2) << V.coeff(0, (i * 3) + 2), V.coeff(1, (i * 3) + 2), V.coeff(2, (i * 3) + 2), 1.0, 0.0, 0.0;
                VBO.update(V);
                glUniform1f(program.uniform("shift_x"), 0.0);
                glUniform1f(program.uniform("shift_y"), 0.0);
            }

            if (vert_count == i * 3 + 1)
            {
                glDrawArrays(GL_LINES, i * 3, 2);
            }
            else
            {
                glDrawArrays(GL_TRIANGLES, i * 3, 3);
            }
        }
    }
    // Swap front and back buffers
    glfwSwapBuffers(window);
}

void getWorldPos(GLFWwindow* window, double &x, double &y)
{
    // Get viewport size (canvas in number of pixels)
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // Get the size of the window (may be different than the canvas size on retina displays)
    int width_window, height_window;
    glfwGetWindowSize(window, &width_window, &height_window);

    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Deduce position of the mouse in the viewport
    double highdpi = (double)width / (double)width_window;
    xpos *= highdpi;
    ypos *= highdpi;

    // Convert screen position to world coordinates
    x = ((xpos / double(width)) * 2) - 1;
    y = (((height - 1 - ypos) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw
}

void mouse_curson_pos_callback(GLFWwindow* window, double x, double y)
{
    if (vert_count != 0 && Key_i && !triangle_selected)
    {
        // Get viewport size (canvas in number of pixels)
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Get the size of the window (may be different than the canvas size on retina displays)
        int width_window, height_window;
        glfwGetWindowSize(window, &width_window, &height_window);

        // Deduce position of the mouse in the viewport
        double highdpi = (double)width / (double)width_window;
        x *= highdpi;
        y *= highdpi;

        if (vert_count == num_Triangles * 3 + 1)
        {
            double w_x, w_y;
            // Convert screen position to world coordinates
            w_x = ((x / double(width)) * 2) - 1;
            w_y = (((height - 1 - y) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw
            Eigen::Vector3f Vin = Eigen::Vector3f(w_x, w_y, 0.0);
            Eigen::Vector3f Vout = mat_View.inverse() * Vin;
            V.col((num_Triangles * 3) + 1) << Vout[0], Vout[1], 1.0, 1.0, 0.0, 0.0;
            V.col((num_Triangles * 3) + 2) << Vout[0], Vout[1], 1.0, 1.0, 0.0, 0.0;
        }
        else if (vert_count == num_Triangles * 3 + 2)
        {
            double w_x, w_y;
            // Convert screen position to world coordinates
            w_x = ((x / double(width)) * 2) - 1;
            w_y = (((height - 1 - y) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw
            Eigen::Vector3f Vin = Eigen::Vector3f(w_x, w_y, 0.0);
            Eigen::Vector3f Vout = mat_View.inverse() * Vin;
            V.col((num_Triangles * 3) + 2) << Vout[0], Vout[1], 1.0, 1.0, 0.0, 0.0;
        }
        VBO.update(V);
    }
    else if (triangle_selected && (triangle_selected_index != -1) && mouse_move_flag)
    {
        // Get viewport size (canvas in number of pixels)
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Get the size of the window (may be different than the canvas size on retina displays)
        int width_window, height_window;
        glfwGetWindowSize(window, &width_window, &height_window);
        // Deduce position of the mouse in the viewport
        double highdpi = (double)width / (double)width_window;
        x *= highdpi;
        y *= highdpi;

        double w_x, w_y;
        // Convert screen position to world coordinates
        w_x = ((x / double(width)) * 2) - 1;
        w_y = (((height - 1 - y) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw

        shift_x = current_x - w_x;
        shift_y = current_y - w_y;

        current_x = w_x;
        current_y = w_y;

        int pos_0 = triangle_selected_index * 3 + 0;
        int pos_1 = triangle_selected_index * 3 + 1;
        int pos_2 = triangle_selected_index * 3 + 2;

        V.col(pos_0) << V.coeff(0, pos_0) - shift_x, V.coeff(1, pos_0) - shift_y, 1.0, 1.0, 0.0, 0.0;
        V.col(pos_1) << V.coeff(0, pos_1) - shift_x, V.coeff(1, pos_1) - shift_y, 1.0, 1.0, 0.0, 0.0;
        V.col(pos_2) << V.coeff(0, pos_2) - shift_x, V.coeff(1, pos_2) - shift_y, 1.0, 1.0, 0.0, 0.0;

        VBO.update(V);
    }
    draw_triangle(window);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

    double xworld, yworld;
    getWorldPos(window, xworld, yworld);

    // Update the position of the first vertex if the left button is pressed

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && Key_i) {
        if (vert_count == (num_Triangles * 3)) {
            Eigen::Vector3f Vin = Eigen::Vector3f(xworld, yworld, 0.0);
            Eigen::Vector3f Vout = mat_View.inverse() * Vin;
            V.col((num_Triangles * 3) + 0) << Vout[0], Vout[1], 1.0, 1.0, 0.0, 0.0;
            V.col((num_Triangles * 3) + 1) << Vout[0], Vout[1], 1.0, 1.0, 0.0, 0.0;
            V.col((num_Triangles * 3) + 2) << Vout[0], Vout[1], 1.0, 1.0, 0.0, 0.0;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && Key_i) {
        vert_count++;
        if (vert_count == (num_Triangles * 3) + 3)
        {
            num_Triangles++;
        }
    }
    // Upload the change to the GPU
    VBO.update(V);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && triangle_selected)
    {
        if(animation_on){
            Key_frame_pos.col(((key_frames_count-1) * 3) + 0)<<V.coeff(0, (triangle_selected_index * 3) + 0), V.coeff(1, (triangle_selected_index * 3) + 0);
            Key_frame_pos.col(((key_frames_count-1) * 3) + 1)<<V.coeff(0, (triangle_selected_index * 3) + 1), V.coeff(1, (triangle_selected_index * 3) + 1);
            Key_frame_pos.col(((key_frames_count-1) * 3) + 2)<<V.coeff(0, (triangle_selected_index * 3) + 2), V.coeff(1, (triangle_selected_index * 3) + 2);
        }
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        mouse_move_flag = true;
        mouse_curson_pos_callback(window, x, y);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && triangle_selected)
    {
        if(animation_on){
            Key_frame_pos.col(((key_frames_count) * 3) + 0)<<V.coeff(0, (triangle_selected_index * 3) + 0), V.coeff(1, (triangle_selected_index * 3) + 0);
            Key_frame_pos.col(((key_frames_count) * 3) + 1)<<V.coeff(0, (triangle_selected_index * 3) + 1), V.coeff(1, (triangle_selected_index * 3) + 1);
            Key_frame_pos.col(((key_frames_count) * 3) + 2)<<V.coeff(0, (triangle_selected_index * 3) + 2), V.coeff(1, (triangle_selected_index * 3) + 2);
        }
        triangle_selected = false;
        mouse_move_flag = false;
        triangle_selected_index = -1;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && color_change)
    {
        triangle_selected_index = -1;
        findselectedtriangle(xworld, yworld);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    switch (key) {
    case GLFW_KEY_I:
        //triangle insertion mode
        if (Key_i && action == GLFW_RELEASE)
        {
            Key_i = false;
        }
        else if (!Key_i && action == GLFW_RELEASE)
        {
            Key_i = true;
        }
        break;
    case GLFW_KEY_O:
        // triangle selection and translation mode
        if (!triangle_selected && action == GLFW_PRESS)
        {
            triangle_selected = true;
            triangle_selected_index = -1;
            double x, y;
            getWorldPos(window, x, y);
            findselectedtriangle(x, y);
            current_x = x;
            current_y = y;
        }
        else if (triangle_selected && action == GLFW_RELEASE)
        {
            //triangle_selected_index = -1;
        }
        break;
    case GLFW_KEY_P:
        //enable delete mode
        if (action == GLFW_PRESS)
        {
            double x, y;
            getWorldPos(window, x, y);
            triangle_selected_index = -1;
            findselectedtriangle(x, y);
            removeselectedtriangle();
        }
        else
        {
            triangle_selected_index = -1;
        }
        break;
    case GLFW_KEY_H:
        //triangle rotate mode
        if (action == GLFW_PRESS)
        {
            for (int i = 0; i <= num_Triangles; i++)
            {
                int pos_0 = i * 3 + 0;
                int pos_1 = i * 3 + 1;
                int pos_2 = i * 3 + 2;

                double center_x = (V.coeff(0, pos_0) + V.coeff(0, pos_1) + V.coeff(0, pos_2)) / 3;
                double center_y = (V.coeff(1, pos_0) + V.coeff(1, pos_1) + V.coeff(1, pos_2)) / 3;

                const float pi = 3.142;
                const float theta = (10 * 3.142) / 180;

                double r_x0 = (V.coeff(0, pos_0) - center_x) * std::cos(theta) - (V.coeff(1, pos_0) - center_y) * std::sin(theta);
                double r_y0 = (V.coeff(0, pos_0) - center_x) * std::sin(theta) + (V.coeff(1, pos_0) - center_y) * std::cos(theta);

                double r_x1 = (V.coeff(0, pos_1) - center_x) * std::cos(theta) - (V.coeff(1, pos_1) - center_y) * std::sin(theta);
                double r_y1 = (V.coeff(0, pos_1) - center_x) * std::sin(theta) + (V.coeff(1, pos_1) - center_y) * std::cos(theta);

                double r_x2 = (V.coeff(0, pos_2) - center_x) * std::cos(theta) - (V.coeff(0, pos_2) - center_x) * std::sin(theta);
                double r_y2 = (V.coeff(0, pos_2) - center_x) * std::sin(theta) + (V.coeff(0, pos_2) - center_x) * std::cos(theta);

                V.col(pos_0) << center_x + r_x0, center_y + r_y0, 1.0, 1.0, 0.0, 0.0;
                V.col(pos_1) << center_x + r_x1, center_y + r_y1, 1.0, 1.0, 0.0, 0.0;
                V.col(pos_2) << center_x + r_x2, center_y + r_y2, 1.0, 1.0, 0.0, 0.0;
            }
            VBO.update(V);
        }
        break;
    case GLFW_KEY_J:
        // triangle rotate mode
        //triangle rotate mode
        if (action == GLFW_PRESS)
        {
            for (int i = 0; i <= num_Triangles; i++)
            {
                int pos_0 = i * 3 + 0;
                int pos_1 = i * 3 + 1;
                int pos_2 = i * 3 + 2;

                double center_x = (V.coeff(0, pos_0) + V.coeff(0, pos_1) + V.coeff(0, pos_2)) / 3;
                double center_y = (V.coeff(1, pos_0) + V.coeff(1, pos_1) + V.coeff(1, pos_2)) / 3;

                const float pi = 3.142;
                const float theta = (-10 * 3.142) / 180;

                double r_x0 = (V.coeff(0, pos_0) - center_x) * std::cos(theta) - (V.coeff(1, pos_0) - center_y) * std::sin(theta);
                double r_y0 = (V.coeff(0, pos_0) - center_x) * std::sin(theta) + (V.coeff(1, pos_0) - center_y) * std::cos(theta);

                double r_x1 = (V.coeff(0, pos_1) - center_x) * std::cos(theta) - (V.coeff(1, pos_1) - center_y) * std::sin(theta);
                double r_y1 = (V.coeff(0, pos_1) - center_x) * std::sin(theta) + (V.coeff(1, pos_1) - center_y) * std::cos(theta);

                double r_x2 = (V.coeff(0, pos_2) - center_x) * std::cos(theta) - (V.coeff(0, pos_2) - center_x) * std::sin(theta);
                double r_y2 = (V.coeff(0, pos_2) - center_x) * std::sin(theta) + (V.coeff(0, pos_2) - center_x) * std::cos(theta);

                V.col(pos_0) << center_x + r_x0, center_y + r_y0, 1.0, 1.0, 0.0, 0.0;
                V.col(pos_1) << center_x + r_x1, center_y + r_y1, 1.0, 1.0, 0.0, 0.0;
                V.col(pos_2) << center_x + r_x2, center_y + r_y2, 1.0, 1.0, 0.0, 0.0;
            }
            VBO.update(V);
        }
        break;
    case GLFW_KEY_K:
        //enable scale mode
        if (action == GLFW_PRESS)
        {
            for (int i = 0; i <= num_Triangles; i++)
            {
                int pos_0 = i * 3 + 0;
                int pos_1 = i * 3 + 1;
                int pos_2 = i * 3 + 2;

                double center_x = (V.coeff(0, pos_0) + V.coeff(0, pos_1) + V.coeff(0, pos_2)) / 3;
                double center_y = (V.coeff(1, pos_0) + V.coeff(1, pos_1) + V.coeff(1, pos_2)) / 3;

                V.col(pos_0) << V.coeff(0, pos_0) + (V.coeff(0, pos_0) - center_x)* 0.25, V.coeff(1, pos_0) + (V.coeff(1, pos_0) - center_y)* 0.25, 1.0, 1.0, 0.0, 0.0;
                V.col(pos_1) << V.coeff(0, pos_1) + (V.coeff(0, pos_1) - center_x)* 0.25, V.coeff(1, pos_1) + (V.coeff(1, pos_1) - center_y)* 0.25, 1.0, 1.0, 0.0, 0.0;
                V.col(pos_2) << V.coeff(0, pos_2) + (V.coeff(0, pos_2) - center_x)* 0.25, V.coeff(1, pos_2) + (V.coeff(1, pos_2) - center_y)* 0.25, 1.0, 1.0, 0.0, 0.0;
            }
            VBO.update(V);
        }
        break;
    case GLFW_KEY_L:
        //enable scale mode
        if (action == GLFW_PRESS)
        {
            for (int i = 0; i <= num_Triangles; i++)
            {
                int pos_0 = i * 3 + 0;
                int pos_1 = i * 3 + 1;
                int pos_2 = i * 3 + 2;

                double center_x = (V.coeff(0, pos_0) + V.coeff(0, pos_1) + V.coeff(0, pos_2)) / 3;
                double center_y = (V.coeff(1, pos_0) + V.coeff(1, pos_1) + V.coeff(1, pos_2)) / 3;

                V.col(pos_0) << V.coeff(0, pos_0) - (V.coeff(0, pos_0) - center_x)* 0.25, V.coeff(1, pos_0) - (V.coeff(1, pos_0) - center_y)* 0.25, 1.0, 1.0, 0.0, 0.0;
                V.col(pos_1) << V.coeff(0, pos_1) - (V.coeff(0, pos_1) - center_x)* 0.25, V.coeff(1, pos_1) - (V.coeff(1, pos_1) - center_y)* 0.25, 1.0, 1.0, 0.0, 0.0;
                V.col(pos_2) << V.coeff(0, pos_2) - (V.coeff(0, pos_2) - center_x)* 0.25, V.coeff(1, pos_2) - (V.coeff(1, pos_2) - center_y)* 0.25, 1.0, 1.0, 0.0, 0.0;
            }
            VBO.update(V);
        }
        break;
    case GLFW_KEY_C:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color_change = false;
            closer_vertex = -1;
        }
        else if (!color_change && action == GLFW_RELEASE)
        {
            color_change = true;
        }
    case GLFW_KEY_1:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
        }
        break;
    case GLFW_KEY_2:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color = Eigen::Vector3f(1.0f, 1.0f, 0.0f);
        }
        break;
    case GLFW_KEY_3:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color = Eigen::Vector3f(0.0f, 1.0f, 1.0f);
        }
        break;
    case GLFW_KEY_4:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color = Eigen::Vector3f(0.5f, 1.0f, 0.0f);
        }
        break;
    case GLFW_KEY_5:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color = Eigen::Vector3f(0.0f, 1.0f, 0.5f);
        }
        break;
    case GLFW_KEY_6:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color = Eigen::Vector3f(1.0f, 0.5f, 0.0f);
        }
        break;
    case GLFW_KEY_7:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color = Eigen::Vector3f(1.0f, 0.0f, 1.0f);
        }
        break;
    case GLFW_KEY_8:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color = Eigen::Vector3f(0.5f, 1.0f, 0.5f);
        }
        break;
    case GLFW_KEY_9:
        //enable color change mode
        if (color_change && action == GLFW_RELEASE)
        {
            color = Eigen::Vector3f(0.5f, 0.5f, 0.5f);
        }
        break;
    case GLFW_KEY_T:
        if (apply_shader_translation && action == GLFW_RELEASE)
        {
            apply_shader_translation = false;
        }
        else if (!apply_shader_translation && action == GLFW_RELEASE)
        {
            apply_shader_translation = true;
        }
        break;
    case GLFW_KEY_F:
        //key frames
        if(action == GLFW_PRESS)
        {
            //count the number of frames user wish to create.
            triangle_selected = true;
            triangle_selected_index = -1;
            double x, y;
            getWorldPos(window, x, y);
            findselectedtriangle(x, y);
            current_x = x;
            current_y = y;
            selected_triangle_animation = triangle_selected_index;
            key_frames_count++;
            animation_on = true;
        }
        break;
    case GLFW_KEY_N:
        //perform animation linear interpolation
        if(action == GLFW_PRESS)
        {
            //update current primitive postion by a constant factor(0.01) after every 33ms(30fps)
            for(int i = 0; i<key_frames_count; i++){
                int num_frames = -1;//devide the movement between two frames for smoothness.
                double x1_current = Key_frame_pos(0, ((i+1) * 3) + 0);
                double x2_current = Key_frame_pos(0, ((i+1) * 3) + 1);
                double x3_current = Key_frame_pos(0, ((i+1) * 3) + 2);
                double y1_current = Key_frame_pos(1, ((i+1) * 3) + 0);
                double y2_current = Key_frame_pos(1, ((i+1) * 3) + 1);
                double y3_current = Key_frame_pos(1, ((i+1) * 3) + 2);
                while((num_frames < 11)){
                    ++num_frames;
                    double x1 = Key_frame_pos(0, (i * 3) + 0)+((x1_current - (Key_frame_pos(0, (i * 3) + 0)))*(0.1 * num_frames));
                    double x2 = Key_frame_pos(0, (i * 3) + 1)+((x2_current - (Key_frame_pos(0, (i * 3) + 1)))*(0.1 * num_frames));
                    double x3 = Key_frame_pos(0, (i * 3) + 2)+((x3_current - (Key_frame_pos(0, (i * 3) + 2)))*(0.1 * num_frames));

                    double y1 = Key_frame_pos(1, (i * 3) + 0)+((y1_current - (Key_frame_pos(1, (i * 3) + 0)))*(0.1 * num_frames));
                    double y2 = Key_frame_pos(1, (i * 3) + 1)+((y2_current - (Key_frame_pos(1, (i * 3) + 1)))*(0.1 * num_frames));
                    double y3 = Key_frame_pos(1, (i * 3) + 2)+((y3_current - (Key_frame_pos(1, (i * 3) + 2)))*(0.1 * num_frames));

                    V.col((selected_triangle_animation * 3) + 0) << x1, y1, V.coeff(2, (selected_triangle_animation * 3) + 0), 0.0, 0.0, 1.0;
                    V.col((selected_triangle_animation * 3) + 1) << x2, y2, V.coeff(2, (selected_triangle_animation * 3) + 1), 0.0, 0.0, 1.0;
                    V.col((selected_triangle_animation * 3) + 2) << x3, y3, V.coeff(2, (selected_triangle_animation * 3) + 2), 0.0, 0.0, 1.0;
                    //wait for 33ms
                    std::this_thread::sleep_for(std::chrono::milliseconds(33));
                    VBO.update(V);
                    draw_triangle(window);
                }
            }
        }
        break;
    case GLFW_KEY_B:
        //perform animation Quadratic Bézier curves around a predefined fixed point.
        if (action == GLFW_PRESS)
        {
            const double pivot_x = 0.5;
            const double pivot_y = 0.5;
            for (int i = 0; i < key_frames_count; i++)
            {
                int num_frames = -1;//devide the movement between two frames for smoothness.
                double x1_current = Key_frame_pos(0, ((i + 1) * 3) + 0);
                double x2_current = Key_frame_pos(0, ((i + 1) * 3) + 1);
                double x3_current = Key_frame_pos(0, ((i + 1) * 3) + 2);
                double y1_current = Key_frame_pos(1, ((i + 1) * 3) + 0);
                double y2_current = Key_frame_pos(1, ((i + 1) * 3) + 1);
                double y3_current = Key_frame_pos(1, ((i + 1) * 3) + 2);

                while ((num_frames < 11)) {
                    ++num_frames;
                    float t = (0.1 * num_frames);
                    double A_x1 = (1 - t) * ((1 - t)*Key_frame_pos(0, (i * 3) + 0) + t * pivot_x);
                    double B_x1 = t*((1 - t)*pivot_x + t * Key_frame_pos(0, ((i + 1) * 3) + 0));
                    double x1 = A_x1 + B_x1;

                    double A_x2 = (1 - t) * ((1 - t)*Key_frame_pos(0, (i * 3) + 1) + t * pivot_x);
                    double B_x2 = t*((1 - t)*pivot_x + t * Key_frame_pos(0, ((i + 1) * 3) + 1));
                    double x2 = A_x2 + B_x2;

                    double A_x3 = (1 - t) * ((1 - t)*Key_frame_pos(0, (i * 3) + 2) + t * pivot_x);
                    double B_x3 = t*((1 - t)*pivot_x + t * Key_frame_pos(0, ((i + 1) * 3) + 2));
                    double x3 = A_x3 + B_x3;

                    double A_y1 = (1 - t) * ((1 - t)*Key_frame_pos(1, (i * 3) + 0) + t * pivot_y);
                    double B_y1 = t*((1 - t)*pivot_y + t * Key_frame_pos(1, ((i + 1) * 3) + 0));
                    double y1 = A_y1 + B_y1;

                    double A_y2 = (1 - t) * ((1 - t)*Key_frame_pos(1, (i * 3) + 1) + t * pivot_y);
                    double B_y2 = t*((1 - t)*pivot_y + t * Key_frame_pos(1, ((i + 1) * 3) + 1));
                    double y2 = A_y2 + B_y2;

                    double A_y3 = (1 - t) * ((1 - t)*Key_frame_pos(1, (i * 3) + 2) + t * pivot_y);
                    double B_y3 = t*((1 - t)*pivot_y + t * Key_frame_pos(1, ((i + 1) * 3) + 2));
                    double y3 = A_y3 + B_y3;

                    V.col((selected_triangle_animation * 3) + 0) << x1, y1, V.coeff(2, (selected_triangle_animation * 3) + 0), 0.0, 0.0, 1.0;
                    V.col((selected_triangle_animation * 3) + 1) << x2, y2, V.coeff(2, (selected_triangle_animation * 3) + 1), 0.0, 0.0, 1.0;
                    V.col((selected_triangle_animation * 3) + 2) << x3, y3, V.coeff(2, (selected_triangle_animation * 3) + 2), 0.0, 0.0, 1.0;
                    //wait for 33ms
                    std::this_thread::sleep_for(std::chrono::milliseconds(33));
                    VBO.update(V);
                    draw_triangle(window);
                }
            }
        }
        break;
    case GLFW_KEY_R:
        //reset animation
        if((key_frames_count != 0) && action == GLFW_PRESS)
        {
            key_frames_count = 0;
            V.col((selected_triangle_animation * 3) + 0) << Key_frame_pos(0, (selected_triangle_animation * 3) + 0), Key_frame_pos(1, (selected_triangle_animation * 3) + 0), V.coeff(2, (selected_triangle_animation * 3) + 0), 0.0, 0.0, 1.0;
            V.col((selected_triangle_animation * 3) + 1) << Key_frame_pos(0, (selected_triangle_animation * 3) + 1), Key_frame_pos(1, (selected_triangle_animation * 3) + 1), V.coeff(2, (selected_triangle_animation * 3) + 1), 0.0, 0.0, 1.0;
            V.col((selected_triangle_animation * 3) + 2) << Key_frame_pos(0, (selected_triangle_animation * 3) + 2), Key_frame_pos(1, (selected_triangle_animation * 3) + 2), V.coeff(2, (selected_triangle_animation * 3) + 2), 0.0, 0.0, 1.0;
            selected_triangle_animation = -1;
            animation_on = false;
        }
        break;
    case GLFW_KEY_KP_ADD:
        if(action == GLFW_PRESS)
        {
            Eigen::Matrix<float, 3, 3> camPos;
            Eigen::Vector3f camdirection = Eigen::Vector3f(0.0, 0.0, 1.0);
            Eigen::Vector3f up = Eigen::Vector3f(0.0, 1.1, 0.0);
            Eigen::Vector3f camrigh = up.cross(camdirection);
            Eigen::Vector3f camup = camdirection.cross(camrigh);

            camPos.col(0) << camrigh[0],camrigh[1],camrigh[2];
            camPos.col(1) << camup[0],camup[1],camup[2];
            camPos.col(2) << camdirection[0],camdirection[1],camdirection[2];
            mat_View = mat_View * camPos;
        }
        break;
    case GLFW_KEY_MINUS:
        if(action == GLFW_PRESS)
        {
            Eigen::Matrix<float, 3, 3> camPos;
            Eigen::Vector3f camdirection = Eigen::Vector3f(0.0, 0.0, 1.0);
            Eigen::Vector3f up = Eigen::Vector3f(0.0, 0.9, 0.0);
            Eigen::Vector3f camrigh = up.cross(camdirection);
            Eigen::Vector3f camup = camdirection.cross(camrigh);

            camPos.col(0) << camrigh[0],camrigh[1],camrigh[2];
            camPos.col(1) << camup[0],camup[1],camup[2];
            camPos.col(2) << camdirection[0],camdirection[1],camdirection[2];
            mat_View = mat_View * camPos;
        }
        break;
    case GLFW_KEY_W:
        if(action == GLFW_PRESS)
        {
            Eigen::Matrix<float, 3, 3> camPos;

            camPos.col(0) << 0,0, 0;
            camPos.col(1) << 0,0, 0;
            camPos.col(2) << 0,0.1, 0;
            mat_View = mat_View + camPos;
        }
        break;
    case GLFW_KEY_S:
        if(action == GLFW_PRESS)
        {
            Eigen::Matrix<float, 3, 3> camPos;

            camPos.col(0) << 0,0, 0;
            camPos.col(1) << 0,0, 0;
            camPos.col(2) << 0,-0.1, 0;

            mat_View =  mat_View + camPos;
        }
        break;
    case GLFW_KEY_A:
        if(action == GLFW_PRESS)
        {
            Eigen::Matrix<float, 3, 3> camPos;

            camPos.col(0) << 0,0, 0;
            camPos.col(1) << 0,0, 0;
            camPos.col(2) << -0.1,0, 0;

            mat_View =  mat_View + camPos;
        }
        break;
    case GLFW_KEY_D:
        if(action == GLFW_PRESS)
        {
            Eigen::Matrix<float, 3, 3> camPos;

            camPos.col(0) << 0,0, 0;
            camPos.col(1) << 0,0, 0;
            camPos.col(2) << 0.1,0, 0;

            mat_View =  mat_View + camPos;
        }
        break;
    default:
        break;
    }

    // Upload the change to the GPU
    VBO.update(V);
}

int main(void) {
    // Initialize the GLFW library
    if (!glfwInit()) {
        return -1;
    }

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    GLFWwindow * window = glfwCreateWindow(640, 480, "Interactive Trangle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Load OpenGL and its extensions
    if (!gladLoadGL()) {
        printf("Failed to load OpenGL and its extensions");
        return(-1);
    }
    printf("OpenGL Version %d.%d loaded", GLVersion.major, GLVersion.minor);

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    //Register the cursor positon callback
    glfwSetCursorPosCallback(window, mouse_curson_pos_callback);

    init();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {

        draw_triangle(window);
        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    VBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}

double dot(double x1, double y1, double x2, double y2) {
    return(x1*x2 + y1*y2);
}

void findselectedtriangle(double x, double y)
{
    for (int i = 0; i < num_Triangles; i++)
    {
        int pos_0 = i * 3 + 0;
        int pos_1 = i * 3 + 1;
        int pos_2 = i * 3 + 2;

        double v0_x = V.coeff(0, pos_2) - V.coeff(0, pos_0);
        double v0_y = V.coeff(1, pos_2) - V.coeff(1, pos_0);

            double v1_x = V.coeff(0, pos_1) - V.coeff(0, pos_0);
        double v1_y = V.coeff(1, pos_1) - V.coeff(1, pos_0);

        double v2_x = x - V.coeff(0, pos_0);
        double v2_y = y - V.coeff(1, pos_0);

        double dot00 = dot(v0_x, v0_y, v0_x, v0_y);
        double dot01 = dot(v0_x, v0_y, v1_x, v1_y);
        double dot02 = dot(v0_x, v0_y, v2_x, v2_y);
        double dot11 = dot(v1_x, v1_y, v1_x, v1_y);
        double dot12 = dot(v1_x, v1_y, v2_x, v2_y);

        double invdenom = 1 / (dot00 * dot11 - dot01 * dot01);
        double u = (dot11 * dot02 - dot01 * dot12) * invdenom;
        double v = (dot00 * dot12 - dot01 * dot02) * invdenom;

        if ((u >= 0) && (v >= 0) && (u + v < 1))
        {
            triangle_selected_index = i;
        }
    }

    if (color_change)
    {
        int pos_0 = triangle_selected_index * 3 + 0;
        int pos_1 = triangle_selected_index * 3 + 1;
        int pos_2 = triangle_selected_index * 3 + 2;

        // Given the two points (x1, y1) and (x2, y2), the distance d between these points is given by the formula:
        // d = sqrt((x2-x1)^2 + (y2-y1)^2)
        double dist_1 = std::sqrt(std::pow((V.coeff(0, pos_0) - (x)),2) + std::pow(((V.coeff(1, pos_0)) - (y)), 2));
        double dist_2 = std::sqrt(std::pow((V.coeff(0, pos_1) - (x)), 2) + std::pow(((V.coeff(1, pos_1)) - (y)), 2));
        double dist_3 = std::sqrt(std::pow((V.coeff(0, pos_2) - (x)), 2) + std::pow(((V.coeff(1, pos_2)) - (y)), 2));

        double temp = dist_1;
        closer_vertex = 0;

        if (dist_2 < temp)
        {
            temp = dist_2;
            closer_vertex = 1;
        }
        if (dist_3 < temp)
        {
            temp = dist_3;
            closer_vertex = 2;
        }
    }
}

void removeselectedtriangle()
{
    int pos_0 = triangle_selected_index * 3 + 0;
    int pos_1 = triangle_selected_index * 3 + 1;
    int pos_2 = triangle_selected_index * 3 + 2;

    for (int i = triangle_selected + 1; i <= num_Triangles; i++)
    {
        int nextpos_0 = i * 3 + 0;
        int nextpos_1 = i * 3 + 1;
        int nextpos_2 = i * 3 + 2;
        V.col(pos_0) << V.coeff(0, nextpos_0), V.coeff(1, nextpos_0), 1.0, 1.0, 0.0, 0.0;
        V.col(pos_1) << V.coeff(0, nextpos_1), V.coeff(1, nextpos_1), 1.0, 1.0, 0.0, 0.0;
        V.col(pos_2) << V.coeff(0, nextpos_2), V.coeff(1, nextpos_2), 1.0, 1.0, 0.0, 0.0;
    }
    VBO.update(V);
}
