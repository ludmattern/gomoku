#include "scene/AScene.hpp"

namespace gomoku::scene {

AScene::AScene(Context& context)
    : context_(context)
{
}

void AScene::playSfx(const char* name, float volume) const
{
    if (!context_.resourceManager || !context_.sfxVoices)
        return;
    if (!context_.sfxEnabled)
        return;
    auto* buffer = context_.resourceManager->getSound(name);
    if (!buffer)
        return;
    // Find a free voice (stopped) or the first one
    for (auto& voice : *context_.sfxVoices) {
        if (voice.getStatus() == sf::Sound::Status::Stopped) {
            voice.setBuffer(*buffer);
            float vol = std::max(0.f, std::min(100.f, context_.sfxVolume));
            voice.setVolume(std::min(100.f, volume * (vol / 100.f)));
            voice.play();
            return;
        }
    }
    // If all busy, steal the first voice
    auto& v = (*context_.sfxVoices)[0];
    v.stop();
    v.setBuffer(*buffer);
    float vol = std::max(0.f, std::min(100.f, context_.sfxVolume));
    v.setVolume(std::min(100.f, volume * (vol / 100.f)));
    v.play();
}

void AScene::playMusic(const char* path, bool loop, float volume) const
{
    if (!context_.music)
        return;
    context_.music->stop();
    if (context_.music->openFromFile(path)) {
        context_.music->setLoop(loop);
        float vol = std::max(0.f, std::min(100.f, context_.musicVolume));
        context_.music->setVolume(context_.musicEnabled ? std::min(100.f, volume * (vol / 100.f)) : 0.f);
        if (context_.musicEnabled)
            context_.music->play();
    }
}

} // namespace gomoku::scene