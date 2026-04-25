#include "bench.hpp"

#include "entity4.hpp"

namespace JanSordid::ExampleDataModel
{
	using namespace JanSordid::Core;

	void BM_SimulateDOD( benchmark::State & state )
	{
		const i64 size = state.range( 0 );

		srand( 7940 );

		//Array<Entity,16> es;
		DynArray<Entity4Health>    ehp(size);
		DynArray<Entity4Transform> etf(size);
		DynArray<Entity4Target>    etg(size);
		DynArray<Entity4Cooldown>  ecd(size);
		DynArray<Entity4Cold>      ecl(size);

		// Init
		for( int i = 0; i < size; ++i ) {
			(AllAlive || (rand() % 10) == 0)
				? ehp[i].setAlive()
				: ehp[i].setDead();
			etf[i].posX = ((rand() % 401) - 200);
			etf[i].posY = ((rand() % 401) - 200);
			etf[i].velX = ((rand() % 401) - 200) * 0.01;
			etf[i].velY = ((rand() % 401) - 200) * 0.01;
			ecl[i].name = format("Entity #{}", i);
			etg[i].unsetTarget();
			if( !ehp[i].isAlive() )
				ecd[i].setCooldown( 1 );
			else
				ecd[i].setRespawn( ((rand()%100) + 1) * 0.1 ); // 0.1-10s respawn timer
		}

		for( auto _ : state )
		{
			const f32 ddt = ((rand()%201) - 100) * 0.00001; // simulate flux +/- 0.001
			const f32 dt  = 0.01666 + ddt;                  // ~60fps

			// Movement
			//#pragma clang loop unroll_count(8)
			//#pragma clang loop vectorize(enable)
			//#pragma omp for simd
			for( int i = 0; i < size; ++i ) {
				if (ehp[i].isAlive())
				{
					auto & e = etf[i];
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
			if(!SkipTargeting) {
				for (int i = 0; i < size; ++i) {

					// Old target still in range?
					if (etg[i].hasTarget()) {
						const u32 ti = etg[i].targetID;
						if (ehp[ti].isAlive()) {

							const f32 diffX = etf[i].posX - etf[ti].posX;
							const f32 diffY = etf[i].posY - etf[ti].posY;
							const f32 dist = diffX * diffX + diffY * diffY;

							if (dist > TargetRangeSq) {
								// Target moved out of range
								etg[i].unsetTarget();
							}
						} else {
							// Target dead
							etg[i].unsetTarget();
						}
					}

					// Find new target
					if( !etg[i].hasTarget() ) {
						f32 closestDist = INFINITY;
						u32 closestTargetID = Entity4Target::InvalidID;

						for( int oi = 0; oi < size; ++oi ) {
							if( !ehp[oi].isAlive() )
								continue;

							if( i == oi ) [[unlikely]]
								continue;

							const f32 diffX = etf[i].posX - etf[oi].posX;
							const f32 diffY = etf[i].posY - etf[oi].posY;
							const f32 dist = diffX * diffX + diffY * diffY;

							// Remember new best target
							if( closestDist > dist ) {
								closestDist = dist;
								closestTargetID = oi;
							}

							// Early exit
							if constexpr( DoEarlyExit )
								if (closestDist <= TargetRangeSq)
									break;
						}

						// New viable target
						if( closestDist <= TargetRangeSq ) {
							etg[i].setTarget( closestTargetID );
						}
					}
				}
			}

			// Damage / Respawn
			for( int i = 0; i < size; ++i ) {

				ecd[i].progressCooldown(dt);

				// Damaging
				if( ehp[i].isAlive() ) {

					// Damage the target, if attack-cooldown has elapsed and target is alive (why need to check again?)
					const u32 ti = etg[i].targetID;
					if (   ecd[i].isCooldownDone()
						   && etg[i].hasTarget()
						   && ehp[ti].isAlive())
					{
						// Hit
						ecd[i].setCooldown( 1 );
						ehp[ti].doDamage( 1 );

						// Killed?
						if( ehp[ti].isAlive() == false ) {
							ecl[i].xp += 1;
							ecd[ti].setRespawn( 10 );

							IfDebug
								print(
									"{0} has killed {1}. {0} gets {2} xp and now has {3} xp total.\r",
									ecl[i].name,
									ecl[ti].name,
									1,
									ecl[i].xp);

							etg[i].unsetTarget();
						}
					}
				}

				// Respawn
				//if( ehp[i].isAlive() == false ) {
				else {

					if( ecd[i].canRespawn() ) {
						// Respawn timer elapsed
						ehp[i].setAlive();
						etf[i].posX = (rand() % 401) - 200;
						etf[i].posY = (rand() % 401) - 200;
						etg[i].unsetTarget();
						ecd[i].setCooldown( 1 );

						IfDebug
							print(
								"{0} has respawned at {1} {2}.\r",
								ecl[i].name,
								etf[i].posX,
								etf[i].posY);
					}
				}
			}

			for( int i = 0; i < size; ++i ) {
				if(ehp[i].isAlive()) {
					RenderThing(etf[i].posX, etf[i].posY);
				}
			}
		}

		state.SetItemsProcessed( state.iterations() * state.range(0) );
	}
}
