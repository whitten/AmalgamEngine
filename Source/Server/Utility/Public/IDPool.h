#ifndef IDPOOL_H
#define IDPOOL_H

#include "GameDefs.h"
#include <SDL_stdinc.h>
#include <array>

namespace AM
{
namespace Server
{
class IDPool
{
public:
    IDPool();

    Uint32 reserveID();

    void freeID(Uint32 ID);

private:
    /**
     * If ID 'x' is available, IDs[x] will be true. Else, it will be false.
     */
    std::array<bool, MAX_ENTITIES> IDs;
};

} // namespace Server
} // namespace AM

#endif /* IDPOOL_H */
