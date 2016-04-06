/*
*
* Copyright (c) 2013, Thomas Sanchez Lengeling
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or
* without modification, are permitted provided that the following
* conditions are met:
*
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
*
* Neither the name of the Ban the Rewind nor the names of its
* contributors may be used to endorse or promote products
* derived from this software without specific prior written
* permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Triangulate.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Perlin.h"



#include "Kinectv1.h"

#include "../Contour/ParticleContour.h"
#include "../Contour/ParticlesBox2D.h"
#include "../Contour/ContourFinder.h"
#include "../Contour/ParticleSystemBin.h"

namespace kinect {

	//namespaces
	using namespace physics;
	using namespace contour;


	const int CIRCLE_RAD = 3;


	//particle grid
	const float kPointSpacing = 15.0f;
	const float kInteractiveForce = 0.9f;
	const float kInteractiveRadius = 70.0f;
	const float kFloatMax = std::numeric_limits<float>::max();

	//smart pointer 
	class KinectManager;
	typedef std::shared_ptr<KinectManager> KinectManagerRef;

	class KinectManager{
	public:

		static KinectManagerRef create()
		{
			return std::make_shared<KinectManager>();
		}

		void cleanUp();

		void setupKinect();
		void setupPhysics(ci::vec2 gravity);
		void setupParticleGrid();

		//Kinect
		void updateKinect(float scaleX1, float scaleY1, int transX1, int transY1, float scaleX2, float scaleY2, int transX2, int transY2, std::vector<bool> usersK1, std::vector<bool> usersK2);

		void drawSkeleton();

		//contour
		void updateParticlesBox2d();
		void drawParticlesBox2d(ci::ColorA col);
		void drawContours();
		void drawContoursAlter(float colorR, float colorG, float colorB);
		void drawUpdateTriangulated(float colR, float colG, float colB);

		void setBox2dDrawMode(int value){ mBox2dManager->setDrawMode(value); }


		//particles grid
		void drawParticlePointGrid(float colorR, float colorG, float colorB);
		void drawParticleSquareGrid(float colorR, float colorG, float colorB);
		void drawParticleGrid(float colorR, float colorG, float colorB);
		void drawParticlesLineH(float colorR, float colorG, float colorB);
		void drawParticlesLineV(float colorR, float colorG, float colorB);

		//Rain
		void  setupRainParticles();
		void  updateRainParticles();
		void  updateContourBinParticles();
		void drawRain(float colorR, float colorG, float colorB);

		void updateParticleGrid();


		void addParticle(ci::vec2 & pos);
		void deleteParticle();
		int getParticleSize();

		int getNumKinects(){ return mDevices.size(); }

		void setTimePerlin(float value){ mTimeSpeed = value; }


		bool									mDrawTriangulatedContour;

	private:
		ParticleManagerRef					    mBox2dManager;


		// Device info
		struct Device
		{
			int32_t								mCallbackId;
			int32_t								mSkeletonCallbackId;

			KinectSdk::KinectRef				mKinect;
			ci::gl::TextureRef					mTexture;
			ci::Channel16uRef					mChannel;

			std::vector<KinectSdk::Skeleton>	mSkeletons;

			std::vector<contour::Contour>		mContours;
			std::vector<float>					mCenters;
		};

		// Callback
		void									onDepthData(ci::Surface16u surface, const KinectSdk::DeviceOptions &deviceOptions);
		void									onSkeletonData(std::vector<KinectSdk::Skeleton> skeletons, const KinectSdk::DeviceOptions &deviceOptions);

		ci::vec2								mKinectDims;
		ci::vec2								mScaleContour;
		float									mKinectTranslateY;

		std::vector<Device>						mDevices;
		int										mNumOfUsers;

		//CONTOUR
		contour::ContourFinderRef				mContourFinder;


		// PARTICLE GRID
		std::vector<contour::Particle>			mParticles;
		ci::ivec2								mGridDims;

		//perlin
		ci::Perlin								mPerlin;
		int										mSeed;
		int										mOctaves;
		float									mTime;
		float									mTimeSpeed;
		float									mFrequency;


		//BIN PARTICLES
		ci::ivec2						 mouse;

		float							 timeStep;
		float							 lineOpacity, pointOpacity;
		float							 particleNeighborhood, particleRepulsion;
		float							 centerAttraction;

		int								 kParticles;
		bin::ParticleSystem				 particleSystemBin;
		bool							 isMousePressed;
		bool							 slowMotion;

		//Circle batch
		ci::gl::BatchRef						mCircleBatch;

		ci::gl::BatchRef						mCircleContourBatch;

		ci::gl::BatchRef						mSquareContourBatch;

		ci::gl::BatchRef						mSquareBatch;
		
	};
}