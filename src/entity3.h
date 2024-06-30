#pragma once

#include "global.h"

namespace JanSordid::ExampleDataModel {
    struct Entity3 {
        using    Self = Entity3;

        string   name;

        Self *   target;

        f32      posX, posY;
        f32      velX, velY;

        f32      cd;

        u16      health;
        u16      xp;






        bool     isAlive()   const { return health != 0; }
        bool     isVisible() const { return health != 0; }
        bool     hasTarget() const { return target != nullptr; }

        void     setAlive()                   { health = 10; }
        void     setDead()                    { health =  0; }
        void     doDamage(int value)          { health -= value; }
        void     setVisible(const bool value) {}
        void     setTarget(Self * value)      { target = value;   }
        void     unsetTarget()                { target = nullptr; }

        bool     isCooldownDone() const { Assert( isAlive()); return cd <= 0; }
        bool     canRespawn()     const { Assert(!isAlive()); return cd <= 0; }

        void     progressCooldown(const f32 dt)    { Assert( isAlive()); cd -= dt; }
        void     progressRespawn (const f32 dt)    { Assert(!isAlive()); cd -= dt; }
        void     setCooldown     (const f32 value) { Assert( isAlive()); cd  = value; }
        void     setRespawn      (const f32 value) { Assert(!isAlive()); cd  = value; }
    };
}
