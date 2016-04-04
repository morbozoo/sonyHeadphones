//
//  Bloom.cpp
//
//  Created by tom on 5/20/14.
//
//

#include "Bloom.h"

namespace shaders{

	using namespace std;

	void Bloom::setupBlur(ci::vec2 dims)
	{
		
		//ci::gl::Texture2d::Format texFormat = ci::gl::Texture2d::Format();
		//texFormat.enableMipmapping(false);
		//texFormat.setWrap(GL_CLAMP_TO_BORDER_ARB, GL_CLAMP_TO_BORDER_ARB);

		//ci::gl::Fbo::Format format = ci::gl::Fbo::Format();
		//format.setColorTextureFormat(texFormat);

		ci::gl::Fbo::Format fmt;
		fmt.setSamples(8);
		fmt.setCoverageSamples(8);
		
		mFboBlur1 = ci::gl::Fbo::create(dims.x / 4.0f, dims.y / 4.0f, fmt);
		mFboBlur2 = ci::gl::Fbo::create(dims.x / 4.0f, dims.y / 4.0f, fmt);

		mFboScene = ci::gl::Fbo::create(dims.x / 1.0f, dims.y / 1.0f, fmt);

		this->bloomDims = dims;
		this->bloomBounds = ci::Area(0, 0, dims.x, dims.y);
		ci::app::console() << "bloom bounds " << bloomBounds << std::endl;

	}

	void Bloom::compileShader(){

		try {
			mShaderBlur = ci::gl::GlslProg::create(ci::app::loadAsset("blur_vert.glsl"), ci::app::loadAsset("blur_frag.glsl"));
		}
		catch (std::exception &exc) {
			ci::app::console()<<"exception caught what: " << exc.what()<<std::endl;
		}
	}

	void Bloom::bindFboScene()
	{
		mFboScene->bindFramebuffer();
		//ci::gl::viewport(ci::vec2(0), mFboScene->getSize());
		//ci::gl::setMatricesWindow(mFboScene->getSize());

	}

	void Bloom::unbindFboScene()
	{
		mFboScene->unbindFramebuffer();
	}

	void Bloom::updateBlur()
	{
		ci::gl::pushMatrices();

		ci::gl::ScopedGlslProg scpGlsl(mShaderBlur);
		mShaderBlur->uniform("tex0", 0);
		mShaderBlur->uniform("sample_offset", ci::vec2(1.0f / mFboBlur1->getWidth(), 0.0f));
		mShaderBlur->uniform("attenuation", attenuationBloom);

		{
			ci::gl::ScopedFramebuffer fbo(mFboBlur1);
			ci::gl::ScopedViewport(ci::vec2(0), mFboBlur1->getSize());
			ci::gl::ScopedTextureBind tex(mFboScene->getColorTexture(), 0);

			ci::gl::setMatricesWindow(mFboBlur1->getSize());
			ci::gl::clear(ci::Color::black());
			ci::gl::drawSolidRect(bloomBounds);
		}

		mShaderBlur->uniform("sample_offset", ci::vec2(0.0f, 1.0f / mFboBlur2->getHeight()));
		mShaderBlur->uniform("attenuation", attenuationBloom);

		{

			ci::gl::ScopedFramebuffer fbo(mFboBlur2);
			ci::gl::ScopedViewport viewport(ci::vec2(0), mFboBlur2->getSize());
			ci::gl::ScopedTextureBind tex(mFboBlur1->getColorTexture(), 0);

	
			ci::gl::setMatricesWindow(mFboBlur2->getSize());
			ci::gl::clear(ci::Color::black());

			ci::gl::drawSolidRect(bloomBounds);

		}

		ci::gl::popMatrices();
	}

	void Bloom::drawBlur()
	{

		ci::gl::pushModelView();
		ci::gl::color(ci::Color::white());
		ci::gl::draw(mFboScene->getColorTexture(), bloomBounds);

		/*
		ci::gl::enableAdditiveBlending();
		ci::gl::draw(mFboBlur2->getColorTexture(), bloomBounds);
		ci::gl::disableAlphaBlending();
		ci::gl::popModelView();
		*/
	}
}