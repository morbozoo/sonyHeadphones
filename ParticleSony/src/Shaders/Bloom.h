//
//  Bloom.h
//
//  Created by tom on 5/20/14.
//
//
#pragma once

#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/app/App.h"


namespace shaders{

	class Bloom;
	typedef std::shared_ptr<Bloom> BloomRef;

	//BLOOM
	class Bloom{
	public:

		static BloomRef create(){
			return std::make_shared<Bloom>();
		}

		Bloom(){ attenuationBloom = 0.0; }

		void                setupBlur(ci::vec2 dims);
		void				compileShader();

		void                updateBlur();
		void                drawBlur();

		void                bindFboScene();
		void                unbindFboScene();

		void                setAttenuation(float v){ attenuationBloom = v; }

		ci::gl::FboRef      getFBOScene(){ return mFboScene; }

		ci::Area			getBounds(){ return bloomBounds; }

		ci::vec2			getSize(){ return bloomDims; }
	protected:
		float               attenuationBloom;

		//FBO
		ci::gl::FboRef         mFboScene;
		ci::gl::FboRef         mFboBlur1;
		ci::gl::FboRef         mFboBlur2;

		//GLSL
		ci::gl::GlslProgRef    mShaderBlur;

		ci::vec2               bloomDims;

		ci::Area	           bloomBounds;

	};
}