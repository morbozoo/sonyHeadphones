#pragma once

#include <vector>
#include <cmath>
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

namespace bin{

	class Particle {
	public:
		float x, y;
		float xv, yv;
		float xf, yf;
		float xd, yd;
		float xp, yp;
		bool type;

		float gravity;

		Particle(float _x = 0, float _y = 0,
			float _xv = 0, float _yv = 0) :
			x(_x), y(_y),
			xv(_xv), yv(_yv) {

			xd = 0;
			yd = 0;
			xp = 0;
			yp = 0;

			float rand = ci::Rand::randFloat(0, 1);
			if (rand > 0.91)
				type = false;
			else
				type = true;

			gravity = ci::randFloat(0.005, 0.0099);
		}
		void updatePosition(float timeStep) {
			// f = ma, m = 1, f = a, v = int(a)
			xv += xf;
			yv += yf;
			x += xv * timeStep;
			y += yv * timeStep;

			xd = x - xp;
			yd = y - yp;

			xp = x;
			yp = y;
		}

		void resetForce() {
			xf = 0;
			yf = gravity;
		}
		void bounceOffWalls(float left, float top, float right, float bottom, float damping = .3) {
			bool collision = false;

			if (x > right){
				x = right;
				xv *= -1;
				collision = true;
			}
			else if (x < left){
				x = left;
				xv *= -1;
				collision = true;
			}

			if (y > bottom){
				y = bottom;
				yv *= -1;
				collision = true;
			}
			else if (y < top){
				y = top;
				yv *= -1;
				collision = true;
			}

			if (collision == true){
				xv *= damping;
				yv *= damping;
			}
		}

		void throughWallSBottom(float left, float top, float bottom, float damping = .3) {
			if (y > bottom){
				y = left - 50;
				x = ci::Rand::randFloat(0, top);
			}

			if (x < 0 || x > top){
				y = left - 50;
				x = ci::Rand::randFloat(0, top);
			}
		}

		void addDampingForce(float damping = .01) {
			xf = xf - xv * damping;
			yf = yf - yv * damping;
		}

		ci::vec2 getPos(){
			return ci::vec2(x, y);
		}

	};
}
