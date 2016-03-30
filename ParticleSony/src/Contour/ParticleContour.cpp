#include "cinder/Rand.h"

#include "ParticleContour.h"


namespace contour{
	using namespace ci;

	/////////////////////////////////////////////////////////////////////////////

	Particle::Particle(const vec2 &position, const ColorA &color)
	{
		mAcceleration = vec2(0);
		mInitColor = color;
		mVelColor = ci::ColorA(0, 0, 0, 0.5);
		mOrigin = position;
		mPosition = mOrigin;
		mVelocity = vec2(0);
		mType = 0;
		mActivated = true;
	}

	void Particle::addAcceleration(const vec2 &acceleration)
	{
		mAcceleration += acceleration;
	}

	const ColorA & Particle::getIntColor() const
	{
		return mInitColor;
	}

	const ColorA & Particle::getVelColor() const
	{
		return mVelColor;
	}

	const vec2 & Particle::getPosition() const
	{
		return mPosition;
	}

	const float & Particle::getVelocity() const
	{
		return mVel;
	}

	void Particle::update()
	{
		if (mActivated == true){
			mVelocity += mAcceleration;
			mPosition += mVelocity * vec2(0.333);
			mVelocity *= 0.97f;
			mAcceleration = vec2(0);

			mVel = abs(mVelocity.x + mVelocity.y) / 3.0;
			float distance = ci::distance(mOrigin, mPosition);
			if (distance < 0.2f) {
				mPosition = mOrigin;
				mVelocity = vec2(0);
				mVelColor = mInitColor;
				mActivated = false;
			}
			else {
				mPosition += (mOrigin - mPosition) * 0.1f;
				mVelColor.r = mVel;
				mVelColor.g = mVel;
				mVelColor.b = mVel + 0.3;
				mVelColor.a = 0.5 + mVel;
			}
		}
		//else{

		//}
	}
}