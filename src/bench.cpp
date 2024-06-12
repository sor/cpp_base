#include "global.h"

#include <benchmark/benchmark.h>
#include <Eigen/Core>

#include "entity0.h"
#include "entity1.h"

using namespace JanSordid::Core;

namespace JanSordid::ExampleDatamodel {

    constexpr bool AllAlive      = false;
    constexpr bool DoEarlyExit   = true;
    constexpr f32  TargetRange   = 5;

    void RenderThing(f32 posX, f32 posY) {
        benchmark::DoNotOptimize(posX);
        benchmark::DoNotOptimize(posY);
    }

#if defined( WIN32 ) && defined( _DEBUG )
    static_assert( sizeof(Entity0)      == 96 );
    static_assert( sizeof(Entity1)      == 96 );
    static_assert( sizeof(String)       == 40 );
    static_assert( sizeof(Vector<char>) == 32 );
#else
    static_assert( sizeof(Entity0)      == 88 );
    static_assert( sizeof(Entity1)      == 88 );
    static_assert( sizeof(String)       == 32 );
    static_assert( sizeof(Vector<char>) == 24 );
#endif

    static_assert( alignof(Entity0) ==  8 );
    static_assert( alignof(Entity1) ==  8 );
}

static void BM_SimulateEntity0( benchmark::State & state )
{
    using namespace JanSordid::ExampleDatamodel;

    const i64 size = state.range( 0 );

    srand( 7940 );

    using NTT = Entity0;
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

        for( NTT & e : es ) {
            e.Targeting<DoEarlyExit>(TargetRange, es);
        }

        for( NTT & e : es ) {
            e.Damaging(dt);
        }

        for( NTT & e : es ) {
            e.Respawn(dt);
        }

        for( NTT & e : es ) {
            e.Render();
        }
    }

    state.SetItemsProcessed( state.iterations() * state.range(0) );
}

static void BM_SimulateEntity123( benchmark::State & state )
{
    using namespace JanSordid::ExampleDatamodel;

    const i64 size = state.range( 0 );

    srand( 7940 );

    using NTT = Entity1;
    Vector<NTT> es(size);
    //Array<NTT,16> es;

    int ii = 0;
    // Init
    for( NTT & e : es ) {
        (AllAlive || (rand() % 10) == 0)
            ? e.setAlive()
            : e.setDead();
        e.unsetTarget();
        e.health    = e.isAlive() ? 100 : 0;
        e.posX      = (rand() % 401) - 200;
        e.posY      = (rand() % 401) - 200;
        e.velX      = ((rand() % 401) - 200) * 0.01;
        e.velY      = ((rand() % 401) - 200) * 0.01;
        e.name      = format("Entity #{}", ii);
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
            if (e.isAlive())
            {
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

        // Targeting / Damage / Respawn
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
        } for( NTT & e : es ) {
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
                    e.target->health -= 1;

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
                    e.health = 100;
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

constexpr long long fib( long long n )
{
    AssertInOptimized( n >= 0 );
    /*
    return (n == 0) ? 0
         : (n == 1) ? 1
         : fib( n - 1 )
         + fib( n - 2 );
    //*/
    //*
    if( n == 0 ) return 0;
    if( n == 1 ) return 1;
    return fib( n - 1 )
         + fib( n - 2 );
    //*/
}

static void BM_Fibonacci( benchmark::State & state )
{
    for( auto _ : state )
    {
        // This code gets timed
        const int64_t paramForFib = state.range( 0 );
        constexpr int64_t result = fib( 27 ); // muss zur CT ausgerechnet werden
        //benchmark::DoNotOptimize( result );
        int64_t result2 = fib( paramForFib );
        //benchmark::DoNotOptimize( result2 );
    }
}

/*
static void BM_MatrixMulF( benchmark::State & state )
{
    const usize size = state.range( 0 ); // Not used yet
    Eigen::MatrixXf a( size, size ), b( size, size );
    benchmark::DoNotOptimize( a );

    for( auto _ : state )
    {
        a *= b;
    }
}

static void BM_MatrixMulD( benchmark::State & state )
{
    const usize size = state.range( 0 ); // Not used yet
    Eigen::MatrixXd a( size, size ), b( size, size );
    benchmark::DoNotOptimize( a );

    for( auto _ : state )
    {
        a *= b;
    }
}
//*/

static void BM_CreateString( benchmark::State & state )
{
    const usize size = state.range( 0 );

    for( auto _ : state )
    {
        // Create strings with increasing size, via ctor( size, char )
        String sizedString( size,'x' );
        benchmark::DoNotOptimize( sizedString );
    }

    state.SetBytesProcessed( state.iterations() * state.range(0) );
}

// benchmark::ClobberMemory();

BENCHMARK( BM_SimulateEntity0      )->RangeMultiplier( 2 )->Range( 1<<4, 1<<16 );
BENCHMARK( BM_SimulateEntity123    )->RangeMultiplier( 2 )->Range( 1<<4, 1<<16 );


//        ->Range( 1<<4, 1<<20 )
        //->Complexity()
//        ;

/*
BENCHMARK( BM_MatrixMulF )
    ->Range( 1<<4, 1<<12 );

BENCHMARK( BM_MatrixMulD )
    ->Range( 1<<4, 1<<12 );
//*/

/*
BENCHMARK( BM_CreateString )
	->RangeMultiplier( 2 )
	->Range( 0, 1<<13 );

BENCHMARK( BM_Fibonacci )
    ->DenseRange( 0, 47, 1 );
//*/

BENCHMARK_MAIN();

// TODO: read up on doctest
