#include "MusicalBloomGame.hpp"
#include <iostream>

MusicalBloom::MusicalBloomGame::MusicalBloomGame()
{
    // Seed the random number generator forr sequence creation
    srand((unsigned int)time(NULL));
};

uint32_t MusicalBloom::MusicalBloomGame::increment_sequence()
{
    //uint32_t next_cube = (uint32_t)(rand() % (int)cubes.size());
    uint32_t next_cube = (uint32_t)(rand() % 4);
    sequence.push_back(next_cube);
    return next_cube;
};

void MusicalBloom::MusicalBloomGame::clear_sequence()
{
    sequence.clear();
};

void MusicalBloom::MusicalBloomGame::print_sequence()
{
    std::cout << "Sequence: ";
    for (uint32_t i = 0; i < sequence.size(); i++)
    {
        std::cout << sequence[i] + 1<< ", ";
    }
    std::cout << std::endl;
};

void MusicalBloom::MusicalBloomGame::activate_cube(uint32_t cube_index)
{
    if (cube_index < cubes.size())
    {
        cubes[cube_index].active = true;
        //sounds[cube_index].play()
    }
};

void MusicalBloom::MusicalBloomGame::deactivate_cube(uint32_t cube_index)
{
    if (cube_index < cubes.size())
    {
        cubes[cube_index].active = false;
    }
};

bool MusicalBloom::MusicalBloomGame::is_valid()
{
    return cubes.size() == sounds.size();
};

uint32_t MusicalBloom::MusicalBloomGame::get_score()
{
    return (uint32_t)(sequence.size());
};
