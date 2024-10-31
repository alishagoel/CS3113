#include "Lose.h"
#include "Utility.h"

constexpr char LOSE_IMAGE_PATH[] = "assets/lose.png";

Lose::Lose() {}

Lose::~Lose() {}

void Lose::initialise() {
    m_game_state.next_scene_id = -1;
    m_game_state.player = nullptr;
    m_game_state.enemies = nullptr;
    m_win_texture_id= Utility::load_texture(LOSE_IMAGE_PATH);
}
    
void Lose::update(float delta_time) {
    }
    
void Lose::render(ShaderProgram *program) {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen before rendering the new frame

    glBindTexture(GL_TEXTURE_2D, m_win_texture_id);

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

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());

    m_game_state.next_scene_id = 1;
}
