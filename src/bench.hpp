#pragma once

#include "global.hpp"

#include <benchmark/benchmark.h>

namespace JanSordid::ExampleDataModel
{
	using namespace JanSordid::Core;

	constexpr bool AllAlive      = false;
	constexpr bool SkipTargeting = false;
	constexpr bool DoEarlyExit   = false;
	constexpr f32  TargetRange   = 5;
	constexpr f32  TargetRangeSq = TargetRange * TargetRange;

	void RenderThing(f32 posX, f32 posY);

	template <typename NTT>
	void BM_SimulateOOP( benchmark::State & state );

	template <typename NTT>
	void BM_SimulateProc( benchmark::State & state );

	template <typename NTT>
	void BM_SimulateProcOpt( benchmark::State & state );

	void BM_SimulateDOD( benchmark::State & state );
}

namespace JanSordid::ExampleBenchmark
{
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

	void BM_Fibonacci( benchmark::State & state );
	void BM_MatrixMulF( benchmark::State & state );
	void BM_MatrixMulD( benchmark::State & state );
	void BM_CreateString( benchmark::State & state );
}