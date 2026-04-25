#include "entity0.hpp"

namespace JanSordid::ExampleDataModel
{
    // Forward decl
    void RenderThing(f32 posX, f32 posY);

    f32 Entity0::calcDistTo(const Self & other) const {
        const f32 diffX = posX - other.posX;
        const f32 diffY = posY - other.posY;
        const f32 dist  = sqrt(diffX * diffX + diffY * diffY); // TODO sqrtf
        return dist;
    }

    f32 Entity0::calcSqDistTo(const Self & other) const {
        const f32 diffX = posX - other.posX;
        const f32 diffY = posY - other.posY;
        const f32 dist  = diffX * diffX + diffY * diffY;
        return dist;
    }

    void Entity0::Init(const bool allAlive, const int ii) {
        (allAlive || (rand() % 10) == 0)
            ? setAlive()
            : setDead();
        health = isAlive() ? 10 : 0;
        posX   = ((rand() % 401) - 200);
        posY   = ((rand() % 401) - 200);
        velX   = ((rand() % 401) - 200) * 0.01;
        velY   = ((rand() % 401) - 200) * 0.01;
        name   = format("Entity #{}", ii);
        unsetTarget();
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

	void Entity0::TargetingNeedsReset(const f32 targetRange, DynArray<Self> & es) // this Vector can not be const :(
	{
		// Determine if targeting needs to be reset
		if (!hasTarget())
			return;

		if (target->isAlive()) {

			const f32 dist = calcDistTo( *target );

			if (dist > targetRange) {
				// Target moved out of range
				unsetTarget();
			}
		} else {
			// Target dead
			unsetTarget();
		}
	}

	template<bool DoEarlyExit>
	void Entity0::Targeting(const f32 targetRange, DynArray<Self> & es) // this Vector can not be const :(
	{
		// If no target, find a new target
		if (hasTarget())
			return;

		f32    closestDist   = INFINITY;
		Self * closestTarget = nullptr;

		//if(0)
		for ( Self & o : es ) {
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

			// Early exit, HACK: hotfix
			if constexpr (DoEarlyExit)
				if (closestDist <= targetRange)
					break;
		}

		// New viable target
		if (closestDist <= targetRange) {
			setTarget(closestTarget);
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
                target->doDamage( 1 );

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
                posX   = (rand() % 401) - 200;
                posY   = (rand() % 401) - 200;
                unsetTarget();
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

	template void Entity0::Targeting<false>(const f32 TargetRange, DynArray<Self> &es);
	template void Entity0::Targeting<true> (const f32 TargetRange, DynArray<Self> &es);
}

