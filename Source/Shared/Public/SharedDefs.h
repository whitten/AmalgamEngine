#ifndef AMSTRUCTS_H
#define AMSTRUCTS_H

#include <SDL_stdinc.h>
#include <memory>
#include <vector>

/**
 * This file contains shared definitions that should be
 * consistent between the server and client.
 */
namespace AM
{

/** Game constants. */
static constexpr unsigned int MAX_ENTITIES = 100;
static constexpr unsigned int SCREEN_WIDTH = 1280;
static constexpr unsigned int SCREEN_HEIGHT = 720;

typedef Uint32 EntityID;

typedef std::unique_ptr<std::vector<Uint8>> BinaryBufferPtr;
typedef std::shared_ptr<std::vector<Uint8>> BinaryBufferSharedPtr;

/** Structs. */
struct ComponentFlag
{
    enum FlagType
    {
        Position = 1 << 0,
        Movement = 1 << 1,
        Input = 1 << 2,
        Sprite = 1 << 3
    };
};

struct Position
{
    float x;
    float y;
};

} /* End namespace AM */

#endif /* End AMSTRUCTS_H */
