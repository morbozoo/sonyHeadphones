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
#include "cinder/app/RendererGl.h"

#include "cinder/gl/gl.h"
#include "cinder/app/Renderer.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder\params\Params.h"
#include "cinder\Rand.h"

#include "cinder/Filesystem.h"
#include "cinder/app/msw/AppImplMswBasic.h"


#include "Kinect\KinectManager.h"

#include "Common/Config.h"
#include "Common/SystemVars.h"
#include "Common/Utf8.h"

#include "Shaders\Bloom.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include "../blocks/Spout/include/Spout.h"
//spout
//#include "Spout.h




class ParticleSonyApp : public ci::app::App
{

public:

	static void							prepareSettings(ci::app::App::Settings *settings);

	// Cinder callbacks
	void								draw();
	void								keyDown(ci::app::KeyEvent event);

	void								shutdown();
	void								setup();
	void								update();

	void								updateMode();
	void								drawMode();

	static ci::fs::path findPath(const ci::fs::path & folder);

private:

	//PARAMS
	ci::params::InterfaceGlRef			mParams;
	float								mAvgFps;
	bool								mDrawGUI;
	bool								mDrawContours;
	int									mDrawMode;
	int									mNumKinects;
	

	//Kinect Manager
	kinect::KinectManagerRef			mKinectManagerRef;
	int									numParticles;
	int									numParticlesBox2d;
	int									rateParticles;
	int									maxNumParticles;



	shaders::BloomRef					mBloom;
	float								mBloomFactor;

	float								mTimeStep;


	void setupSpout();
	void sendSpout();
	SpoutSender spoutsender;					// Create a Spout sender object
	bool bInitialized;							// true if a sender initializes OK
	bool bMemoryMode;							// tells us if texture share compatible
	unsigned int g_Width, g_Height;				// size of the texture being sent out
	char SenderName[256];
	ci::gl::Texture2dRef spoutTexture;				// Local Cinder texture used for sharing
	// ----------------------------
};