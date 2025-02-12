#include "TileMapBase.h"
#include "SpriteDataBase.h"
#include "Paths.h"
#include "Position.h"
#include "Transforms.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
#include "SharedConfig.h"
#include "EmptySpriteID.h"
#include "Timer.h"
#include "Log.h"
#include "AMAssert.h"
#include "Ignore.h"
#include <algorithm>

namespace AM
{
TileMapBase::TileMapBase(SpriteDataBase& inSpriteData)
: spriteData{inSpriteData}
, chunkExtent{}
, tileExtent{}
, tiles{}
{
}

void TileMapBase::setTileSpriteLayer(int tileX, int tileY,
                                     unsigned int layerIndex,
                                     const Sprite& sprite)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    std::vector<Tile::SpriteLayer>& spriteLayers{tile.spriteLayers};

    // If we're being asked to set the highest layer in the tile to the empty 
    // sprite, erase it and any empties below it instead (to reduce space).
    if ((sprite.numericID == EMPTY_SPRITE_ID) 
        && (layerIndex == (spriteLayers.size() - 1))) {
        // Erase the sprite.
        spriteLayers.erase(spriteLayers.begin() + layerIndex);

        // Erase any empty sprites below it.
        for (unsigned int endIndex = (layerIndex - 1); endIndex-- > 0; ) {
            if (spriteLayers[endIndex].sprite.numericID == EMPTY_SPRITE_ID) {
                spriteLayers.erase(spriteLayers.begin() + endIndex);
            }
            else {
                break;
            }
        }
    }
    // Else, set the sprite layer.
    else {
        // If the sprite has a bounding box, calculate its position.
        BoundingBox worldBounds{};
        if (sprite.hasBoundingBox) {
            Position tilePosition{
                static_cast<float>(tileX * SharedConfig::TILE_WORLD_WIDTH),
                static_cast<float>(tileY * SharedConfig::TILE_WORLD_WIDTH), 0};
            worldBounds
                = Transforms::modelToWorld(sprite.modelBounds, tilePosition);
        }

        // If the tile's layers vector isn't big enough, resize it.
        // Note: This sets intermediate layers to the empty sprite.
        if (spriteLayers.size() <= layerIndex) {
            spriteLayers.resize(
                (layerIndex + 1),
                {spriteData.get(EMPTY_SPRITE_ID), BoundingBox{}});
        }

        // Replace the sprite.
        spriteLayers[layerIndex] = {sprite, worldBounds};
    }
}

void TileMapBase::setTileSpriteLayer(int tileX, int tileY,
                                     unsigned int layerIndex,
                                     const std::string& stringID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(stringID));
}

void TileMapBase::setTileSpriteLayer(int tileX, int tileY,
                                     unsigned int layerIndex, int numericID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(numericID));
}

void TileMapBase::clearTile(int tileX, int tileY)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    tile.spriteLayers.clear();
}

const Tile& TileMapBase::getTile(unsigned int x, unsigned int y) const
{
    unsigned int tileIndex{linearizeTileIndex(x, y)};
    unsigned int maxTileIndex{
        static_cast<unsigned int>(tileExtent.xLength * tileExtent.yLength)};
    AM_ASSERT((tileIndex < maxTileIndex),
              "Tried to get an out of bounds tile. tileIndex: %u, max: %u",
              tileIndex, maxTileIndex);
    ignore(maxTileIndex);

    return tiles[tileIndex];
}

const ChunkExtent& TileMapBase::getChunkExtent() const
{
    return chunkExtent;
}

const TileExtent& TileMapBase::getTileExtent() const
{
    return tileExtent;
}

} // End namespace AM
