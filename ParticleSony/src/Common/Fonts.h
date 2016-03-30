pragma once

#include "cinder/gl/gl.h"
#include "cinder/app/App.h"

namespace font{
	//-----------------Fonts class manager-----------------
	const std::string	COURIER_PATH = "fonts/Courier New.ttf";
	const std::string	COURIER_FONT = "Courier New";


	//References shared pointer
	class Fonts;
	typedef std::shared_ptr<Fonts> FontsRef;

	enum class FontType{
		FONT_SIZE_TINY = 13,
		FONT_SIZE_SMALL = 18,
		FONT_SIZE_NORMAL = 23,
		FONT_SIZE_BIG = 28,
		FONT_SIZE_LARGE = 33
	};

	class Fonts{
	public:

		Fonts(){}

		static FontsRef create(){
			return std::make_shared<Fonts>();
		}

		//FONTS
		void Fonts::createFonts()
		{
			mFont_tiny = ci::Font(COURIER_FONT, static_cast<std::underlying_type<FontType>::type>(FontType::FONT_SIZE_TINY));
			mFont_small = ci::Font(COURIER_FONT, static_cast<std::underlying_type<FontType>::type>(FontType::FONT_SIZE_SMALL));
			mFont_normal = ci::Font(COURIER_FONT, static_cast<std::underlying_type<FontType>::type>(FontType::FONT_SIZE_NORMAL));
			mFont_big = ci::Font(COURIER_FONT, static_cast<std::underlying_type<FontType>::type>(FontType::FONT_SIZE_BIG));
			mFont_large = ci::Font(COURIER_FONT, static_cast<std::underlying_type<FontType>::type>(FontType::FONT_SIZE_LARGE));


		}

		//get fonts
		ci::Font & Fonts::getFont(FontType fontType)
		{
			if (fontType == FontType::FONT_SIZE_TINY){
				return mFont_tiny;
			}
			else if (fontType == FontType::FONT_SIZE_SMALL){
				return mFont_small;
			}
			else if (fontType == FontType::FONT_SIZE_NORMAL){
				return mFont_normal;
			}
			else if (fontType == FontType::FONT_SIZE_BIG){
				return mFont_big;
			}
			else if (fontType == FontType::FONT_SIZE_LARGE){
				return mFont_large;
			}

			return mFont_normal;
		}

	private:
		//Display text
		ci::Font			mFont_tiny;
		ci::Font			mFont_small;
		ci::Font			mFont_normal;
		ci::Font			mFont_big;
		ci::Font			mFont_large;
	};
}