/**
* Author: Alisha Goel
* Assignment: Simple 2D Scene
* Date due: 2024-02-20, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum Coordinate
{
    x_coordinate,
    y_coordinate
};

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640 * 2,
          WINDOW_HEIGHT = 480 * 2;

const float BG_RED = 0.1922f,
            BG_BLUE = 0.7008f,
            BG_GREEN = 0.9500f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char AIRPLANE_SPRITE_FILEPATH[] = "airplane.png";
const char SUN_SPRIRE_FILEPATH[] = "sun.png";

SDL_Window* g_display_window;
bool g_game_is_running = true;
bool g_is_growing = true;

ShaderProgram g_shader_program;
glm::mat4 view_matrix, g_model_marix, g_model_marix2, g_projection_matrix, g_trans_matrix;

float g_previous_ticks = 0.0f;
float g_player_rotate = 0.0f;
float player_mov = 0.0f;
float player_mov2 = 0.0f;
float airplane_mov = 0.0f;
float airplane_mov2 = 0.0f;
float scaling_amount = 1.0f;

GLuint g_player_texture_id;
GLuint g_sun_texture_id;
SDL_Joystick *g_player_one_controller;

// movement tracker
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

float get_screen_to_ortho(float coordinate, Coordinate axis)
{
    switch (axis) {
        case x_coordinate:
            return ((coordinate / WINDOW_WIDTH) * 10.0f ) - (10.0f / 2.0f);
        case y_coordinate:
            return (((WINDOW_HEIGHT - coordinate) / WINDOW_HEIGHT) * 7.5f) - (7.5f / 2.0);
        default:
            return 0.0f;
    }
}

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    
    // Open the first controller found. Returns null on error
    g_player_one_controller = SDL_JoystickOpen(0);
    
    g_display_window = SDL_CreateWindow("Assignment 1",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_model_marix = glm::mat4(1.0f);
    g_model_marix2 = glm::mat4(1.0f);
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(view_matrix);
    // Notice we haven't set our model matrix yet!
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_player_texture_id = load_texture(AIRPLANE_SPRITE_FILEPATH);
    g_sun_texture_id = load_texture(SUN_SPRIRE_FILEPATH);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_player_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_WINDOWEVENT_CLOSE:
            case SDL_QUIT:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT:
                        g_player_movement.x = 1.0f;
                        break;
                    case SDLK_LEFT:
                        g_player_movement.x = -1.0f;
                        break;
                    case SDLK_q:
                        g_game_is_running = false;
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }
    
    const Uint8 *key_states = SDL_GetKeyboardState(NULL); // array of key states [0, 0, 1, 0, 0, ...]
    
    if (key_states[SDL_SCANCODE_LEFT])
    {
        g_player_movement.x = -1.0f;
    } else if (key_states[SDL_SCANCODE_RIGHT])
    {
        g_player_movement.x = 1.0f;
    }
    
    if (key_states[SDL_SCANCODE_UP])
    {
        g_player_movement.y = 1.0f;
    } else if (key_states[SDL_SCANCODE_DOWN])
    {
        g_player_movement.y = -1.0f;
    }
    
    if (glm::length(g_player_movement) > 1.0f)
    {
        g_player_movement = glm::normalize(g_player_movement);
    }
}

void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    // Add             direction       * elapsed time * units per second
    player_mov += delta_time * 0.2f;
    player_mov2 += delta_time * 0.2f;
    g_player_rotate += DEGREES_PER_SECOND * delta_time;
    airplane_mov += delta_time * 0.1f;
    airplane_mov2 += delta_time * 0.1f;
    scaling_amount += delta_time * 0.1f;
    
    g_model_marix = glm::mat4(1.0f);
    g_model_marix2 = glm::mat4(1.0f);
    
    g_model_marix2 = glm::translate(g_model_marix2, glm::vec3(airplane_mov, airplane_mov2, 0.0f));
    g_model_marix = glm::translate(g_model_marix, glm::vec3(player_mov, player_mov2, 0.0f));
    g_model_marix = glm::rotate(g_model_marix, glm::radians(g_player_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    g_model_marix = glm::scale(g_model_marix, glm::vec3(scaling_amount, scaling_amount, 1.0f));

}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    // Bind texture
    draw_object(g_model_marix, g_sun_texture_id);
    
    draw_object(g_model_marix2, g_player_texture_id);
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_JoystickClose(g_player_one_controller);
    SDL_Quit();
}

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
