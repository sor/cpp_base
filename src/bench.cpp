#include "global.hpp"

#include <benchmark/benchmark.h>
#include <Eigen/Core>

#include "entity0.h"
#include "entity1.h"
#include "entity2.h"
#include "entity3.h"
#include "entity4.h"

namespace JanSordid::ExampleDataModel
{
	using namespace JanSordid::Core;

#pragma region Bonus
	// Either we have size 16 and align 8, or with pack(4) have size 12
	//  and align 4, but not size 12 with align 8?
	// This way a consecutive Align4or8 would have 4 padding in between,
	//  but a <=32 bit struct could fit as shown in Align8AndColor
	//  (with workaround `alignas(8)` at use site)
	#pragma pack(push, 4)
	//#pragma align(8) // <- this does not work
	struct Align4or8 {
		void * ptr; // Assumes 64bit system
		f32    f;
	};
	#pragma pack(pop)

	struct Color {
		u8 r, g, b;
	};

	struct Align8AndColor {
		alignas( 8 )
		Align4or8 a8;
		Color     c;
	};

	static_assert(  sizeof(Align4or8) == 12 );
	static_assert( alignof(Align4or8) ==  4 );

	static_assert(  sizeof(Color) == 3 );
	static_assert( alignof(Color) == 1 );

	static_assert(  sizeof(Align8AndColor) == 16 );
	static_assert( alignof(Align8AndColor) ==  8 );
#pragma endregion

	constexpr bool AllAlive      = false;
	constexpr bool SkipTargeting = false;
	constexpr bool DoEarlyExit   = false;
	constexpr f32  TargetRange   = 5;
	constexpr f32  TargetRangeSq = TargetRange * TargetRange;

	void RenderThing(f32 posX, f32 posY) {
		benchmark::DoNotOptimize(posX);
		benchmark::DoNotOptimize(posY);
	}

#pragma region SA NTT0-3
#if defined( WIN32 ) && defined( _DEBUG )
	static_assert( sizeof(String)       == 40 );
	static_assert( sizeof(Vector<char>) == 32 );

	static_assert( sizeof(Entity0)      == 96 );
	static_assert( sizeof(Entity1)      == 96 );
//	static_assert( sizeof(Entity2)      == 80 );
//	static_assert( sizeof(Entity3)      == 72 );
#else
	static_assert( sizeof(String)       == 32 );
	static_assert( sizeof(Vector<char>) == 24 );

	static_assert( sizeof(Entity0)      == 88 );
	static_assert( sizeof(Entity1)      == 88 );
	static_assert( sizeof(Entity2)      == 72 );
	static_assert( sizeof(Entity3)      == 64 );
#endif

	static_assert( alignof(Entity0) ==  8 );
	static_assert( alignof(Entity1) ==  8 );
	static_assert( alignof(Entity2) ==  8 );
	static_assert( alignof(Entity3) ==  8 );
#pragma endregion

#pragma region SA NTT4
	static_assert(  sizeof(Entity4Health)    ==  2 );   // 32 per Cacheline
	static_assert( alignof(Entity4Health)    ==  2 );
	static_assert(  sizeof(Entity4Transform) == 16 );   //  4 per Cacheline
	static_assert( alignof(Entity4Transform) ==  4 );
	static_assert(  sizeof(Entity4Target)    ==  4 );   // 16 per Cacheline, a pointer would have been size 8 / 8 per Cacheline
	static_assert( alignof(Entity4Target)    ==  4 );
	static_assert(  sizeof(Entity4Cooldown)  ==  4 );   // 16 per Cacheline
	static_assert( alignof(Entity4Cooldown)  ==  4 );
	static_assert(  sizeof(Entity4Cold)      == 40 );   // 1.6 per Cacheline
	static_assert( alignof(Entity4Cold)      ==  8 );
#pragma endregion

	template <typename NTT>
	static void BM_SimulateOOP( benchmark::State & state )
	{
		const i64 size = state.range( 0 );

		srand( 7940 );

		//using NTT = Entity0;
		Vector<NTT> es(size);

		int ii = 0;
		// Init
		for( NTT & e : es ) {
			e.Init(AllAlive, ii);

			++ii;
		}

		for( auto _ : state )
		{
			const f32 ddt = ((rand()%201) - 100) * 0.00001; // simulate flux +/- 0.001
			const f32 dt  = 0.01666 + ddt;                  // ~60fps

			for( NTT & e : es ) {
				e.Move(dt);
			}

			if(!SkipTargeting) {
				for (NTT &e: es) {
					e.template Targeting<DoEarlyExit>(TargetRange, es);
				}
			}

			for( NTT & e : es ) {
				e.Damaging(dt);
				e.Respawn(dt);
			}

			for( NTT & e : es ) {
				e.Render();
			}
		}

		state.SetItemsProcessed( state.iterations() * state.range(0) );
	}

	template <typename NTT>
	static void BM_SimulateProc( benchmark::State & state )
	{
		const i64 size = state.range( 0 );

		srand( 7940 );

		//using NTT = Entity1;
		Vector<NTT> es(size);
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

			//int aliveCount = 0, deadCount  = 0;

			// Movement
			//#pragma clang loop unroll_count(8)
			//#pragma clang loop vectorize(enable)
			for( NTT & e : es ) {
				if (e.isAlive())
				{
					//aliveCount++;

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
				//else deadCount++;
			}

			//print( "{0:0.3},", (f32)aliveCount / (aliveCount+deadCount) * 100 );

			// Targeting
			if(!SkipTargeting)
			for( NTT & e : es ) {
				// Targeting
				if (e.hasTarget()) {
					if (e.target->isAlive()) {

						const f32 diffX = e.posX - e.target->posX;
						const f32 diffY = e.posY - e.target->posY;
						const f32 dist  = sqrt(diffX * diffX + diffY * diffY);

						if (dist > TargetRange) {
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
				if (!e.hasTarget()) {
					f32   closestDist   = INFINITY;
					NTT * closestTarget = nullptr;

					for( NTT & o: es ) {
						if (&e == &o)
							continue;

						if (!o.isAlive())
							continue;

						const f32 diffX = e.posX - o.posX;
						const f32 diffY = e.posY - o.posY;
						const f32 dist  = sqrt(diffX * diffX + diffY * diffY);
						//const f32 dist  = diffX * diffX + diffY * diffY;

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
						e.setTarget( closestTarget );
					}
				}
			}

			// Damage / Respawn
			for( NTT & e : es ) {
				// Damaging
				if( e.isAlive() ) {
					e.progressCooldown( dt );

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
			//} for( NTT & e : es ) {
				// Respawn
				if( !e.isAlive() ) {
					e.progressRespawn( dt );

					//if (e.respawn <= 0) {
					if (e.canRespawn()) {
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
				e.setVisible(e.isAlive());
				if(e.isVisible())
				{
					RenderThing(e.posX, e.posY);
				}
			}
		}

		state.SetItemsProcessed( state.iterations() * state.range(0) );
	}

	template <typename NTT>
	static void BM_SimulateProcOpt( benchmark::State & state )
	{
		const i64 size = state.range( 0 );

		srand( 7940 );

		//using NTT = Entity1;
		Vector<NTT> es(size);
		//Array<NTT,16> es;

		int ii = 0;
		// Init
		for( NTT & e : es ) {
			(AllAlive || (rand() % 10) == 0)
				? e.setAlive()
				: e.setDead();
			e.posX      = (rand() % 401) - 200;
			e.posY      = (rand() % 401) - 200;
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
				if (e.isAlive()) {
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
			if(!SkipTargeting)
			for( NTT & e : es ) {
				// Targeting
				if (e.hasTarget()) {
					if (e.target->isAlive()) {

						const f32 diffX = e.posX - e.target->posX;
						const f32 diffY = e.posY - e.target->posY;
						const f32 dist  = diffX * diffX + diffY * diffY; // Squared still

						if (dist > TargetRangeSq) {
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
				if (!e.hasTarget()) {
					f32   closestDist   = INFINITY;
					NTT * closestTarget = nullptr;

					for( NTT & o: es ) {
						if (!o.isAlive())
							continue;

						if (&e == &o) [[unlikely]]
							continue;

						const f32 diffX = e.posX - o.posX;
						const f32 diffY = e.posY - o.posY;
						const f32 dist  = diffX * diffX + diffY * diffY; // Squared still

						// Remember new best target
						if (closestDist   > dist) {
							closestDist   = dist;
							closestTarget = &o; // safe?
						}

						// Early exit
						if constexpr (DoEarlyExit)
							if (closestDist <= TargetRangeSq)
								break;
					}

					// New viable target
					if (closestDist <= TargetRangeSq) {
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
					if (e.canRespawn()) {
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
				if(e.isAlive()) {
					RenderThing(e.posX, e.posY);
				}
			}
		}

		state.SetItemsProcessed( state.iterations() * state.range(0) );
	}

	static void BM_SimulateDOD( benchmark::State & state )
	{
		const i64 size = state.range( 0 );

		srand( 7940 );

		//Array<Entity,16> es;
		Vector<Entity4Health>    ehp(size);
		Vector<Entity4Transform> etf(size);
		Vector<Entity4Target>    etg(size);
		Vector<Entity4Cooldown>  ecd(size);
		Vector<Entity4Cold>      ecl(size);

		// Init
		for( int i = 0; i < size; ++i ) {
			(AllAlive || (rand() % 10) == 0)
				? ehp[i].setAlive()
				: ehp[i].setDead();
			etf[i].posX = (rand() % 401) - 200;
			etf[i].posY = (rand() % 401) - 200;
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
					if (!etg[i].hasTarget()) {
						f32 closestDist = INFINITY;
						u32 closestTargetID = Entity4Target::invalidID;

						for (int oi = 0; oi < size; ++oi) {
							if (!ehp[oi].isAlive())
								continue;

							if (i == oi) [[unlikely]]
								continue;

							const f32 diffX = etf[i].posX - etf[oi].posX;
							const f32 diffY = etf[i].posY - etf[oi].posY;
							const f32 dist = diffX * diffX + diffY * diffY;

							// Remember new best target
							if (closestDist > dist) {
								closestDist = dist;
								closestTargetID = oi;
							}

							// Early exit
							if constexpr (DoEarlyExit)
								if (closestDist <= TargetRangeSq)
									break;
						}

						// New viable target
						if (closestDist <= TargetRangeSq) {
							etg[i].setTarget(closestTargetID);
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

	// benchmark::ClobberMemory();
	//*
	BENCHMARK( BM_SimulateOOP<Entity0>     )->RangeMultiplier( 2 )->Range( 1<<4, 1<<15 );
	BENCHMARK( BM_SimulateProc<Entity1>    )->RangeMultiplier( 2 )->Range( 1<<4, 1<<15 );
	BENCHMARK( BM_SimulateProc<Entity2>    )->RangeMultiplier( 2 )->Range( 1<<4, 1<<15 );
	BENCHMARK( BM_SimulateProc<Entity3>    )->RangeMultiplier( 2 )->Range( 1<<4, 1<<15 );
	BENCHMARK( BM_SimulateProcOpt<Entity2> )->RangeMultiplier( 2 )->Range( 1<<4, 1<<15 );
	BENCHMARK( BM_SimulateProcOpt<Entity3> )->RangeMultiplier( 2 )->Range( 1<<4, 1<<15 );
	BENCHMARK( BM_SimulateDOD              )->RangeMultiplier( 2 )->Range( 1<<4, 1<<15 );
	//*/
}

// Benchmarks below have rather long compilation time, if not needed comment the block out
//*
namespace JanSordid::ExampleBenchmark {

	using namespace JanSordid::Core;

	constexpr i64 fib( i64 n )
	{
		Assert( n >= 0 );

		return (n == 0) ? 0
			 : (n == 1) ? 1
			 : fib( n - 1 )
			 + fib( n - 2 );

		//if( n == 0 ) return 0;
		//if( n == 1 ) return 1;
		//return fib( n - 1 )
		//     + fib( n - 2 );
	}

	static void BM_Fibonacci( benchmark::State & state )
	{
		const i64 paramForFib = state.range( 0 );
		for( auto _ : state )
		{
			// This code gets timed
			//constexpr i64 result = fib( 27 ); // muss zur CT ausgerechnet werden
			//benchmark::DoNotOptimize( result );

			i64 result2 = fib( paramForFib );
			benchmark::DoNotOptimize( result2 );
		}
	}

	static void BM_MatrixMulF( benchmark::State & state )
	{
		//_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
		//_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

		//fesetenv(FE_DFL_DISABLE_SSE_DENORMS_ENV);

		//_mm_setcsr( _mm_getcsr() | (1<<15) | (1<<6) );

		const i64 size = state.range( 0 );
		Eigen::MatrixXf a( size, size ), b( size, size );
		//benchmark::DoNotOptimize( a );

		for( auto _ : state )
		{
			a *= b;
		}
	}

	static void BM_MatrixMulD( benchmark::State & state )
	{
		const i64 size = state.range( 0 ); // Not used yet
		Eigen::MatrixXd a(size, size ), b( size, size );
		benchmark::DoNotOptimize( a );

		for( auto _ : state )
		{
			a *= b;
		}
	}

	static void BM_CreateString( benchmark::State & state )
	{
		const i64 size = state.range( 0 );

		for( auto _ : state )
		{
			// Create strings with increasing size, via ctor( size, char )
			String sizedString( size,'x' );
			benchmark::DoNotOptimize( sizedString );
		}

		state.SetBytesProcessed( state.iterations() * state.range(0) );
	}

	BENCHMARK( BM_MatrixMulF )
		->RangeMultiplier( 2 )
		->Range( 1<<4, 1<<12 );

	BENCHMARK( BM_MatrixMulD )
		->Range( 1<<4, 1<<12 );

	/*BENCHMARK( BM_CreateString )
		->RangeMultiplier( 2 )
		->Range( 0, 1<<13 );
	*/
	BENCHMARK( BM_Fibonacci )
		->DenseRange( 0, 47, 1 );
}
//*/

BENCHMARK_MAIN();

// TODO: read up on doctest
