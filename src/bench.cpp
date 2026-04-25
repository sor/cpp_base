#include "bench.hpp"

#include <Eigen/Core>

#include "entity0.hpp"
#include "entity1.hpp"
#include "entity2.hpp"
#include "entity3.hpp"
#include "entity4.hpp"

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

#pragma region SA NTT0-3
#if defined( WIN32 ) && defined( _DEBUG )
	static_assert( sizeof(String)         == 40 );
	static_assert( sizeof(DynArray<char>) == 32 );

	static_assert( sizeof(Entity0)      == 96 );
	static_assert( sizeof(Entity1)      == 96 );
//	static_assert( sizeof(Entity2)      == 80 );
//	static_assert( sizeof(Entity3)      == 72 );
#else
	static_assert( sizeof(String)         == 32 );
	static_assert( sizeof(DynArray<char>) == 24 );

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
#if defined( WIN32 ) && defined( _DEBUG )
	static_assert(  sizeof(Entity4Cold)      == 48 );
#else
	static_assert(  sizeof(Entity4Cold)      == 40 );   // 1.6 per Cacheline
#endif
	static_assert( alignof(Entity4Cold)      ==  8 );
#pragma endregion

	void RenderThing(f32 posX, f32 posY) {
		benchmark::DoNotOptimize(posX);
		benchmark::DoNotOptimize(posY);
	}

	extern template	void BM_SimulateOOP<Entity0>( benchmark::State & state );

	extern template	void BM_SimulateProc<Entity1>( benchmark::State & state );
	extern template	void BM_SimulateProc<Entity2>( benchmark::State & state );
	extern template	void BM_SimulateProc<Entity3>( benchmark::State & state );

	extern template	void BM_SimulateProcOpt<Entity1>( benchmark::State & state );
	extern template	void BM_SimulateProcOpt<Entity2>( benchmark::State & state );
	extern template	void BM_SimulateProcOpt<Entity3>( benchmark::State & state );

	constexpr i64
		BM_Start = 1<<4,
		BM_Stop  = 1<<15;

	// benchmark::ClobberMemory();
	//*
	BENCHMARK( BM_SimulateOOP<Entity0>     )->RangeMultiplier( 2 )->Range( BM_Start, BM_Stop );
	BENCHMARK( BM_SimulateProc<Entity1>    )->RangeMultiplier( 2 )->Range( BM_Start, BM_Stop );
	BENCHMARK( BM_SimulateProc<Entity2>    )->RangeMultiplier( 2 )->Range( BM_Start, BM_Stop );
	BENCHMARK( BM_SimulateProc<Entity3>    )->RangeMultiplier( 2 )->Range( BM_Start, BM_Stop );
	BENCHMARK( BM_SimulateProcOpt<Entity2> )->RangeMultiplier( 2 )->Range( BM_Start, BM_Stop );
	BENCHMARK( BM_SimulateProcOpt<Entity3> )->RangeMultiplier( 2 )->Range( BM_Start, BM_Stop );
	BENCHMARK( BM_SimulateDOD              )->RangeMultiplier( 2 )->Range( BM_Start, BM_Stop );
	//*/
}

// Benchmarks below have rather long compilation time, if not needed comment the block out
//*
namespace JanSordid::ExampleBenchmark {

	using namespace JanSordid::Core;

	void BM_Fibonacci( benchmark::State & state )
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

	void BM_MatrixMulF( benchmark::State & state )
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

	void BM_MatrixMulD( benchmark::State & state )
	{
		const i64 size = state.range( 0 ); // Not used yet
		Eigen::MatrixXd a(size, size ), b( size, size );
		benchmark::DoNotOptimize( a );

		for( auto _ : state )
		{
			a *= b;
		}
	}

	void BM_CreateString( benchmark::State & state )
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

	//*
	BENCHMARK( BM_CreateString )
		->RangeMultiplier( 2 )
		->Range( 0, 1<<13 );
	//*/

	BENCHMARK( BM_Fibonacci )
		->DenseRange( 0, 47, 1 );
}
//*/

BENCHMARK_MAIN();

// TODO: read up on doctest
