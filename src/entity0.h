#pragma once

#include "global.hpp"

namespace JanSordid::ExampleDataModel
{
	//#pragma pack(push, 1)
	struct Entity0 {
		using    Self = Entity0;

		bool     _isAlive;
		u16      health;

		bool     _isVisible;
		f32      posX, posY;
		f32      velX, velY;

		bool     _hasTarget;
		Self *   target;

		u16      xp;

		f32      cd;
		f32      respawn;

		String   name;

		bool     isAlive()   const { return _isAlive; }
		bool     isVisible() const { return _isVisible; }
		bool     hasTarget() const { return _hasTarget; }

		void     setAlive()                   { _isAlive   = true;  health = 10; }
		void     setDead()                    { _isAlive   = false; health =  0; }
		void     doDamage(int value)          { health -= value; }
		void     setVisible(const bool value) { _isVisible = value; }
		void     setTarget(Self * value)      { _hasTarget = true;  target = value; }
		void     unsetTarget()                { _hasTarget = false; }

		bool     isCooldownDone() const { Assert( isAlive()); return cd      <= 0; }
		bool     canRespawn()     const { Assert(!isAlive()); return respawn <= 0; }

		void     progressCooldown(const f32 dt)    { Assert( isAlive()); cd      -= dt; }
		void     progressRespawn (const f32 dt)    { Assert(!isAlive()); respawn -= dt; }
		void     setCooldown     (const f32 value) { Assert( isAlive()); cd       = value; }
		void     setRespawn      (const f32 value) { Assert(!isAlive()); respawn  = value; }

		f32      calcDistTo(const Self & other) const;
		f32      calcSqDistTo(const Self & other) const;

		void     Init(const bool AllAlive, const int ii);
		void     Move(const f32 dt);
		template <bool DoEarlyExit>
		void     Targeting(const f32 TargetRange, Vector<Self> & es );
		void     Damaging(const f32 dt);
		void     Respawn(const f32 dt);
		void     Render();

		//virtual ~Entity0() = default;
	};
	//#pragma pack(pop)

	extern template void Entity0::Targeting<false>(const f32 TargetRange, Vector<Self> & es );
	extern template void Entity0::Targeting<true> (const f32 TargetRange, Vector<Self> & es );
}
