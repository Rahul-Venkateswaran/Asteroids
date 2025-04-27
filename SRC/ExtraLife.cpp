#include "ExtraLife.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "Sprite.h"

ExtraLife::ExtraLife() : GameObject("ExtraLife")
{
    Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("heart");
    shared_ptr<Sprite> sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
    SetSprite(sprite);
    SetScale(0.05f);
}