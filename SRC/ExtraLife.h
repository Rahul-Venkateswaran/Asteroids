#pragma once
#ifndef EXTRALIFE_H
#define EXTRALIFE_H

#include "GameObject.h"
#include "GameUtil.h"

class ExtraLife : public GameObject
{
public:
    ExtraLife();
    virtual ~ExtraLife() {}
    virtual void Update(int t) {} // No movement
    virtual void Render() { GameObject::Render(); } // Use default rendering
    void SetPosition(const GLVector3f& position) { mPosition = position; }
};

#endif