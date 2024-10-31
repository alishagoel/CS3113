#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "Effects.h"
#include "Menu.h"
#include "Win.h"
#include "Lose.h"

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.0f,
            BG_BLUE    = 0.0f,
            BG_GREEN   = 0.0f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

// ––––– GLOBAL VARIABLES ––––– //
Scene  *g_current_scene;
LevelA *g_levelA;
LevelB *g_levelB;
LevelC *g_levelC;
Menu   *gmenu;
Win   *gwin;
Lose  *glose;

Effects *g_effects;
Scene   *g_levels[6]; // Adjusted to three levels

SDL_Window* g_display_window;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;
float m_last_collision_time = 0.0f;
constexpr float m_collision_cooldown = 0.7f;
int LIVES = 3;

AppStatus g_app_status = RUNNING;

void switch_to_scene(Scene *scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise();
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Rogue Game",
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
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    gmenu = new Menu();
    gwin = new Win();
    glose = new Lose();
    
    g_levels[0] = gmenu;
        g_levels[1] = g_levelA;
        g_levels[2] = g_levelB;
        g_levels[3] = g_levelC;
        g_levels[4] = gwin;
        g_levels[5] = glose;
        
        switch_to_scene(g_levels[0]); // Start at menu
    
    g_effects = new Effects(g_projection_matrix, g_view_matrix);
    g_effects->start(SHRINK, 2.0f);
}

void process_input()
{
    if (g_current_scene != gmenu && g_current_scene != gwin && g_current_scene != glose) {
            g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
        }
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_SPACE:
                        if (g_current_scene != gmenu && g_current_scene != gwin && g_current_scene != glose && g_current_scene->get_state().player->get_collided_bottom()) {
                                                g_current_scene->get_state().player->jump();
                                                Mix_PlayChannel(-1, g_current_scene->get_state().jump_sfx, 0);
                            }
                                            break;
                    case SDLK_RETURN:
                            if (g_current_scene == gmenu) {
                                            switch_to_scene(g_levelA);
                            }
                            break;
                    
                        
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }
    
    if (g_current_scene != gmenu && g_current_scene != gwin && g_current_scene != glose){
        
        const Uint8 *key_state = SDL_GetKeyboardState(NULL);
        
        if (key_state[SDL_SCANCODE_LEFT])        g_current_scene->get_state().player->move_left();
        else if (key_state[SDL_SCANCODE_RIGHT])  g_current_scene->get_state().player->move_right();
        else if (key_state[SDL_SCANCODE_UP])  g_current_scene->get_state().player->move_up();
        else if (key_state[SDL_SCANCODE_DOWN])  g_current_scene->get_state().player->move_down();
        
        
        if (glm::length(g_current_scene->get_state().player->get_movement()) > 1.0f)
            g_current_scene->get_state().player->normalise_movement();
        
    }
}

void update()
{
    if (g_current_scene == gmenu && g_current_scene != gwin && g_current_scene != glose) return;
    
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP) {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
        g_effects->update(FIXED_TIMESTEP);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    g_view_matrix = glm::mat4(1.0f);
    
    if (g_current_scene->get_state().player->get_position().x > LEVEL1_LEFT_EDGE) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
    } else {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
    }
    if (g_current_scene->get_state().player->m_enemy_collision) {
            float current_time = SDL_GetTicks() / MILLISECONDS_IN_SECOND;

            if (current_time - m_last_collision_time >= m_collision_cooldown) {
                LIVES--;
                m_last_collision_time = current_time;
            }
            
            g_current_scene->get_state().player->m_enemy_collision = false;
        }
    
       
    if (g_current_scene == g_levelA && g_current_scene->get_state().player->get_position().x > 40.0f) switch_to_scene(g_levelB);
    if (g_current_scene == g_levelB && g_current_scene->get_state().player->get_position().x > 50.0f) switch_to_scene(g_levelC);
    
    if (LIVES == 0){
           switch_to_scene(glose);
        return;
           
       }
    if (LIVES>=1 && g_current_scene == g_levelC && g_current_scene->get_state().player->get_position().x > 50.0f){switch_to_scene(gwin);
           return;}

}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
       
    glClear(GL_COLOR_BUFFER_BIT);
       
    g_current_scene->render(&g_shader_program);
       
    g_effects->render();
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    delete g_effects;
    delete gmenu;
        delete gwin;
        delete glose;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
