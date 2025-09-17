#include "scene/AScene.hpp"

namespace gomoku::scene {

AScene::AScene(Context& context)
    : context_(context)
{
}

void AScene::playSfx(const char* name, float volume) const
{
    if (!context_.resourceManager || !context_.sfx)
        return;
    auto* buffer = context_.resourceManager->getSound(name);
    if (!buffer)
        return;
    context_.sfx->setBuffer(*buffer);
    context_.sfx->setVolume(volume);
    context_.sfx->play();
}

void AScene::playMusic(const char* path, bool loop, float volume) const
{
    if (!context_.music)
        return;
    context_.music->stop();
    if (context_.music->openFromFile(path)) {
        context_.music->setLoop(loop);
        context_.music->setVolume(volume);
        context_.music->play();
    }
}

} // namespace gomoku::scene