#pragma once

#include "MusicalBloomGame.hpp"
#include "Mode.hpp"
#include "Scene.hpp"
#include "Sound.hpp"

#include "MeshBuffer.hpp"
#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

namespace MusicalBloom
{
    
    struct MusicalBloomMode : public Mode
    {
        MusicalBloomMode();

        //handle_event is called when new mouse or keyboard events are received:
        // (note that this might be many times per frame or never)
        //The function should return 'true' if it handled the event.
        virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;

        //update is called at the start of a new frame, after events are handled:
        virtual void update(float elapsed) override;

        //draw is called after update:
        virtual void draw(glm::uvec2 const &drawable_size) override;

        //pulls up the pause menu
        void show_pause_menu();

        // Plays the sequence at the beginning of a round
        void play_sequence(float elapsed);

        // Changes the shader of the selected cube and
        // Plays the sounds attahed to that cube
        void highlight_cube(uint32_t cube_index);

        // Changes the shader of the cube back to the
        // default dim shader
        void reset_cube(uint32_t cube_index);

        // Changes the shader all cubes back to the
        // default dim shader
        void reset_all_cubes();

        MusicalBloomGame game;
        Scene scene;
        Scene::Camera* camera;

        uint32_t player_choice = -1U;
        uint32_t player_streak = 0;
        bool playing_sequence = false;
        bool round_started = false;
        bool game_over = false;

        // Time in seconds between playing sounds in a sequence
        float time_between_highlights = 1.0f;
        // Time between the start of rounds
        float time_between_rounds = 2.5f;
        // timer until next play
        float time_to_next_highlight = 2.5f;
        // Current index in the sequence
        uint32_t sequence_pos = 0;

        // Program information for highlighting the cubes
        Scene::Object::ProgramInfo highlight_test_program_info;
        Scene::Object::ProgramInfo vertex_color_program_info;


        std::shared_ptr< Sound::PlayingSample > current_sound;
    };

}