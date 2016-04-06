#pragma once

#include "ParticleBin.h"
#include "cinder/app/App.h"
#include "cinder/TriMesh.h"

#include "cinder/gl/Batch.h"


namespace bin{

	
	class ParticleSystem {
	protected:
		float timeStep;
		std::vector<Particle> particles;
		std::vector< std::vector<Particle*> > bins;
		int width, height, k, xBins, yBins, binSize;


	public:
		ParticleSystem();


		void setup(int width, int height, int k);
		void setTimeStep(float timeStep);

		void add(Particle particle);
		std::vector<Particle*> getNeighbors(Particle& particle, float radius);
		std::vector<Particle*> getNeighbors(float x, float y, float radius);
		std::vector<Particle*> getRegion(unsigned minX, unsigned minY, unsigned maxX, unsigned maxY);

		unsigned size() const;
		Particle& operator[](unsigned i);

		void setupForces();
		void addRepulsionForce(const Particle& particle, float radius, float scale);
		void addRepulsionForce(float x, float y, float radius, float scale);
		void addAttractionForce(const Particle& particle, float radius, float scale);
		void addAttractionForce(float x, float y, float radius, float scale);
		void addForce(const Particle& particle, float radius, float scale);
		void addForce(float x, float y, float radius, float scale);
		void update();

		void draw();
		void drawRain();

		ci::gl::BatchRef					mCircleBatch;
		
	};

	inline float InvSqrt(float x){
		float xhalf = 0.5f * x;
		int i = *(int*)&x; // store floating-point bits in integer
		i = 0x5f3759d5 - (i >> 1); // initial guess for Newton's method
		x = *(float*)&i; // convert new bits into float
		x = x*(1.5f - xhalf*x*x); // One round of Newton's method
		return x;
	}

}