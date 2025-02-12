#pragma once

#include "AUI/Screen.h"
#include "AUI/Thumbnail.h"
#include <string>

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
/**
 * The thumbnail style used for the main screen.
 */
class MainThumbnail : public AUI::Thumbnail
{
public:
    MainThumbnail(AssetCache& assetCache,
                  const std::string& inDebugName = "MainThumbnail");
};

} // End namespace SpriteEditor
} // End namespace AM
