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
#include "Entity.h"

// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
      m_speed(0.0f), m_texture_id(0){}


// Simpler constructor for partial initialization
Entity::Entity(GLuint texture_id, float speed, glm::vec3 scale)
    : m_position(0.0f), m_movement(0.0f), m_scale(scale), m_model_matrix(1.0f),
      m_speed(speed), m_texture_id(texture_id){}

Entity::~Entity() { }

void Entity::update(float delta_time, Entity** balls, Entity* background, Entity* player2)
{
   //Translation
    if (single_player) {
        if (player2->m_position.y > 3.3f) {
            player2->m_movement.y = -1.0f;
        }
        else {
            if (player2->m_position.y < -3.3f) {
                player2->m_movement.y = 1.0f;
            }
        }
    }

    for (int i = 0; i < number_of_balls; i++) {
        Entity* ball = balls[i];
        if (check_collision(ball) || player2->check_collision(ball)) {
            ball->m_movement.x *= -1.0f;
            ball->m_movement.y *= -1.0f;
        }

        if (ball->m_position.y > 3.3f) {
            ball->m_movement.y = -1.0f;
        }
        else {
            if (ball->m_position.y < -3.3f) {
                ball->m_movement.y = 1.0f;
            }
        }

        ball->m_position += ball->m_movement * ball->m_speed * delta_time;
        ball->m_model_matrix = glm::mat4(1.0f);
        if (!(end_game(balls))) {
            ball->m_model_matrix = glm::translate(ball->m_model_matrix, ball->m_position);
            ball->m_model_matrix = glm::scale(ball->m_model_matrix, ball->m_scale);
        }
        else {
            ball->m_model_matrix = glm::scale(ball->m_model_matrix, glm::vec3(0.0f,0.0f,0.0f));

        }

    }
    

    //game stops when ball disapear
    if (!(end_game(balls))) {
        m_position += m_movement * m_speed * delta_time;
        player2->m_position += player2->m_movement * player2->m_speed * delta_time;
    }

    
    
    //RESET
    m_model_matrix = glm::mat4(1.0f);
    player2->m_model_matrix = glm::mat4(1.0f);
    background->m_model_matrix = glm::mat4(1.0f);

    //TRANSLATE
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    player2->m_model_matrix = glm::translate(player2->m_model_matrix, player2->m_position);


    //SCALE
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
    player2->m_model_matrix = glm::scale(player2->m_model_matrix, player2->m_scale);
    background->m_model_matrix = glm::scale(background->m_model_matrix, background->m_scale);


}


void Entity::render(ShaderProgram *program)
{
    program->set_model_matrix(m_model_matrix);
    
    float vertices[]   = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = {  0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, m_texture_id);


    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}


bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((this->get_scale().x + other->get_scale().x) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((this->get_scale().y + other->get_scale().y) / 2.0f);
    return x_distance < 0.0f && y_distance < 0.0f;
}

bool const Entity::check_top_collision() {
    return m_position.y > 3.3f;
}


bool const Entity::check_bottom_collision() {
    return m_position.y <-3.3f;
}

bool const Entity::end_game(Entity** balls) {
    bool end = false;
    for (int i = 0; i < number_of_balls; i++) {
        if (balls[i]->m_position.x > 5) {
            m_collided_right = true;
            return  true;
        }
        else if(balls[i]->m_position.x < -5){
            m_collided_left = true;
            return  true;

        }
    }
    return false;
}









