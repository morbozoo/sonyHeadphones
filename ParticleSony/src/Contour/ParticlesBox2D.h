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
#include "cinder/TriMesh.h"
#include "cinder/Rand.h"
#include "cinder/Log.h"
#include <exception>

#include <Box2D/Box2D.h>


namespace physics{

	const float			BOX_SIZE = 10;

	///-- Class particle Box, includes physics of the box2d with extra properties
	//    Such as texture, color, alive
	/////////////////////////////////////////////////////////////////////////////////

	//smart pointer 
	class ParticleBox;
	typedef std::shared_ptr<ParticleBox> ParticleBoxRef;

	class ParticleBox{

	public:
		ParticleBox(const b2Body * b);

		//create the instance of the particle
		static ParticleBoxRef create(const b2Body* b){
			return std::make_shared<ParticleBox>(b);
		}

		void draw();
		void update();

		const b2Body * getB2Body(){ return b2b; }



		void setInitPos(ci::vec2 & pos){ mInitPos = pos; }
		ci::vec2 getInitPos(){ return mInitPos; }

		void setSize(float & size){ mSize = size; }
		float getSize(){ return mSize; }

		const b2Body	* b2b;

		bool			isDead(){ return mDead; }

		void			setId(int id){ mId = id; }
		int				getId(){ return mId; }

		void            setType(int val){ mType = val; }
		int				getType(){ return mType; }

	private:

		//body particle, manages all the physics, collisions


		ci::vec2					mInitPos;
		float						mSize;
		float						life;

		bool						mDead;
		int							mId;
		int							mType;

	};


	//--------------  Physics Manager -----------------------------------------------
	/////////////////////////////////////////////////////////////////////////////////

	//smart pointer
	class ParticleManager;
	typedef std::shared_ptr<ParticleManager> ParticleManagerRef;

	class ParticleManager{
	public:

		//create the instance of the physics manager
		static ParticleManagerRef create(){
			return std::make_shared< ParticleManager >();
		}

		void setup(ci::vec2 gravityWind);

		void draw(ci::ColorA col);

		void update();

		void clean();
		void addContourTriangulation(ci::TriMesh & mesh);

		//add a particle to te physics
		void addParticle(ci::vec2 &  pos);
		void deleteParticle();

		int getNumParticles(){ return mBoxes.size(); }

		void setDrawMode(int value){ mDrawMode = value; }

	private:
		//Physics
		b2World								*mWorld;

		std::vector<ParticleBoxRef >		mBoxes;
		std::vector<b2Body *>				mPolygons;
		std::map<int, int>					mDeleteIndex;

		ci::gl::Texture2dRef				mParticleTexture;
		int									mParticleCounter;


		int									mDrawMode;

		//Circle batch
		ci::gl::BatchRef					mCircleBatch;

		ci::gl::BatchRef					mCiruferenceBatch;

		ci::gl::BatchRef					mSquareBatch;

		ci::gl::BatchRef					mTriangleBatch;
	};
}