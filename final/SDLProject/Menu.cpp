#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#include "Menu.h"
#include "Utility.h"
#include "glm/glm.hpp"
#include "ShaderProgram.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

constexpr char MENU_IMAGE_PATH[] = "assets/menu.png";

Menu::Menu() {}

Menu::~Menu() {
}

void Menu::initialise() {
    m_game_state.next_scene_id = -1;
    m_game_state.player = nullptr;
    m_game_state.enemies = nullptr;
    m_menu_texture_id= Utility::load_texture(MENU_IMAGE_PATH);
    
}
    
void Menu::update(float delta_time) {
    }
    
void Menu::render(ShaderProgram *program) {

    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    float tex_coords[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };
    
    glBindTexture(GL_TEXTURE_2D, m_menu_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());

    m_game_state.next_scene_id = 1;
}
