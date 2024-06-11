#include "entity0.h"

namespace JanSordid::ExampleDatamodel {

    // Forward decl
    void RenderThing(f32 posX, f32 posY);

    f32 Entity0::calcDistTo(const Self & other) const {
        const f32 diffX = posX - other.posX;
        const f32 diffY = posY - other.posY;
        const f32 dist  = sqrt(diffX * diffX + diffY * diffY); // TODO sqrtf
        return dist;
    }

    void Entity0::Init(const bool AllAlive, const int ii) {
        (AllAlive || (rand() % 10) == 0)
            ? setAlive()
            : setDead();
        unsetTarget();
        health      = isAlive() ? 100 : 0;
        posX        = (rand() % 401) - 200;
        posY        = (rand() % 401) - 200;
        velX        = ((rand() % 401) - 200) * 0.01;
        velY        = ((rand() % 401) - 200) * 0.01;
        name        = format("Entity #{}", ii);
        if( isAlive() )
            setCooldown( 1 );
        else
            setRespawn( ((rand() % 100) + 1) * 0.1 ); // 0.1-10s respawn timer
    }

    void Entity0::Move(const f32 dt) {
        if (isAlive()) {
            // Move
            posX += velX * dt;
            posY += velY * dt;

            // Limit to playfield, x and y in (-200 .. 200)
            if (posX < -200) {
                // e.g. -(-205 + 200) * 2 == 10;
                posX += -(posX + 200) * 2;
                velX *= -1;
            } else if (posX > 200) {
                // e.g. ( 205 - 200) * 2 == 10;
                posX -= (posX - 200) * 2;
                velX *= -1;
            }

            if (posY < -200) {
                // e.g. -(-205 + 200) * 2 == 10;
                posY += -(posY + 200) * 2;
                velY *= -1;
            } else if (posY > 200) {
                // e.g. ( 205 - 200) * 2 == 10;
                posY -= (posY - 200) * 2;
                velY *= -1;
            }
        }
    }

    template<bool DoEarlyExit>
    void Entity0::Targeting(const f32 TargetRange, Vector<Self> & es) // this Vector can not be const :(
    {
        // Targeting
        if (hasTarget()) {
            if (target->isAlive()) {

                const f32 dist = calcDistTo( *target );

                if (dist > TargetRange) {
                    // Target moved out of range
                    unsetTarget();
                }
            } else {
                // Target dead
                unsetTarget();
            }
        }

        // Find new target
        if (!hasTarget()) {
            f32    closestDist   = INFINITY;
            Self * closestTarget = nullptr;

            //if(0)
            for ( Self & o: es ) {
                if (this == &o)
                    continue;

                if (!o.isAlive())
                    continue;

                const f32 dist = calcDistTo( o );

                // Remember new best target
                if (closestDist   > dist) {
                    closestDist   = dist;
                    closestTarget = &o; // safe?
                }

                // Early exit
                if constexpr (DoEarlyExit)
                    if (closestDist <= TargetRange)
                        break;
            }

            // New viable target
            if (closestDist <= TargetRange) {
                setTarget(closestTarget);
            }
        }
    }

    void Entity0::Damaging(const f32 dt) {
        if (isAlive()) {
            progressCooldown( dt );

            // Damage the target, if attack-cooldown has elapsed and target is alive (why need to check again?)
            if (   isCooldownDone()
                && hasTarget()
                && target->isAlive())
            {
                // Hit
                setCooldown( 1 );
                target->health -= 1;

                // Killed?
                if (target->health == 0) {
                    xp += 1;
                    target->setDead();
                    target->setRespawn( 10 );

                    IfDebug
                        print(
                            "{0} has killed {1}. {0} gets {2} xp and now has {3} xp total.\r",
                            name,
                            target->name,
                            1,
                            xp);

                    unsetTarget();
                }
            }
        }
    }

    void Entity0::Respawn(const f32 dt) {
        if (!isAlive()) {
            progressRespawn( dt );

            if (canRespawn()) {
                // Respawn timer elapsed, do respawn
                setAlive();
                unsetTarget();
                health = 100;
                posX   = (rand() % 401) - 200;
                posY   = (rand() % 401) - 200;
                setCooldown(1);

                IfDebug
                    print(
                        "{0} has respawned at {1} {2}.\r",
                        name,
                        posX,
                        posY);
            }
        }
    }

    void Entity0::Render() {
        setVisible(isAlive());
        if (isVisible()) {
            RenderThing(posX, posY);
        }
    }

    template void Entity0::Targeting<false>(const f32 TargetRange, Vector<Self> &es);
    template void Entity0::Targeting<true> (const f32 TargetRange, Vector<Self> &es);
}
