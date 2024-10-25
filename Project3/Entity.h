/**
* Author: Joori Bin Jardan
* Assignment: Lunar Lander
* Date due: 2024-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#ifndef ENTITY_H
#define ENTITY_H

#include "glm/glm.hpp"
#include "ShaderProgram.h"
enum EntityType { PLATFORM, PLAYER, ITEM };

class Entity
{
private:
    bool m_is_active = true;

    // ––––– ANIMATION ––––– //
    int* m_animation_right = NULL; // move to the right
    int* m_animation_left = NULL; // move to the left
    int* m_animation_up = NULL; // move upwards (flying)
    int* m_animation_down = NULL; // move downwards (the player cant move downwards this will be the aniimation they realse the flying key)

    ////ANIMATION TIME
    //float m_animation_time = 0.0f;  // Time passed since last frame update
    //float m_animation_speed = 0.1f; // Duration of each frame (in seconds)
    //int m_current_frame = 0;        // Current frame within the animation
    //int m_num_frames = 0;           // Total number of frames in current animation set

    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    float m_width = 1;
    float m_height = 1;

public:
    // ––––– STATIC ATTRIBUTES ––––– //
    static const int SECONDS_PER_FRAME = 4;
    static const int LEFT = 0,
        RIGHT = 1,
        UP = 2,
        DOWN = 3;

    // ––––– SETUP AND RENDERING ––––– //
    GLuint m_texture_id;
    glm::mat4 m_model_matrix;
    EntityType m_type;


    // ––––– TRANSLATIONS ––––– //
    float m_speed;
    glm::vec3 m_movement;
    float fuel = 0.0f;
    bool is_background = false;

    // ––––– ANIMATIONS ––––– //
    int** m_walking = new int* [4] { m_animation_left, m_animation_right, m_animation_up, m_animation_down };
    int* m_animation_indices = NULL;
    int m_animation_frames = 0;
    int m_animation_index = 0;
    float m_animation_time = 0.0f;
    int m_animation_cols = 0;
    int m_animation_rows = 0;
    int m_frames_left = 0;
    int m_frames_right = 0;
    int m_frames_up = 0;
    int m_frames_down = 0;


    //––––– WINNING ––––– //
    bool mission_accomplished = false;
    bool mission_failed = false;

    // ––––– PHYSICS (JUMPING) ––––– //
    bool m_is_jumping = false;
    float m_jumping_power = 0;

    // ––––– PHYSICS (COLLISIONS) ––––– //
    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;

    // ––––– METHODS ––––– //
    Entity();
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void update(float delta_time, Entity* collidable_entities, int collidable_entity_count, Entity* collidable_entities_lose, int collidable_entity_count_lose);
    void render(ShaderProgram* program);

    //void update_animation(float delta_time);

    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    bool const check_collision(Entity* other) const;

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    // ––––– GETTERS ––––– //
    glm::vec3 const get_position()     const { return m_position; };
    glm::vec3 const get_movement()     const { return m_movement; };
    glm::vec3 const get_velocity()     const { return m_velocity; };
    glm::vec3 const get_acceleration() const { return m_acceleration; };
    int       const get_width()        const { return m_width; };
    int       const get_height()       const { return m_height; };
    bool is_fueld() { return fuel > 0; };

    // ––––– SETTERS ––––– //
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; };
    void const set_acceleration_x(float new_acceleration) { m_acceleration.x = new_acceleration; };
    void const set_acceleration_y(float new_acceleration) { m_acceleration.y = new_acceleration; };
    void const set_width(float new_width) { m_width = new_width; };
    void const set_height(float new_height) { m_height = new_height; };
};
#endif