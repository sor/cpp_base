#include "bench.hpp"

#include "entity0.hpp"

namespace JanSordid::ExampleDataModel
{
	using namespace JanSordid::Core;

	template <typename NTT>
	void BM_SimulateOOP( benchmark::State & state )
	{
		const i64 size = state.range( 0 );

		srand( 7940 );

		//using NTT = Entity0;
		DynArray<NTT> es(size);

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

			if( !SkipTargeting ) {
				for( NTT &e: es ) {
					e.TargetingNeedsReset( TargetRange, es );
					e.template Targeting<DoEarlyExit>( TargetRange, es );
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

	template void BM_SimulateOOP<Entity0>( benchmark::State & state );
}
