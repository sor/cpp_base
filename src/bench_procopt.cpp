#include "bench.hpp"

#include "entity1.hpp"
#include "entity2.hpp"
#include "entity3.hpp"

namespace JanSordid::ExampleDataModel
{
	using namespace JanSordid::Core;

	template <typename NTT>
	void BM_SimulateProcOpt( benchmark::State & state )
	{
		const i64 size = state.range( 0 );

		srand( 7940 );

		//using NTT = Entity1;
		DynArray<NTT> es(size);
		//Array<NTT,16> es;

		int ii = 0;
		// Init
		for( NTT & e : es ) {
			(AllAlive || (rand() % 10) == 0)
				? e.setAlive()
				: e.setDead();
			e.posX      = ((rand() % 401) - 200);
			e.posY      = ((rand() % 401) - 200);
			e.velX      = ((rand() % 401) - 200) * 0.01;
			e.velY      = ((rand() % 401) - 200) * 0.01;
			e.name      = format("Entity #{}", ii);
			e.unsetTarget();
			if( e.isAlive() )
				e.setCooldown( 1 );
			else
				e.setRespawn( ((rand() % 100) + 1) * 0.1 ); // 0.1-10s respawn timer

			++ii;
		}

		for( auto _ : state )
		{
			const f32 ddt = ((rand()%201) - 100) * 0.00001; // simulate flux +/- 0.001
			const f32 dt  = 0.01666 + ddt;                  // ~60fps

			// Movement
			//#pragma clang loop unroll_count(8)
			//#pragma clang loop vectorize(enable)
			for( NTT & e : es ) {
				if( e.isAlive() ) {
					// Move
					e.posX += e.velX * dt;
					e.posY += e.velY * dt;

					// Limit to playfield, x and y in (-200 .. 200)
					if( e.posX < -200 ) {
						// e.g.   -(  -205 + 200) * 2 == 10;
						e.posX += -(e.posX + 200) * 2;
						e.velX *= -1;
					}
					else if( e.posX > 200 ) {
						// e.g.   (   205 - 200) * 2 == 10;
						e.posX -= (e.posX - 200) * 2;
						e.velX *= -1;
					}

					if( e.posY < -200 ) {
						// e.g.   -(  -205 + 200) * 2 == 10;
						e.posY += -(e.posY + 200) * 2;
						e.velY *= -1;
					}
					else if( e.posY > 200 ) {
						// e.g.   (   205 - 200) * 2 == 10;
						e.posY -= (e.posY - 200) * 2;
						e.velY *= -1;
					}
				}
			}
			//benchmark::ClobberMemory();

			// Targeting
			if( !SkipTargeting )
			for( NTT & e : es ) {
				// Does targeting need reset?
				if( e.hasTarget() ) {
					if( e.target->isAlive() ) {

						const f32 diffX = e.posX - e.target->posX;
						const f32 diffY = e.posY - e.target->posY;
						const f32 distSq = diffX * diffX + diffY * diffY; // Squared still

						if( distSq > TargetRangeSq ) {
							// Target moved out of range
							e.unsetTarget();
						}
					} else {
						// Target dead
						e.unsetTarget();
					}
				}
			//} for( NTT & e : es ) {
				// Find new target
				if( !e.hasTarget() ) {
					f32   closestDist   = INFINITY;
					NTT * closestTarget = nullptr;

					for( NTT & o: es ) {
						if( !o.isAlive() )
							continue;

						if( &e == &o ) [[unlikely]]
							continue;

						const f32 diffX = e.posX - o.posX;
						const f32 diffY = e.posY - o.posY;
						const f32 distSq= diffX * diffX + diffY * diffY; // Squared still

						// Remember new best target
						if( closestDist   > distSq ) {
							closestDist   = distSq;
							closestTarget = &o; // safe?
						}

						// Early exit
						if constexpr (DoEarlyExit)
							if (closestDist <= TargetRangeSq)
								break;
					}

					// New viable target
					if( closestDist <= TargetRangeSq ) {
						e.setTarget( closestTarget );
					}
				}
			}

			// Damage / Respawn
			for( NTT & e : es ) {
				// Whatever cooldown
				e.progressCooldown( dt );

				// Damaging
				if( e.isAlive() ) {
					// Damage the target, if attack-cooldown has elapsed and target is alive (why need to check again?)
					if (   e.isCooldownDone()
						&& e.hasTarget()
						&& e.target->isAlive())
					{
						// Hit
						e.setCooldown( 1 );
						e.target->doDamage( 1 );

						// Killed?
						if (e.target->health == 0) {
							e.xp += 1;
							e.target->setDead();
							e.target->setRespawn( 10 );

							IfDebug
								print(
									"{0} has killed {1}. {0} gets {2} xp and now has {3} xp total.\r",
									e.name,
									e.target->name,
									1,
									e.xp);

							e.unsetTarget();
						}
					}
				}
				// Respawn
				//if( !e.isAlive() ) {
				else {
					if( e.canRespawn() ) {
						// Respawn timer elapsed, do respawn
						e.setAlive();
						e.posX   = (rand() % 401) - 200;
						e.posY   = (rand() % 401) - 200;
						e.unsetTarget();
						e.setCooldown( 1 );

						IfDebug
							print(
								"{0} has respawned at {1} {2}.\r",
								e.name,
								e.posX,
								e.posY);
					}
				}
			}

			for( NTT & e : es ) {    // this is slower than the for loop below on GCC with LTO enabled
			//for( int i = 0; i < size; ++i ) { NTT & e = es[i];
				if( e.isAlive() ) {
					RenderThing( e.posX, e.posY );
				}
			}
		}

		state.SetItemsProcessed( state.iterations() * state.range(0) );
	}

	template void BM_SimulateProcOpt<Entity1>( benchmark::State & state );
	template void BM_SimulateProcOpt<Entity2>( benchmark::State & state );
	template void BM_SimulateProcOpt<Entity3>( benchmark::State & state );
}
