#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH = 640 ,
WINDOW_HEIGHT = 480;

constexpr float BG_RED = 0.9765625f,
BG_GREEN = 0.97265625f,
BG_BLUE = 0.9609375f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
constexpr char PLAYER1_FILEPATH[] = "assets/towerSmall.png",
PLAYER2_FILEPATH[] = "assets/towerSmallAlt.png",
BALL_FILEPATH[] = "assets/moonFull.png",
BACKGROUND_FILEPATH[] = "assets/backgroundCastles.png",
FONT_FILEPATH[] = "assets/font1.png";


//FIX SCALE
constexpr glm::vec3 BACKGROUND_INIT_SCALE = glm::vec3(10.0f, 8.0f, 0.0f);
constexpr glm::vec3 PLAYER1_INIT_SCALE = glm::vec3((2.37f) * 0.25f, 3.37f * 0.25f, 0.0f);
constexpr glm::vec3 PLAYER2_INIT_SCALE = glm::vec3((2.37f) * 0.25f, 3.37f * 0.25f, 0.0f);
constexpr glm::vec3 BALL_INIT_SCALE = glm::vec3(0.5f, 0.5f, 0.0f);




constexpr GLint NUMBER_OF_TEXTURES = 1,
LEVEL_OF_DETAIL = 0,
TEXTURE_BORDER = 0;

constexpr int NUMBER_OF_PLAYERS = 2,
                NUMBER_OF_BALLS = 3;


// ————— STRUCTS AND ENUMS —————//
enum AppStatus { RUNNING, TERMINATED };
enum FilterType { NEAREST, LINEAR };

struct GameState
{
    Entity* player1; 
    Entity* player2;
    Entity** balls; 
    //Entity* ball;
    Entity* background;
};

// ————— VARIABLES ————— //
GameState g_game_state;
GLuint g_font_texture_id;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;

void initialise();
void process_input();
void update();
void render();
void shutdown();
void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position);
GLuint load_texture(const char* filepath, FilterType filterType);

// ———— GENERAL FUNCTIONS ———— //

void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / 16;
    float height = 1.0f / 16;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % 16) / 16;
        float v_coordinate = (float)(spritesheet_index / 16) / 16;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    shader_program->set_model_matrix(model_matrix);
    glUseProgram(shader_program->get_program_id());

    glVertexAttribPointer(shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(shader_program->get_position_attribute());

    glVertexAttribPointer(shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(shader_program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(shader_program->get_position_attribute());
    glDisableVertexAttribArray(shader_program->get_tex_coordinate_attribute());
}

GLuint load_texture(const char* filepath, FilterType filterType)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER,
        GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        filterType == NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        filterType == NEAREST ? GL_NEAREST : GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}



void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Towers!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        shutdown();
    }

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




    //BACKGROUND
    GLuint background_texture_id = load_texture(BACKGROUND_FILEPATH, NEAREST);

    g_game_state.background = new Entity(
        background_texture_id,  // texture id
        0.0f,              // speed
        BACKGROUND_INIT_SCALE
    );


    //  ------ BALL ------- 
    //GLuint ball_texture_id = load_texture(BALL_FILEPATH, LINEAR);

    //g_game_state.ball = new Entity(
    //    ball_texture_id,  // texture id
    //    1.0f ,             // speed
    //    BALL_INIT_SCALE
    //);
    //g_game_state.ball->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    //g_game_state.ball->set_movement(glm::vec3(1.0f, 1.0f, 0.0f));


    //// ————— Player1 ————— //
    GLuint player1_texture_id = load_texture(PLAYER1_FILEPATH, NEAREST);

    g_game_state.player1 = new Entity(
        player1_texture_id,  // texture id
        1.0f,              // speed
        PLAYER1_INIT_SCALE
    );
    g_game_state.player1->set_position(glm::vec3(4.5f, 0.0f, 0.0f));

    //// ————— Player2 ————— //
    GLuint player2_texture_id = load_texture(PLAYER2_FILEPATH, NEAREST);

    g_game_state.player2 = new Entity(
        player2_texture_id,  // texture id
        1.0f,              // speed
        PLAYER2_INIT_SCALE
    );
    g_game_state.player2->set_position(glm::vec3(-4.5f, 0.0f, 0.0f));

    // ————— BALLS ————— //
    g_game_state.balls = new Entity * [3];
    GLuint ball_texture_id = load_texture(BALL_FILEPATH, NEAREST);
    for (int i = 0; i < 3; i++)
    {
        g_game_state.balls[i] = new Entity(
            ball_texture_id,  // texture id
            1.0f,              // speed
            BALL_INIT_SCALE
        );
        g_game_state.balls[i]->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
        g_game_state.balls[i]->set_movement(glm::vec3(1.0f, 1.0f, 0.0f));
    }

    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    g_font_texture_id = load_texture(FONT_FILEPATH, NEAREST);

}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_game_state.player1->set_movement(glm::vec3(0.0f));
    if(!g_game_state.player1->single_player)
        g_game_state.player2->set_movement(glm::vec3(0.0f));

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
            case SDLK_q: g_app_status = TERMINATED;

            case SDLK_t:  g_game_state.player1->single_player = true;
                          g_game_state.player2->set_movement(glm::vec3(0.0f, 1.0f, 0.0f));



            default:     break;
            }
        


        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (!g_game_state.player1->single_player) {
        if (key_state[SDL_SCANCODE_UP])
        {
            if (!(g_game_state.player2->check_top_collision())) {
                g_game_state.player2->move_up();
            }
        }
        else if (key_state[SDL_SCANCODE_DOWN])
        {
            if (!(g_game_state.player2->check_bottom_collision())) {
                g_game_state.player2->move_down();
            }
        }
    }

    if (key_state[SDL_SCANCODE_W])
    {
        if (!(g_game_state.player1->check_top_collision())) {
            g_game_state.player1->move_up();
        }
            
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        if (!(g_game_state.player1->check_bottom_collision())) {
            g_game_state.player1->move_down();
        }
    }

    if (key_state[SDL_SCANCODE_1]) {
        g_game_state.player1->number_of_balls = 1;
    }
    if (key_state[SDL_SCANCODE_2]) {
        g_game_state.player1->number_of_balls = 2;
    }
    if (key_state[SDL_SCANCODE_3]) {
        g_game_state.player1->number_of_balls = 3;
    }

  



    if (glm::length(g_game_state.player1->get_movement()) > 1.0f)
        g_game_state.player1->normalise_movement();

    if (glm::length(g_game_state.player2->get_movement()) > 1.0f)
        g_game_state.player2->normalise_movement();

   
}


void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    g_game_state.player1->update(delta_time, g_game_state.balls, g_game_state.background, g_game_state.player2);
    /*g_game_state.player2->update(delta_time,);
    g_game_state.ball->update(delta_time);*/

}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    g_game_state.background->render(&g_shader_program);
    g_game_state.player1->render(&g_shader_program);
    g_game_state.player2->render(&g_shader_program);
    for(int i=0; i<g_game_state.player1->number_of_balls; i++)
        g_game_state.balls[i]->render(&g_shader_program);
    if (g_game_state.player1->end_game(g_game_state.balls)) {
        if (g_game_state.player1->m_collided_left) {
            draw_text(&g_shader_program, g_font_texture_id, "BLUE WINS!", 0.5f, 0.05f, glm::vec3(-2.25f, 2.0f, 0.0f));

        }
        else if (g_game_state.player1->m_collided_right) {
            draw_text(&g_shader_program, g_font_texture_id, "RED WINS!", 0.5f, 0.05f, glm::vec3(-2.25f, 2.0f, 0.0f));
        }
    }

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown()
{
    SDL_Quit();
    delete   g_game_state.player1;
    delete   g_game_state.player2;
    delete[] g_game_state.balls;
    delete g_game_state.background;


}


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