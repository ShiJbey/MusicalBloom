#include "MusicalBloomMode.hpp"
#include "GameMode.hpp"
#include "MenuMode.hpp"
#include "Load.hpp"
#include "MeshBuffer.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp" //helper for dumpping OpenGL error messages
#include "check_fb.hpp" //helper for checking currently bound OpenGL framebuffer
#include "read_chunk.hpp" //helper for reading a vector of structures from a file
#include "data_path.hpp" //helper to get paths relative to executable
#include "compile_program.hpp" //helper to compile opengl shader programs
#include "draw_text.hpp" //helper to... um.. draw text
#include "load_save_png.hpp"
#include "vertex_color_program.hpp"
#include "highlight_test_program.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <cmath>
#include <random>

Load< MeshBuffer > musical_bloom_meshes(LoadTagDefault, [](){
	return new MeshBuffer(data_path("musical_bloom.pnc"));
});

Load< GLuint > musical_bloom_meshes_for_vertex_color_program(LoadTagDefault, [](){
	return new GLuint(musical_bloom_meshes->make_vao_for_program(vertex_color_program->program));
});

Load< GLuint > musical_bloom_meshes_for_highlight_test_program(LoadTagDefault, [](){
	return new GLuint(musical_bloom_meshes->make_vao_for_program(highlight_test_program->program));
});

// Sounds from: https://freesound.org/people/DANMITCH3LL/sounds/
Load< Sound::Sample > xylophone_a(LoadTagDefault, [](){
        return new Sound::Sample(data_path("xylophone-a.wav"));
});

Load< Sound::Sample > xylophone_c(LoadTagDefault, [](){
        return new Sound::Sample(data_path("xylophone-c.wav"));
});

Load< Sound::Sample > xylophone_d(LoadTagDefault, [](){
        return new Sound::Sample(data_path("xylophone-d1.wav"));
});

Load< Sound::Sample > xylophone_e(LoadTagDefault, [](){
        return new Sound::Sample(data_path("xylophone-e1.wav"));
});

MusicalBloom::MusicalBloomMode::MusicalBloomMode() {
    std::cout << "DEBUG:: " << "Starting Musical Mode" << std::endl;

    // Set all 4 sounds
    game.sounds.push_back(xylophone_a.value);
    game.sounds.push_back(xylophone_c.value);
    game.sounds.push_back(xylophone_d.value);
    game.sounds.push_back(xylophone_e.value);

    // Add 4 cubes
    game.cubes.push_back(Cube());
    game.cubes.push_back(Cube());
    game.cubes.push_back(Cube());
    game.cubes.push_back(Cube());

    // Set uniform ids for the highlight_test shader program
    highlight_test_program_info.program = highlight_test_program->program;
    highlight_test_program_info.mvp_mat4 = highlight_test_program->object_to_clip_mat4;
    highlight_test_program_info.mv_mat4x3 = highlight_test_program->object_to_light_mat4x3;
    highlight_test_program_info.itmv_mat3 = highlight_test_program->normal_to_light_mat3;
    highlight_test_program_info.vao = *musical_bloom_meshes_for_highlight_test_program;

    // Set uniform IDs fot the dimmer shader
    vertex_color_program_info.program = vertex_color_program->program;
    vertex_color_program_info.mvp_mat4 = vertex_color_program->object_to_clip_mat4;
    vertex_color_program_info.mv_mat4x3 = vertex_color_program->object_to_light_mat4x3;
    vertex_color_program_info.itmv_mat3 = vertex_color_program->normal_to_light_mat3;
    vertex_color_program_info.vao = *musical_bloom_meshes_for_vertex_color_program;


    auto attach_object = [this](Scene::Transform *transform, std::string const &name)
    {
        Scene::Object *object = scene.new_object(transform);
        MeshBuffer::Mesh const &mesh = musical_bloom_meshes->lookup(name);
        object->programs[Scene::Object::ProgramTypeDefault] = vertex_color_program_info;
        object->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
		object->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;
        return object;
    };

    // Create Cube Objects and Camera
    Scene::Transform *transform_1 = scene.new_transform();
    transform_1->position = glm::vec3(-4.5f, 0.0f, 0.0f);
    game.cubes[0].object = attach_object(transform_1, "GreenCube");
    game.cubes[0].object->transform->name = "GreenCube";

    Scene::Transform *transform_2 = scene.new_transform();
    transform_2->position = glm::vec3(-1.5f, 0.0f, 0.0f);
    game.cubes[1].object = attach_object(transform_2, "RedCube");
    game.cubes[1].object->transform->name = "RedCube";

    Scene::Transform *transform_3 = scene.new_transform();
    transform_3->position = glm::vec3(1.5f, 0.0f, 0.0f);
    game.cubes[2].object =attach_object(transform_3, "BlueCube");
    game.cubes[2].object->transform->name = "BlueCube";

    Scene::Transform *transform_4 = scene.new_transform();
    transform_4->position = glm::vec3(4.5f, 0.0f, 0.0f);
    game.cubes[3].object = attach_object(transform_4, "YellowCube");
    game.cubes[3].object->transform->name = "YellowCube";

    Scene::Transform *transform_5 = scene.new_transform();
    transform_5->name = "Floor";
    transform_5->scale = glm::vec3(10.0f, 1.0f, 10.0f);
    transform_5->position.y -= 3.0f;
    transform_5->position.z += 2.0f;
    attach_object(transform_5, "Floor");

    Scene::Transform *camera_transform = scene.new_transform();
    camera_transform->position = glm::vec3(0.0f, 0.0f, 20.0f);
    camera_transform->position.y += 6.0f;
    camera_transform->rotation *= glm::angleAxis((float)M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
    camera_transform->rotation *= glm::angleAxis(glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    camera = scene.new_camera(camera_transform);

    if (!game.is_valid())
    {
        std::cout << "ERROR:: " << "Mismatch number of sounds and cubes" << std::endl;
        Mode::set_current(nullptr);
    }
};

void MusicalBloom::MusicalBloomMode::highlight_cube(uint32_t cube_index)
{

    // Get the program information for highlighting cubes and set it
    // as the cubes default program
    Scene::Object *cube_object = game.cubes[cube_index].object;
    cube_object->programs[Scene::Object::ProgramTypeDefault] = highlight_test_program_info;
    MeshBuffer::Mesh const &mesh = musical_bloom_meshes->lookup(cube_object->transform->name);
    cube_object->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
	cube_object->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;
};

void MusicalBloom::MusicalBloomMode::reset_cube(uint32_t cube_index)
{
    // Set the cubes shader back to being the dimmer shader
    Scene::Object *cube_object = game.cubes[cube_index].object;
    cube_object->programs[Scene::Object::ProgramTypeDefault] = vertex_color_program_info;    
    MeshBuffer::Mesh const &mesh = musical_bloom_meshes->lookup(cube_object->transform->name);
    cube_object->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
    cube_object->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;
};

void MusicalBloom::MusicalBloomMode::reset_all_cubes()
{
    for (uint32_t cube_index = 0; cube_index < game.cubes.size(); cube_index++)
    {
        reset_cube(cube_index);
    }
};

void MusicalBloom::MusicalBloomMode::play_sequence(float elapsed)
{
    // Reduce the timer
    time_to_next_highlight -= elapsed;

    if (time_to_next_highlight <= 0.0f)
    {
        if (sequence_pos < game.sequence.size())
        {
            // Play the sound, move the counter and reset timer
            uint32_t cube_index = game.sequence[sequence_pos];

            reset_all_cubes();
            
            highlight_cube(cube_index);
            game.sounds[cube_index]->play(camera->transform->position, 1.0f, Sound::Once);
            sequence_pos++;
            time_to_next_highlight = time_between_highlights;
        }
        else
        {
            // Check to see if we are at the end of the sequence
            if (sequence_pos == game.sequence.size())
            {
                reset_all_cubes();
                // The sequence stops playing and we reset the counter for next time
                playing_sequence = false;
                sequence_pos = 0;
                time_to_next_highlight = time_between_rounds;
            }
        }
    }
};

bool MusicalBloom::MusicalBloomMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{
    player_choice = -1U;
    //ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
        
		return false;
	}

    if (evt.type == SDL_KEYUP) {
        if (evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			//open pause menu on 'ESCAPE':
			show_pause_menu();
			return true;
		}
    }

    // Ignore all keys, but the pause button if we are not
    // waiting for the player to repeat the sequence back
    if (playing_sequence) {
        return false;
    }

    //handle tracking the state of WSAD for roll control:
	if (evt.type == SDL_KEYDOWN && !evt.key.repeat) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_1) {
            std::cout << "DEBUG:: " << "Player said: " << 1 << std::endl;
            highlight_cube(0);
            game.sounds[0]->play(camera->transform->position, 1.0f, Sound::Once);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_2) {
            std::cout << "DEBUG:: " << "Player said: " << 2 << std::endl;
            highlight_cube(1);
            game.sounds[1]->play(camera->transform->position, 1.0f, Sound::Once);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_3) {
            std::cout << "DEBUG:: " << "Player said: " << 3 << std::endl;
            highlight_cube(2);
            game.sounds[2]->play(camera->transform->position, 1.0f, Sound::Once);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_4) {
            std::cout << "DEBUG:: " << "Player said: " << 4 << std::endl;
            highlight_cube(3);
            game.sounds[3]->play(camera->transform->position, 1.0f, Sound::Once);
			return true;
		}
	}

    //handle tracking the state of WSAD for roll control:
	if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_1) {
            player_choice = 0;
            reset_cube(player_choice);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_2) {
            player_choice = 1;
            reset_cube(player_choice);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_3) {
            player_choice = 2;
            reset_cube(player_choice);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_4) {
            player_choice = 3;
            reset_cube(player_choice);
			return true;
		}
	}

	return false;
};

void MusicalBloom::MusicalBloomMode::update(float elapsed)
{
    // Do nothing if this is not the current mode
    if (Mode::current.get() != this)
    {
        return;
    }

    if (!round_started)
    {
        std::cout << "DEBUG:: Starting round" << std::endl;
        round_started = true;

        // Add a number to the current sequence
        game.increment_sequence();

        playing_sequence = true;
        // Play the sequence for the player
        std::cout << "DEBUG:: ";
        game.print_sequence();
        play_sequence(elapsed);
    }
    else if (round_started && playing_sequence)
    {
        play_sequence(elapsed);
        //playing_sequence = false;
    }
    else if (round_started && !playing_sequence && player_choice != -1U)
    {
        if (player_choice == game.sequence[player_streak])
        {
            std::cout << "DEBUG:: " << "Correct" << std::endl;
            player_streak++;
            // We are at the end of the current round
            if (player_streak == game.sequence.size()) {
                round_started = false;
                player_streak = 0; 
            }
        }
        else
        {
            std::cout << "DEBUG:: " << "Incorrect, Reseting" << std::endl;
            std::cout << "DEBUG:: " << "Score: " << game.get_score() << std::endl;
            game_over = true;
            round_started = false;
            player_streak = 0;
            game.clear_sequence();
        }
        player_choice = -1U;
    }
};

void MusicalBloom::MusicalBloomMode::draw(glm::uvec2 const &drawable_size)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   { // Draw the scene
        glUseProgram(vertex_color_program->program);
        glUniform3fv(vertex_color_program->sun_color_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
        glUniform3fv(vertex_color_program->sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(-0.2f, 0.2f, 1.0f))));
        glUniform3fv(vertex_color_program->sky_color_vec3, 1, glm::value_ptr(glm::vec3(0.2f, 0.2f, 0.3f)));
        glUniform3fv(vertex_color_program->sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));

        glUseProgram(highlight_test_program->program);
        glUniform3fv(highlight_test_program->sun_color_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
        glUniform3fv(highlight_test_program->sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(-0.2f, 0.2f, 1.0f))));
        glUniform3fv(highlight_test_program->sky_color_vec3, 1, glm::value_ptr(glm::vec3(0.2f, 0.2f, 0.3f)));
        glUniform3fv(highlight_test_program->sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));

        scene.draw(camera);
    }

    GL_ERRORS();
};

void MusicalBloom::MusicalBloomMode::show_pause_menu() {
	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >();

	std::shared_ptr< Mode > game = shared_from_this();
	menu->background = game;

	menu->choices.emplace_back("PAUSED");
	menu->choices.emplace_back("RESUME", [game](){
		Mode::set_current(game);
	});
	menu->choices.emplace_back("QUIT", [](){
		Mode::set_current(nullptr);
	});

	menu->selected = 1;

	Mode::set_current(menu);
}
