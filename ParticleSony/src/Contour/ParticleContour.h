#pragma once

#include "cinder/app/App.h"
#include "cinder/Color.h"
#include "cinder/Vector.h"


namespace contour{

	class Particle
	{
	public:
		Particle(const ci::vec2 & position = ci::vec2(0), const ci::ColorA &color = ci::ColorAf::white());

		void				addAcceleration(const ci::vec2 &acceleration);

		const ci::ColorA &	getIntColor() const;
		const ci::ColorA &  getVelColor() const;
		const ci::vec2   &	getPosition() const;
		const float		 &  getVelocity() const;

		void				update();

		void				activateParticles(){
			mActivated = true;
		}

		void addIncrement(float & val){ mCosInc = val; }

		void setVelColor(ci::ColorA col) { mVelColor = col; }

		bool isActivated(){ return mActivated; }

		int getType(){ return mType; }
		void setType(int t){ mType = t; }
	private:
		ci::vec2			mAcceleration;
		ci::ColorA			mInitColor;
		ci::ColorA			mVelColor;
		ci::vec2			mOrigin;
		ci::vec2			mPosition;
		ci::vec2			mVelocity;

		float				mVel;

		bool				mActivated;

		float				mCosInc;

		int					mType;
	};
}
