#include "global.h"

#include <benchmark/benchmark.h>
#include <Eigen/Core>

using namespace JanSordid::Core;

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

static void BM_CreateString( benchmark::State & state )
{
    const usize size = state.range( 0 );

    for( auto _ : state )
    {
        // Create strings with increasing size, via ctor( size, char )
        String sizedString( size,'x' );
        benchmark::DoNotOptimize( sizedString );
    }
}

BENCHMARK( BM_MatrixMulF )
    ->Range( 1<<4, 1<<12 );

BENCHMARK( BM_MatrixMulD )
    ->Range( 1<<4, 1<<12 );

BENCHMARK( BM_CreateString )
	->RangeMultiplier( 2 )
	->Range( 1<<0, 1<<13 );

BENCHMARK_MAIN();
