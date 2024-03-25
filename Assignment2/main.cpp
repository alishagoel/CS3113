/**
* Author: Alisha Goel
* Assignment: Pong Clone
* Date due: 2024-03-02, 11:59pm
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
const float MINIMUM_COLLISION_DISTANCE = 1.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char HAYSTACK_SPRITE_FILEPATH[] = "haystack.png";
const char TEDDY_SPRIRE_FILEPATH[] = "teddy.png";

SDL_Window* g_display_window;
bool g_game_is_running = true;

bool multiplayer_mode = true;

ShaderProgram g_shader_program;
glm::mat4 view_matrix, g_matrix_teddy, g_matrix_haystack1, g_matrix_haystack2, g_projection_matrix, g_trans_matrix;

float g_previous_ticks = 0.0f;
//float scaling_amount = 2.0f;
//float scaling_amount_teddy = 0.7f;

const glm::vec3 scaling_amount = glm::vec3(2.0f, 2.0f, 1.0f),
scaling_amount_teddy = glm::vec3(0.7f, 0.7f, 1.0f);

GLuint g_haystack_texture_id;
GLuint g_teddy_texture_id;

// movement tracker
glm::vec3 g_player_position = glm::vec3(0.0f, 0.0f, 0.0f);     //
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);     //
                                                               //
float g_player_speed = 5.0f;

glm::vec3 g_player_position2 = glm::vec3(0.0f, 0.0f, 0.0f);     //
glm::vec3 g_player_movement2 = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);     //
float g_ball_speed = 3.0f;
glm::vec3 g_movement_ball = glm::vec3(0.3f, 0.4f, 0.0f);
                                                               //

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
    // Initialise video
    SDL_Init(SDL_INIT_VIDEO);
    
    g_display_window = SDL_CreateWindow("PONG CLONE",
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
    
    g_matrix_teddy = glm::mat4(1.0f);
    g_matrix_haystack1 = glm::mat4(1.0f);
    g_matrix_haystack2 = glm::mat4(1.0f);
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(view_matrix);
    // Notice we haven't set our model matrix yet!
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_haystack_texture_id = load_texture(HAYSTACK_SPRITE_FILEPATH);
    g_teddy_texture_id = load_texture(TEDDY_SPRIRE_FILEPATH);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // –––––––––––––––––––––––––––––––– NEW STUFF –––––––––––––––––––––––––– //
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere   //
    g_player_movement = glm::vec3(0.0f);
    g_player_movement2 = glm::vec3(0.0f); //
                                                                             //
    // –––––––––––––––––––––––––––––––– KEYSTROKES ––––––––––––––––––––––––– //
                                                                             //
    SDL_Event event;                                                         //
    while (SDL_PollEvent(&event))                                            //
    {                                                                        //
        switch (event.type)                                                  //
        {                                                                    //
            // End game                                                      //
            case SDL_QUIT:                                                   //
            case SDL_WINDOWEVENT_CLOSE:                                      //
                g_game_is_running = false;                                   //
                break;                                                       //
                                                                             //
            case SDL_KEYDOWN:                                                //
                switch (event.key.keysym.sym)                                //
                {                                                            //
                    case SDLK_UP:                                            //
                        g_player_movement2.y = -1.0f;                        //
                        break;                                               //
                                                                             //
                    case SDLK_DOWN:                                          //
                        // Move the player right                             //
                        g_player_movement2.y = 1.0f;                         //
                        break;                                               //
                        
                    case SDLK_w:
                        if (multiplayer_mode){//
                            g_player_movement.y = -1.0f;                         //
                             }
                        break;//
                                                                             //
                    case SDLK_s:                                             //
                        // Move the player right                             //
                        if (multiplayer_mode){//
                            g_player_movement.y = 1.0f;                         //
                             }
                        break;
                                                                             //
                    case SDLK_q:                                             //
                        // Quit the game with a keystroke                    //
                        g_game_is_running = false;                           //
                        break;                                               //
                    
                    case SDLK_t:
                        multiplayer_mode = !multiplayer_mode;
                                                                             //
                    default:                                                 //
                        break;                                               //
                }                                                            //
                                                                             //
            default:                                                         //
                break;                                                       //
        }                                                                    //
    }                                                                        //
                                                                             //
    // ––––––––––––––––––––––––––––––– KEY HOLD –––––––––––––––––––––––––––– //
                                                                             //
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);                     //
                                                                             //
                                                                        //
    if (key_state[SDL_SCANCODE_UP])                                          //
    {                                                                        //
        g_player_movement2.y = 1.0f;                                          //
    }                                                                        //
    else if (key_state[SDL_SCANCODE_DOWN])                                   //
    {                                                                        //
        g_player_movement2.y = -1.0f;                                         //
    }
    
    if (key_state[SDL_SCANCODE_W] && multiplayer_mode)                                          //
    {                                                                        //
        g_player_movement.y = 1.0f;                                          //
    }                                                                        //
    else if (key_state[SDL_SCANCODE_S] && multiplayer_mode)                                   //
    {                                                                        //
        g_player_movement.y = -1.0f;                                         //
    } //
                                                                             //
    // This makes sure that the player can't "cheat" their way into moving   //
    // faster                                                                //
    if (glm::length(g_player_movement) > 1.0f)                               //
    {                                                                        //
        g_player_movement = glm::normalize(g_player_movement);               //
    }
    if (glm::length(g_player_movement2) > 1.0f)                               //
    {                                                                        //
        g_player_movement = glm::normalize(g_player_movement2);               //
    }//
    // ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //
    
    
}

bool walls(glm::vec3 &position)
{
    float height = 1.8f;
    float wall = height - 0.2f * scaling_amount.y;
    if (position.y <= -wall or position.y >= wall)
    { return true; }
    else { return false; }
}

bool walls_balls(glm::vec3 &position)
{
    float height = 5.0f;
    float wall = height - 0.2f * scaling_amount.y;
    if (position.y <= -wall or position.y >= wall)
    { return true; }
    else { return false; }
}

bool collide(glm::vec3 &position_a, glm::vec3 &position_b)
{
    float x_distance = fabs(position_a.x - position_b.x) - ((scaling_amount.x + scaling_amount_teddy.x) / 2.0f);
    float y_distance = fabs(position_a.y - position_b.y) - ((scaling_amount.y + scaling_amount_teddy.y) / 2.0f);
    
    if (x_distance <= 0 && y_distance <= 0)
    { return true; }
    else { return false; }
}


void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    // Add             direction       * elapsed time * units per second
    g_matrix_teddy= glm::mat4(1.0f);
    g_matrix_haystack1 = glm::mat4(1.0f);
    g_matrix_haystack2 =glm::mat4(1.0f);
    g_matrix_haystack1 = glm::scale(g_matrix_haystack1, glm::vec3(scaling_amount.x, scaling_amount.y, 1.0f));
    g_matrix_haystack2 = glm::scale(g_matrix_haystack2, glm::vec3(scaling_amount.x, scaling_amount.y, 1.0f));
    g_matrix_teddy = glm::scale(g_matrix_teddy, glm::vec3(scaling_amount_teddy.x, scaling_amount_teddy.y, 1.0f));
    g_matrix_haystack1 = glm::translate(g_matrix_haystack1, glm::vec3(2.3f, 0.0f, 0.0f));
    g_matrix_haystack2 = glm::translate(g_matrix_haystack2, glm::vec3(-2.3f, 0.0f, 0.0f));
    g_player_position += g_player_movement * g_player_speed * delta_time;
    g_player_position2 += g_player_movement2 * g_player_speed * delta_time;
    
    
    g_matrix_haystack1 = glm::translate(g_matrix_haystack1, g_player_position2);
    g_matrix_haystack2 = glm::translate(g_matrix_haystack2, g_player_position);
    g_matrix_haystack2 = glm::rotate(g_matrix_haystack2, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    if (walls(g_player_position)){
        g_player_position -= g_player_movement * g_player_speed * delta_time;
    }
    
    if (walls(g_player_position2)){
        g_player_position2 -= g_player_movement2 * g_player_speed * delta_time;
        
        
    }
    
    g_ball_position += g_movement_ball * g_ball_speed * delta_time;
    
    if (collide(g_ball_position, g_player_position) ||
        collide(g_ball_position, g_player_position2))
            {
                g_ball_position -= g_movement_ball * g_ball_speed * delta_time;
                g_movement_ball.x = -g_movement_ball.x;
                g_ball_position += g_movement_ball * g_ball_speed * delta_time;
            }
    else if (walls_balls(g_ball_position))
            {
                g_ball_position -= g_movement_ball * g_ball_speed * delta_time;
                g_movement_ball.y = -g_movement_ball.y;
                g_ball_position += g_movement_ball * g_ball_speed * delta_time;
            }

    g_matrix_teddy = glm::translate(g_matrix_teddy, g_ball_position);
    

    
    
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
    draw_object(g_matrix_teddy, g_teddy_texture_id);
    
    draw_object(g_matrix_haystack1, g_haystack_texture_id);
    
    draw_object(g_matrix_haystack2, g_haystack_texture_id);
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    
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
