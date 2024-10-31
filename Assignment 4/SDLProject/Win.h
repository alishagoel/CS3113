#ifndef Win_H
#define Win_H

#include "Scene.h"

class Win : public Scene {
public:
    Win();
    ~Win();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    GameState get_state() const;

private:
    GameState m_game_state;
    GLuint m_win_texture_id;
};

#endif
