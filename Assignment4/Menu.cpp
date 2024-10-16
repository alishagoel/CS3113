#include "Menu.h"
#include "Utility.h"

constexpr char MENU_IMAGE_PATH[] = "assets/menu.png";

Menu::Menu() {}

Menu::~Menu() {}

void Menu::initialise() {
    m_game_state.next_scene_id = -1;
    m_game_state.player = nullptr;
    m_game_state.enemies = nullptr;
}
    
void Menu::update(float delta_time) {
    }
    
    void Menu::render(ShaderProgram *program) {
        glClear(GL_COLOR_BUFFER_BIT); // Clear the screen before rendering the new frame
        
        GLuint menu_texture_id = Utility::load_texture(MENU_IMAGE_PATH);
        glBindTexture(GL_TEXTURE_2D, menu_texture_id);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
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
        
        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());
        
        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
        
        glDrawArrays(GL_QUADS, 0, 4);
        
        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
        
        m_game_state.next_scene_id = 1;
        
}
    
