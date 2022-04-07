#pragma once

namespace AM
{

/**
 * The ID of the "empty sprite", or the ID given to a tile sprite layer that
 * is empty.
 *
 * Placed in a separate file since the Client and Server don't share a
 * SpriteData file.
 */
static constexpr int EMPTY_SPRITE_ID = -1;

} // End namespace AM
