#include <global.h>

#include <benchmark/benchmark.h>

static void BM_CreateString( benchmark::State & state )
{
	const usize size = state.range( 0 );

	for( auto _ : state )
	{
		// Create strings with increasing size, via ctor( size, char )
		const String sizedString( size,'x' );
		benchmark::DoNotOptimize( sizedString );
	}
}

BENCHMARK( BM_CreateString )
	->RangeMultiplier( 2 )
	->Range( 1, 8<<10 );

BENCHMARK_MAIN();
