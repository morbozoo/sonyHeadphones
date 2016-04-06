#include "ParticleSystemBin.h"

namespace bin{

	using namespace ci;

	ParticleSystem::ParticleSystem() :
		timeStep(1) {


		gl::GlslProgRef solidShader = gl::getStockShader(gl::ShaderDef().color());
		mCircleBatch = ci::gl::Batch::create(geom::Circle().radius(1).subdivisions(32), solidShader);
	}

	void ParticleSystem::setup(int width, int height, int k) {
		this->width = width;
		this->height = height;
		this->k = k;
		binSize = 1 << k;
		xBins = (int)ceilf((float)width / (float)binSize);
		yBins = (int)ceilf((float)height / (float)binSize);
		bins.resize(xBins * yBins);
	}

	void ParticleSystem::setTimeStep(float timeStep) {
		this->timeStep = timeStep;
	}

	void ParticleSystem::add(Particle particle) {
		particles.push_back(particle);
	}

	unsigned ParticleSystem::size() const {
		return particles.size();
	}

	Particle& ParticleSystem::operator[](unsigned i) {
		return particles[i];
	}

	std::vector<Particle*> ParticleSystem::getNeighbors(Particle& particle, float radius) {
		return getNeighbors(particle.x, particle.y, radius);
	}

	std::vector<Particle*> ParticleSystem::getNeighbors(float x, float y, float radius) {
		std::vector<Particle*> region = getRegion(
			(int)(x - radius),
			(int)(y - radius),
			(int)(x + radius),
			(int)(y + radius));
		std::vector<Particle*> neighbors;
		int n = region.size();
		float xd, yd, rsq, maxrsq;
		maxrsq = radius * radius;
		for (int i = 0; i < n; i++) {
			Particle& cur = *region[i];
			xd = cur.x - x;
			yd = cur.y - y;
			rsq = xd * xd + yd * yd;
			if (rsq < maxrsq)
				neighbors.push_back(region[i]);
		}
		return neighbors;
	}

	std::vector<Particle*> ParticleSystem::getRegion(unsigned minX, unsigned minY, unsigned maxX, unsigned maxY) {
		std::vector<Particle*> region;
		std::back_insert_iterator< std::vector<Particle*> > back = back_inserter(region);
		unsigned minXBin = minX >> k;
		unsigned maxXBin = maxX >> k;
		unsigned minYBin = minY >> k;
		unsigned maxYBin = maxY >> k;
		maxXBin++;
		maxYBin++;
		if (maxXBin > xBins)
			maxXBin = xBins;
		if (maxYBin > yBins)
			maxYBin = yBins;
		for (int y = minYBin; y < maxYBin; y++) {
			for (int x = minXBin; x < maxXBin; x++) {
				std::vector<Particle*>& cur = bins[y * xBins + x];
				copy(cur.begin(), cur.end(), back);
			}
		}
		return region;
	}

	void ParticleSystem::setupForces() {
		int n = bins.size();
		for (int i = 0; i < n; i++) {
			bins[i].clear();
		}
		n = particles.size();
		unsigned xBin, yBin, bin;
		for (int i = 0; i < n; i++) {
			Particle& cur = particles[i];
			cur.resetForce();
			xBin = ((unsigned)cur.x) >> k;
			yBin = ((unsigned)cur.y) >> k;
			bin = yBin * xBins + xBin;
			if (xBin < xBins && yBin < yBins)
				bins[bin].push_back(&cur);
		}
	}

	void ParticleSystem::addRepulsionForce(const Particle& particle, float radius, float scale) {
		addRepulsionForce(particle.x, particle.y, radius, scale);
	}

	void ParticleSystem::addRepulsionForce(float x, float y, float radius, float scale) {
		addForce(x, y, radius, scale);
	}

	void ParticleSystem::addAttractionForce(const Particle& particle, float radius, float scale) {
		addAttractionForce(particle.x, particle.y, radius, scale);
	}

	void ParticleSystem::addAttractionForce(float x, float y, float radius, float scale) {
		addForce(x, y, radius, -scale);
	}

	void ParticleSystem::addForce(const Particle& particle, float radius, float scale) {
		addForce(particle.x, particle.y, radius, -scale);
	}

	void ParticleSystem::addForce(float targetX, float targetY, float radius, float scale) {
		float minX = targetX - radius;
		float minY = targetY - radius;
		float maxX = targetX + radius;
		float maxY = targetY + radius;
		if (minX < 0)
			minX = 0;
		if (minY < 0)
			minY = 0;
		unsigned minXBin = ((unsigned)minX) >> k;
		unsigned minYBin = ((unsigned)minY) >> k;
		unsigned maxXBin = ((unsigned)maxX) >> k;
		unsigned maxYBin = ((unsigned)maxY) >> k;
		maxXBin++;
		maxYBin++;
		if (maxXBin > xBins)
			maxXBin = xBins;
		if (maxYBin > yBins)
			maxYBin = yBins;
		float xd, yd, length, maxrsq;

		float xhalf;
		int lengthi;

		maxrsq = radius * radius;
		int x, y;
		for (y = minYBin; y < maxYBin; y++) {
			for (x = minXBin; x < maxXBin; x++) {
				std::vector<Particle*>& curBin = bins[y * xBins + x];
				int n = curBin.size();
				for (int i = 0; i < n; i++) {
					Particle& curParticle = *(curBin[i]);
					//				As suggested by Andrew Bell
					//				if(curParticle.x > minX && curParticle.x < maxX &&
					//					curParticle.y > minY && curParticle.y < maxY) {
					xd = curParticle.x - targetX;
					yd = curParticle.y - targetY;
					length = (xd * xd + yd * yd);

					if (length > 0 && length < maxrsq) {

						xhalf = 0.5f * length;
						lengthi = *(int*)&length;
						lengthi = 0x5f3759df - (lengthi >> 1);
						length = *(float*)&lengthi;
						length *= 1.5f - xhalf * length * length;
						xd *= length;
						yd *= length;
						length *= radius;
						length = 1 / length;
						length = (1 - length);
						length *= scale;
						xd *= length;
						yd *= length;
						curParticle.xf += xd;
						curParticle.yf += yd;

					}
					//				}
				}
			}
		}
	}


	void ParticleSystem::update() {
		int n = particles.size();
		for (int i = 0; i < n; i++)
			particles[i].updatePosition(timeStep);
	}

	void ParticleSystem::drawRain() {
		int n = particles.size();

		for (int i = 0; i < n; i++){

			
			float height = (ci::app::getWindowHeight() - particles[i].y) / ci::app::getWindowHeight();
			if (height > 0.02 && height < 0.98){

				float vel = 2.1*(particles[i].xd + particles[i].yd);
	
				if (particles[i].type){

					ci::gl::color(ci::ColorA(0.7, 0.7, 0.7, 0.4));

					gl::drawLine(ci::vec2(particles[i].x, particles[i].y),
						ci::vec2(particles[i].x - vel*0.4, particles[i].y - 12 - 1.8 * vel - height * 35));


					gl::pushMatrices();
					float rad = height * 17 + vel;
					ci::gl::color(ci::ColorA(0.8, 0.8, 0.8, 0.7));
					gl::translate(particles[i].getPos());
					gl::scale(ci::vec2(rad));
					mCircleBatch->draw();
					gl::popMatrices();

				}
				else{

					gl::pushMatrices();
					float rad = height * 12 + vel*0.8 + 1.0;
					ci::gl::color(ci::ColorA(0.0, 0.7, 0.8, 0.7));
					gl::translate(particles[i].getPos());
					gl::scale(ci::vec2(rad));
					mCircleBatch->draw();
					gl::popMatrices();
				}
			}
		}
	}

	void ParticleSystem::draw() {
		int n = particles.size();

		for (int i = 0; i < n; i++){
			float vel = 3 * (particles[i].xd + particles[i].yd);
			float height = (ci::app::getWindowHeight() - particles[i].y) / ci::app::getWindowHeight();

			ci::gl::color(ci::ColorA(0.8, 0.8, 0.8, 1.0));
			gl::drawSolidCircle(particles[i].getPos(), 2);
		}
	}
}