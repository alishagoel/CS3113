#ifndef Lose_H
#define Lose_H

#include "Scene.h"

class Lose : public Scene {
public:
    Lose();
    ~Lose();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    GameState get_state() const;

private:
    GameState m_game_state;
    GLuint m_win_texture_id;
};

#endif
