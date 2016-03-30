//
//  Bloom.cpp
//
//  Created by tom on 5/20/14.
//
//

#include "Bloom.h"

namespace shaders{
	void Bloom::setupBlur(ci::vec2 dims)
	{
		
		ci::gl::Texture2d::Format texFormat = ci::gl::Texture2d::Format();
		texFormat.enableMipmapping(false);
		texFormat.setWrap(GL_CLAMP_TO_BORDER_ARB, GL_CLAMP_TO_BORDER_ARB);

		ci::gl::Fbo::Format format = ci::gl::Fbo::Format();
		format.setColorTextureFormat(texFormat);
		
		mFboBlur1 = ci::gl::Fbo::create(dims.x / 4.0f, dims.y / 4.0f, format);
		mFboBlur2 = ci::gl::Fbo::create(dims.x / 4.0f, dims.y / 4.0f, format);

		format.setCoverageSamples(16);
		format.setSamples(4);

		mFboScene = ci::gl::Fbo::create(dims.x / 1.0f, dims.y / 1.0f, format);

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
		ci::gl::viewport(ci::vec2(0), mFboScene->getSize());
	}

	void Bloom::unbindFboScene()
	{
		mFboScene->unbindFramebuffer();
	}

	void Bloom::updateBlur()
	{
		
		ci::gl::ScopedGlslProg scpGlsl(mShaderBlur);
		mShaderBlur->uniform("tex0", 0);
		mShaderBlur->uniform("sample_offset", ci::vec2(1.0f / mFboBlur1->getWidth(), 0.0f));
		mShaderBlur->uniform("attenuation", attenuationBloom);

		{
			mFboBlur1->bindFramebuffer();
			mFboScene->bindTexture(0);
			ci::gl::ScopedViewport(ci::vec2(0), mFboBlur1->getSize());
			ci::gl::clear(ci::Color::black());
			ci::gl::drawSolidRect(bloomBounds);
			mFboScene->unbindTexture();
			mFboBlur1->unbindFramebuffer();
		}

		mShaderBlur->uniform("sample_offset", ci::vec2(0.0f, 1.0f / mFboBlur2->getHeight()));
		mShaderBlur->uniform("attenuation", attenuationBloom);

		{
			mFboBlur2->bindFramebuffer();
			mFboBlur1->bindTexture(0);
			ci::gl::ScopedViewport(ci::vec2(0), mFboBlur2->getSize());
	
			ci::gl::clear(ci::Color::black());
			ci::gl::drawSolidRect(bloomBounds);
			mFboBlur1->unbindTexture();
			mFboBlur2->unbindFramebuffer();
		}

		ci::gl::viewport(ci::vec2(0), bloomBounds.getSize());
	}

	void Bloom::drawBlur()
	{
		ci::gl::pushModelView();
		ci::gl::translate(ci::vec2(0, bloomDims.y));
		ci::gl::scale(ci::vec3(1, -1, 1));
		ci::gl::color(ci::Color::white());
		ci::gl::draw(mFboScene->getColorTexture(), bloomBounds);

		ci::gl::enableAdditiveBlending();
		ci::gl::draw(mFboBlur2->getColorTexture(), bloomBounds);
		ci::gl::disableAlphaBlending();
		ci::gl::popModelView();
	}
}