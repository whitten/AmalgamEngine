#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace AM
{
namespace MG
{

/**
 * Generates the World.map file based on the given parameters.
 */
class MapGenerator
{
public:
    MapGenerator(unsigned int inMapLengthX, unsigned int inMapLengthY, const std::string& inFillSpriteId);

    /**
     * Generates the map, kept in memory as a string.
     */
    void generate();

    /**
     * Saves the map to a file with the given name, placed in the same
     * directory as the program binary.
     */
    void save(const std::string& fileName);

private:
    /** The version of the map format. Kept as just a 16-bit int for now, we
        can see later if we care to make it more complicated. */
    static constexpr uint16_t MAP_FORMAT_VERSION = 0;

    /** The length, in chunks, of the map's X axis. */
    unsigned int mapXLength;

    /** The length, in chunks, of the map's Y axis. */
    unsigned int mapYLength;

    /** The ID of the sprite to fill the map with. */
    std::string fillSpriteId;

    /** The string that we store the generated map data in. */
    std::vector<uint8_t> mapData;

    /** The first empty element index in our mapData buffer. */
    unsigned long bufferIndex;
};

} // End namespace MG
} // End namespace AM
