#include "SpriteData.h"
#include "AssetCache.h"
#include "Paths.h"
#include "Log.h"

#include <fstream>

#include "nlohmann/json.hpp"

namespace AM
{
namespace Client
{
SpriteData::SpriteData(AssetCache& assetCache)
{
    // Open the file.
    std::string fullPath{Paths::BASE_PATH};
    fullPath += "SpriteData.json";
    std::ifstream workingFile(fullPath);
    if (!(workingFile.is_open())) {
        LOG_ERROR("Failed to open SpriteData.json");
    }

    // Parse the file into a json structure.
    nlohmann::json json = nlohmann::json::parse(workingFile, nullptr, false);
    if (json.is_discarded()) {
        LOG_ERROR("SpriteData.json is not valid JSON.");
    }

    // Parse the json structure to construct our sprites.
    parseJson(json, assetCache);
}

const Sprite& SpriteData::get(const std::string& stringId)
{
    // Attempt to find the given string ID.
    auto it = stringMap.find(stringId);
    if (it == stringMap.end()) {
        LOG_ERROR("Failed to find sprite string ID: %s", stringId.c_str());
    }

    return *(it->second);
}

const Sprite& SpriteData::get(int numericId)
{
    // Attempt to find the given string ID.
    auto it = numericMap.find(numericId);
    if (it == numericMap.end()) {
        LOG_ERROR("Failed to find sprite numeric ID: %d", numericId);
    }

    return *(it->second);
}

void SpriteData::parseJson(nlohmann::json& json, AssetCache& assetCache)
{
    // Parse the json and catch any parsing errors.
    try {
        // For every sprite sheet in the json.
        for (auto& sheetJson : json["spriteSheets"].items()) {
            // Get this sheet's texture.
            std::string texturePath{Paths::TEXTURE_DIR};
            texturePath += sheetJson.value()["relPath"].get<std::string>();
            TextureHandle texture{assetCache.loadTexture(texturePath)};

            // For every sprite in the sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                parseSprite(spriteJson.value(), texture);
            }
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_ERROR("Parse failure - %s", e.what());
    }

    // Fill the maps with pointers to all the sprites.
    // Note: This must be done after filling the sprites vector, since a
    //       push_back() can cause a re-allocation which invalidates pointers.
    for (Sprite& sprite : sprites) {
        stringMap.emplace(sprite.stringId, &sprite);
        numericMap.emplace(sprite.numericId, &sprite);
    }
}

void SpriteData::parseSprite(const nlohmann::json& spriteJson,
                             const TextureHandle& texture)
{
    // Add the parent sprite sheet's texture.
    Sprite sprite{};
    sprite.texture = texture;

    // Add the display name.
    sprite.displayName = spriteJson["displayName"].get<std::string>();

    // Add the string identifier.
    sprite.stringId = spriteJson["stringId"].get<std::string>();

    // Add the numeric identifier.
    sprite.numericId = spriteJson["numericId"];

    // Add this sprite's extent within the sprite sheet.
    sprite.textureExtent.x = spriteJson["textureExtent"]["x"];
    sprite.textureExtent.y = spriteJson["textureExtent"]["y"];
    sprite.textureExtent.w = spriteJson["textureExtent"]["w"];
    sprite.textureExtent.h = spriteJson["textureExtent"]["h"];

    // Add the Y offset.
    sprite.yOffset = spriteJson["yOffset"];

    // Add whether the sprite has a bounding box or not.
    sprite.hasBoundingBox = spriteJson["hasBoundingBox"];

    // Add the model-space bounds.
    sprite.modelBounds.minX = spriteJson["modelBounds"]["minX"];
    sprite.modelBounds.maxX = spriteJson["modelBounds"]["maxX"];
    sprite.modelBounds.minY = spriteJson["modelBounds"]["minY"];
    sprite.modelBounds.maxY = spriteJson["modelBounds"]["maxY"];
    sprite.modelBounds.minZ = spriteJson["modelBounds"]["minZ"];
    sprite.modelBounds.maxZ = spriteJson["modelBounds"]["maxZ"];

    // Save the sprite in the sprites vector.
    sprites.push_back(sprite);
}

} // End namespace Client
} // End namespace AM
