/**
* Author: Joori  Bin  Jardan
* Assignment: Platformer
* Date due: 2023-11-26, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

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
#include "Menu.h"
#include  "string"
#include "LevelC.h"


// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 640 * 2,
          WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED = 250 / 255.0f,
BG_BLUE = 222 / 255.0f,
BG_GREEN = 202 / 255.0f,
BG_OPACITY = 1.0f;



constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";


constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

// ————— GLOBAL VARIABLES ————— //
Scene* g_current_scene;
LevelA* g_levelA;
LevelB* g_levelB;
LevelC* g_levelC;
Menu* g_menu;
Scene* g_levels[4];

int g_is_dead = 0;


GLuint g_font_texture_id3;

SDL_Window* g_display_window;

int curr_player_lives = 3;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise();
}

void initialise();
void process_input();
void update();
void render();
void shutdown();


void initialise()
{
    // ————— VIDEO ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Hello, Scenes!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        shutdown();
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ————— GENERAL ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    
    g_font_texture_id3 = Utility::load_texture("assets/font1.png");
    
    // ————— LEVELS ————— //
    g_menu = new Menu();
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();

    g_levels[0] = g_menu;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;


    // Start at level A
    switch_to_scene(g_levels[0]);


    
    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // ————— KEYSTROKES ————— //
        switch (event.type) {
            // ————— END GAME ————— //
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                        
                    //case SDLK_SPACE:
                    //    // ————— JUMPING ————— //
                    //    if (g_current_scene->get_state().player->get_collided_bottom())
                    //    {
                    //        g_current_scene->get_state().player->jump();
                    //        Mix_PlayChannel(-1,  g_current_scene->get_state().jump_sfx, 0);
                    //    }
                    //     break;
                    case SDLK_RETURN:
                        g_menu->next_level = true;
                        break;
                    case SDLK_LSHIFT :
                        g_current_scene->get_state().player->shoot();
                        Mix_PlayChannel(-1,  g_current_scene->get_state().shoot_sfx, 0);

                        break;
                    case (SDLK_SPACE): // Drop a bomb
                        //bomb each 10 kills
                        if (g_current_scene == g_levelA) {
                            if (g_current_scene->get_state().player->bombs_counter > 0) {
                                    g_current_scene->get_state().player->drop_bomb();
                                }
                        }
                        Mix_PlayChannel(-1, g_current_scene->get_state().bomb_sfx, 0);

                            break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    // ————— KEY HOLD ————— //
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_A])        g_current_scene->get_state().player->move_left();
    else if (key_state[SDL_SCANCODE_D])  g_current_scene->get_state().player->move_right();

    if (key_state[SDL_SCANCODE_W])        g_current_scene->get_state().player->move_up();
    else if (key_state[SDL_SCANCODE_S])  g_current_scene->get_state().player->move_down();
     

    if (glm::length( g_current_scene->get_state().player->get_movement()) > 1.0f)
        g_current_scene->get_state().player->normalise_movement();
 
}

void update()
{
    if (g_current_scene->get_player_lives() == 0 && g_is_dead!=1) {
        g_is_dead = 1;
        Mix_PlayChannel(-1, g_current_scene->get_state().die_sfx, 0);

    }
    g_shader_program.set_is_dead(g_is_dead);

    // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
        g_current_scene->update(FIXED_TIMESTEP);
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    
    // ————— PLAYER CAMERA ————— //
    //if (g_current_scene->get_state().player->get_position().x > LEVEL1_LEFT_EDGE) {
    //    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
    //}
    //else {
    //    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
    //}\
    
    if (g_current_scene != g_menu) {
        g_view_matrix = glm::mat4(1.0f);
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 4.75, 0));
    }
    
    



    if (g_current_scene == g_menu && g_current_scene->next_level == true) switch_to_scene(g_levelA);
    if (g_current_scene == g_levelA && g_current_scene->next_level) switch_to_scene(g_levelB);
    if (g_current_scene == g_levelB && g_current_scene->next_level) switch_to_scene(g_levelC);
    if (g_current_scene == g_levelC && g_current_scene->next_level)  g_current_scene->get_state().player->levels_passed = true;



}

void render()
{

    g_shader_program.set_view_matrix(g_view_matrix);
    
    glClear(GL_COLOR_BUFFER_BIT);
    std::string player_lives_str = "LIVES:" + std::to_string(g_current_scene->get_player_lives());
    std::string bombs_count_str = "BOMBS:" + std::to_string(g_current_scene->get_state().player->bombs_counter);
    if (g_current_scene == g_levelA) {
        Utility::draw_text(&g_shader_program, g_font_texture_id3, bombs_count_str, 0.2f, 0.00001f,
            glm::vec3(0.75f, -1.75f, 0.0f));
    }
    if (g_current_scene->get_player_lives() > 0 && !g_current_scene->get_state().player->levels_passed && g_current_scene!=g_menu) {
        Utility::draw_text(&g_shader_program, g_font_texture_id3, player_lives_str, 0.2f, 0.00001f,
            glm::vec3(g_current_scene->get_state().player->get_position().x - 0.5f, g_current_scene->get_state().player->get_position().y + 0.25f, 0.0f));
    }
    else
    if(g_current_scene->get_player_lives() <= 0 && !g_current_scene->get_state().player->levels_passed){
        Utility::draw_text(&g_shader_program, g_font_texture_id3, "YOU DIED!", 0.5f, 0.05f,
            glm::vec3(3.0f, -4.0f, 0.0f));
    }
    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{    
    SDL_Quit();
    
    // ————— DELETING LEVEL A DATA (i.e. map, character, enemies...) ————— //
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    delete g_menu;


}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        if (g_current_scene->get_state().next_scene_id > 0) switch_to_scene(g_levels[g_current_scene->get_state().next_scene_id]);
        render();
    }
    
    shutdown();
    return 0;
}
