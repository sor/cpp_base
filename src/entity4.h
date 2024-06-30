#pragma once

#include "global.h"

namespace JanSordid::ExampleDataModel {
    struct Entity4Health {
        u16      health;

        bool     isAlive()   const { return health > 0; }
        bool     isVisible() const { return health > 0; }

        void     setAlive()          { health = 10; }
        void     setDead()           { health =  0; }
        void     doDamage(int value) { health -= value; }
    };
    struct Entity4Transform {
        f32      posX, posY;
        f32      velX, velY;
    };
    struct Entity4Target {
        static constexpr u32 invalidID = std::numeric_limits<u32>::max();

        u32      targetID;

        bool     hasTarget() const { return targetID != invalidID; }

        void     setTarget(u32 value) { targetID = value;     }
        void     unsetTarget()        { targetID = invalidID; }
    };
    struct Entity4Cooldown {
        f32      cd;

        // TODO: Asserts don't work anymore *LOL*
        bool     isCooldownDone() const { /*Assert( isAlive());*/ return cd <= 0; }
        bool     canRespawn()     const { /*Assert(!isAlive());*/ return cd <= 0; }

        void     progressCooldown(const f32 dt)    { /*Assert( isAlive());*/ cd -= dt; }
        void     progressRespawn (const f32 dt)    { /*Assert(!isAlive());*/ cd -= dt; }
        void     setCooldown     (const f32 value) { /*Assert( isAlive());*/ cd  = value; }
        void     setRespawn      (const f32 value) { /*Assert(!isAlive());*/ cd  = value; }
    };
    struct Entity4Cold {
        string   name;
        u16      xp;
    };

    // Possible reunification in a single data structure to make working with it DoD'ed data easier
    struct Entity4 {
        static Vector<Entity4Health>    * ehp;
        static Vector<Entity4Transform> * etf;
        static Vector<Entity4Target>    * etg;
        static Vector<Entity4Cooldown>  * ecd;
        static Vector<Entity4Cold>      * ecl;

        u32 id;
        bool     isAlive()   const { return (*ehp)[id].isAlive();   }
        bool     hasTarget() const { return (*etg)[id].hasTarget(); }
        // etc
    };

    // Possible solution for iteration
    struct Entity4PseudoContainerIterator {
        u32 startId;
        u32 endId;
    };

    // Non DoD'ed ECS
    struct MyECSDynEntity {
        //Vector<Component*> compos; // The components would need to derive from Component
    };

    struct MyECSEntity {
        Entity4Health       hp;
        Entity4Transform    transform;
        Entity4Target       target;
        Entity4Cooldown     cd;
        Entity4Cold         cold; // Not really ECS compatible, must be split to make sense
    };

    struct MyECSEntity2 {
        Entity4Health       hp;
        Entity4Transform    transform;
        Entity4Target       target;
        Entity4Cooldown     cd;
        //Entity4AI           ai; // Does not exist, but if it would, then this would not be commented out
    };
}
