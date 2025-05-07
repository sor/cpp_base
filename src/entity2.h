#pragma once

#include "global.hpp"

namespace JanSordid::ExampleDataModel
{

	struct Entity2 {
		using    Self = Entity2;

		String   name;

		Self *   target;

		f32      posX, posY;
		f32      velX, velY;

		f32      cd;
		f32      respawn;

		u16      health;
		u16      xp;

		char     _isAlive;
		char     _isVisible;
		char     _hasTarget;

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
	};
}
