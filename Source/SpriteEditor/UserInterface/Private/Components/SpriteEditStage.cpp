#include "SpriteEditStage.h"
#include "MainScreen.h"
#include "SpriteStaticData.h"
#include "AUI/Core.h"

namespace AM
{
namespace SpriteEditor
{

SpriteEditStage::SpriteEditStage(MainScreen& inScreen)
: AUI::Component(inScreen, "SpriteEditStage", {389, 60, 1142, 684})
, mainScreen{inScreen}
, checkerboardImage(inScreen, "", {0, 0, 100, 100})
, spriteImage(inScreen, "", {0, 0, 100, 100})
, boundingBoxGizmo(inScreen)
{
    /* Active sprite and checkerboard background. */
    checkerboardImage.addResolution({1920, 1080}, "Textures/SpriteEditStage/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    spriteImage.setIsVisible(false);

    /* Bounding box gizmo. */
    boundingBoxGizmo.setIsVisible(false);
}

void SpriteEditStage::loadActiveSprite(SpriteStaticData* activeSprite)
{
    if (activeSprite != nullptr) {
        // Load the new sprite image.
        spriteImage.clearTextures();
        spriteImage.addResolution(AUI::Core::GetLogicalScreenSize()
            , activeSprite->parentSpriteSheet.relPath, activeSprite->textureExtent);

        // Calc the centered sprite position.
        SDL_Rect centeredSpriteExtent{activeSprite->textureExtent};
        centeredSpriteExtent.x = logicalExtent.w / 2;
        centeredSpriteExtent.x -= (centeredSpriteExtent.w / 2);
        centeredSpriteExtent.y = logicalExtent.h / 2;
        centeredSpriteExtent.y -= (centeredSpriteExtent.h / 2);

        // Size the sprite image to the sprite extent size.
        spriteImage.setLogicalExtent(centeredSpriteExtent);

        // Set the background to the size of the sprite.
        checkerboardImage.setLogicalExtent(spriteImage.getLogicalExtent());

        // Set the sprite and background to be visible.
        checkerboardImage.setIsVisible(true);
        spriteImage.setIsVisible(true);

        // Load the sprite into the gizmo and make it visible.
        boundingBoxGizmo.loadActiveSprite(activeSprite, spriteImage.getScaledExtent());
        boundingBoxGizmo.setIsVisible(true);
    }
}

void SpriteEditStage::refresh()
{
    boundingBoxGizmo.refresh();
}

void SpriteEditStage::render(const SDL_Point& parentOffset)
{
    // Keep our scaling up to date.
    refreshScaling();

    // Save the extent that we're going to render at.
    lastRenderedExtent = scaledExtent;
    lastRenderedExtent.x += parentOffset.x;
    lastRenderedExtent.y += parentOffset.y;

    // Children should render at the parent's offset + this component's offset.
    SDL_Point childOffset{parentOffset};
    childOffset.x += scaledExtent.x;
    childOffset.y += scaledExtent.y;

    // Render our children.
    checkerboardImage.render(childOffset);

    spriteImage.render(childOffset);

    boundingBoxGizmo.render(childOffset);
}

} // End namespace SpriteEditor
} // End namespace AM
