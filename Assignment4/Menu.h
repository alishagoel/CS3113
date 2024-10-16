#ifndef MENU_H
#define MENU_H

#include "Scene.h"

class Menu : public Scene {
public:
    Menu();
    ~Menu();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    GameState get_state() const;

private:
    GameState m_game_state;
};

#endif // MENU_H
