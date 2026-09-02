#include "uhepch.h"
#include "SpriteAnimation.h"

namespace UHE
{

void SpriteAnimation::StartAnimation()
{
    Playing = true;
    CurrentFrameIndex = 0;
    Timer = 0.0f;
}

void SpriteAnimation::StopAnimation()
{
    Playing = false;
    CurrentFrameIndex = 0;
    Timer = 0.0f;
}

void SpriteAnimation::Tick(float deltaTime)
{
    if (Playing && FrameDuration > 0.0f && FrameCount > 0)
    {
        Timer += deltaTime;
        if (Timer >= FrameDuration)
        {
            Timer -= FrameDuration;
            CurrentFrameIndex++;
            if (CurrentFrameIndex >= FrameCount)
            {
                CurrentFrameIndex = Loop ? 0 : FrameCount - 1;
                if (!Loop)
                {
                    Playing = false;
                }
            }
        }
    }
}

Ref<SubTexture2D> SpriteAnimation::GetCurrentFrame() const
{
    if (!SpriteSheet || FrameCount <= 0 || FrameSize.x <= 0 || FrameSize.y <= 0)
        return nullptr;

    auto texWidth = (f32)SpriteSheet->GetWidth();
    auto texHeight = (f32)SpriteSheet->GetHeight();
    i32 cols = (int)(texWidth / FrameSize.x);
    if (cols <= 0)
        cols = 1;

    i32 col = CurrentFrameIndex % cols;
    i32 row = CurrentFrameIndex / cols;

    // rrame size Y-flip row=0 is top visibiliyo we flip it for OpenGL texture coordinates
    float flippedY = texHeight - (row * FrameSize.y) - FrameSize.y;

    glm::vec2 min = {(col * FrameSize.x) / texWidth, flippedY / texHeight};
    glm::vec2 max = {((col + 1) * FrameSize.x) / texWidth, (flippedY + FrameSize.y) / texHeight};

    return CreateRef<SubTexture2D>(SpriteSheet, min, max);
}
} // namespace UHE
