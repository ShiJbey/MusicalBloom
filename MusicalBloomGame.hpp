#include "Sound.hpp"
#include "Scene.hpp"

#include <vector>
#include <cmath>
#include <random>
#include <time.h>
#include <stdlib.h>



namespace MusicalBloom
{
    struct Cube {
        // Active gets passed to the cube shader and determines if we illuminate
        // this cube or not
        bool active = false;
        Scene::Object *object;
    };

    struct MusicalBloomGame
    {
        MusicalBloomGame();

        // Randomly choses an integer toadd to the current sequence
        uint32_t increment_sequence();

        // Clears out the current sequence (used when player loses)
        void clear_sequence();

        // Prints the sequence to the console
        void print_sequence();

        // Plays a the sound for a cube and sets it's active variable to true
        void activate_cube(uint32_t cube_index);

        // Sets a specific cube not to be active
        void deactivate_cube(uint32_t cube_index);

        // Returns true if the number of cubes and the numeber of sounds are equal
        bool is_valid();

        // Returns the max size of sequence they made it past
        uint32_t get_score();

        // Cubes and sounds managed by the game
        std::vector<const Sound::Sample*> sounds;
        std::vector<Cube> cubes;
        uint32_t currentCube = 0;

        // The current sequence that the player has to repeat back
        std::vector<uint32_t> sequence;

        // When playing back the sequence, how much time should be placed
        // between cube activations
        float time_between_sounds;
    };
}