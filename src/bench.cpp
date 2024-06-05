#include "global.h"

#include <benchmark/benchmark.h>
#include <Eigen/Core>

using namespace JanSordid::Core;

//consteval
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
}

BENCHMARK( BM_Fibonacci )
->DenseRange( 0, 47, 1 );

/*
BENCHMARK( BM_MatrixMulF )
    ->Range( 1<<4, 1<<12 );

BENCHMARK( BM_MatrixMulD )
    ->Range( 1<<4, 1<<12 );
//*/

BENCHMARK( BM_CreateString )
	->RangeMultiplier( 2 )
	->Range( 1<<0, 1<<13 );

BENCHMARK_MAIN();

// TODO: read up on doctest
