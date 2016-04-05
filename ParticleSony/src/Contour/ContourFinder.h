#pragma once

//#include "CinderOpenCV.h"
#include "CinderOpenCV.h"
#include "cinder/Triangulate.h"


///////////////////////////////////////////////////////////////////////////////////////////////
namespace contour{

	class ContourFinder;

	class Contour
	{
	public:
		Contour();
		~Contour();

		ci::vec2  &						getCentroid();
		const ci::vec2 &				getCentroid() const;
		std::vector<ci::vec2> &			getPoints();
		const std::vector<ci::vec2> &	getPoints() const;

		void							calcCentroid();

		ci::vec2						getPoint(int index){ return mPoints.at(index); }
		float							getDepth(int index){ return mDepth.at(index); }

		ci::Shape2d						getShape(){ return mShape; }

		int								getShapeSize(){ return mNumPointsShape; }

	private:
		void							addPoint(const ci::vec2 &position);
		void							addDepth(float depth);

		ci::vec2						mCentroid;
		std::vector<ci::vec2>			mPoints;
		std::vector<float>				mDepth;

		ci::Shape2d						mShape;

		int								mNumPointsShape;
		bool							mActive;

		friend class					ContourFinder;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////

	typedef std::shared_ptr<ContourFinder> ContourFinderRef;

	class ContourFinder
	{
	public:
		static ContourFinderRef	create();
		~ContourFinder();

		std::vector<Contour>	findContours(const ci::Channel8u &channel, int32_t smoothing = 0);

		std::vector<Contour>	findContours(const ci::Channel8u &channel, const ci::vec2 & scale, const ci::vec2 & trans, std::vector<bool> users, int32_t smoothing = 0);
	private:
		ContourFinder();

		ci::Channel8u			mChannelBackground;
		cv::Mat					mCvMatBackground;
		cv::Mat					mCvMatDiff;
		cv::Mat					mCvMatFrame;
		cv::Mat					mCvMatTemp;
	};
}