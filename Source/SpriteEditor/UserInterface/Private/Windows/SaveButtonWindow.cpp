#include "SaveButtonWindow.h"
#include "AssetCache.h"
#include "MainScreen.h"
#include "SpriteDataModel.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{
SaveButtonWindow::SaveButtonWindow(AssetCache& inAssetCache,
                                   MainScreen& inScreen,
                                   SpriteDataModel& inSpriteDataModel)
: AUI::Window({1537, 0, 58, 58}, "SaveButtonWindow")
, assetCache{inAssetCache}
, mainScreen{inScreen}
, spriteDataModel{inSpriteDataModel}
, saveButton({0, 0, 58, 58})
{
    // Add our children so they're included in rendering, etc.
    children.push_back(saveButton);

    /* Save button. */
    saveButton.normalImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "SaveButton/Normal.png"));
    saveButton.hoveredImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "SaveButton/Hovered.png"));
    saveButton.pressedImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "SaveButton/Pressed.png"));
    saveButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    saveButton.text.setText("");

    // Add a callback to save the current sprite data when pressed.
    saveButton.setOnPressed([this]() {
        // Create our callback.
        std::function<void(void)> onConfirmation = [&]() {
            // Save the data.
            spriteDataModel.save();
        };

        // Open the confirmation dialog.
        mainScreen.openConfirmationDialog("Save over existing SpriteData.json?",
                                          "SAVE", std::move(onConfirmation));
    });
}

} // End namespace SpriteEditor
} // End namespace AM
