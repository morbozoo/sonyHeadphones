#include "ContourFinder.h"

namespace contour{
	using namespace ci;
	using namespace std;

	///////////////////////////////////////////////////////////////////////////////////////////////

	Contour::Contour()
	{
		mCentroid = vec2(0);
	}

	Contour::~Contour()
	{
		mPoints.clear();
	}

	void Contour::addPoint(const vec2 &position)
	{
		mPoints.push_back(position);
	}

	void Contour::addDepth(float depth)
	{
		mDepth.push_back(depth);
	}

	void Contour::calcCentroid()
	{
		mCentroid = vec2(0);
		if (!mPoints.empty()) {
			for (vector<vec2>::const_iterator pointIt = mPoints.cbegin(); pointIt != mPoints.cend(); ++pointIt) {
				mCentroid += *pointIt;
			}
		}
		mCentroid /= (float)mPoints.size();
	}

	vec2 & Contour::getCentroid()
	{
		return mCentroid;
	}

	const vec2 & Contour::getCentroid() const
	{
		return mCentroid;
	}

	vector<vec2>& Contour::getPoints()
	{
		return mPoints;
	}

	const vector<vec2>& Contour::getPoints() const
	{
		return mPoints;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////

	ContourFinderRef ContourFinder::create()
	{
		return ContourFinderRef(new ContourFinder());
	}

	ContourFinder::ContourFinder()
	{
		mChannelBackground = Channel8u(320, 240);
		Channel8u::Iter iter = mChannelBackground.getIter();
		while (iter.line()) {
			while (iter.pixel()) {
				mChannelBackground.setValue(iter.getPos(), 255);
			}
		}
		mCvMatBackground = toOcv(mChannelBackground);
	}

	ContourFinder::~ContourFinder()
	{
		mCvMatBackground.release();
		mCvMatDiff.release();
		mCvMatFrame.release();
		mCvMatTemp.release();
	}

	vector<Contour> ContourFinder::findContours(const Channel8u &channel, int32_t smoothing)
	{
		vector<Contour> contours;
		vector<vector<cv::Point> > cvContours;

		// Convert image to OpenCV
		mCvMatFrame = toOcv(channel);

		cv::Mat  copyFrame = mCvMatFrame;

		// Threshold the image
		cv::threshold(mCvMatFrame, mCvMatFrame, 0.5, 255.0, CV_THRESH_BINARY);

		// Find difference between input channel and solid background
		cv::absdiff(mCvMatBackground, mCvMatFrame, mCvMatDiff);

		// Do not evaluate a pure-zero image or an exception will occur
		if (cv::countNonZero(mCvMatDiff) > 0) {

			// Find contours
			cv::findContours(mCvMatDiff, cvContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
			for (vector<vector<cv::Point> >::const_iterator contourIt = cvContours.cbegin(); contourIt != cvContours.cend(); ++contourIt) {
				Contour contour;

				// Iterate through points in CV contour. Contour smoothing skips over 
				// points to simplify the outline for faster tracking.
				int32_t i = 0;

				contour.mShape.clear();
				contour.mNumPointsShape = 0;
				for (vector<cv::Point>::const_iterator pointIt = contourIt->cbegin(); pointIt != contourIt->cend(); ++pointIt, i++) {
					
					if (smoothing == 0 || i % smoothing == 0) {
						vec2 point((float)pointIt->x, (float)pointIt->y);
						contour.addPoint(point);
						contour.addDepth(channel.getValue(point));

						//fill shape strucutre
						if (smoothing == 0 || i % 5 == 0) {
							if (i == 0){
								contour.mShape.moveTo(point);
								contour.mNumPointsShape++;
							}
							else{
								contour.mShape.lineTo(point);
								contour.mNumPointsShape++;
							}
						}
						
					}
				}

				// Add contour
				contours.push_back(contour);

			}

		}

		return contours;
	}

	//contour scale and translate
	std::vector<Contour>	ContourFinder::findContours(const ci::Channel8u &channel, const ci::vec2 & scale, const ci::vec2 & trans, std::vector<bool> users, int32_t smoothing)
	{
		vector<Contour> contours;
		vector<vector<cv::Point> > cvContours;

		// Convert image to OpenCV
		mCvMatFrame = toOcv(channel);

		cv::Mat  copyFrame = mCvMatFrame;

		// Threshold the image
		cv::threshold(mCvMatFrame, mCvMatFrame, 0.5, 255.0, CV_THRESH_BINARY);

		// Find difference between input channel and solid background
		cv::absdiff(mCvMatBackground, mCvMatFrame, mCvMatDiff);

		// Do not evaluate a pure-zero image or an exception will occur
		if (cv::countNonZero(mCvMatDiff) > 0) {
			int cont = 0;
			// Find contours
			cv::findContours(mCvMatDiff, cvContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
			for (vector<vector<cv::Point> >::const_iterator contourIt = cvContours.cbegin(); contourIt != cvContours.cend(); ++contourIt) {
				Contour contour;

				// Iterate through points in CV contour. Contour smoothing skips over 
				// points to simplify the outline for faster tracking.
				int32_t i = 0;

				contour.mShape.clear();
				contour.mNumPointsShape = 0;
				for (vector<cv::Point>::const_iterator pointIt = contourIt->cbegin(); pointIt != contourIt->cend(); ++pointIt, i++) {
					if (smoothing == 0 || i % smoothing == 0) {
						vec2 point((float)pointIt->x, (float)pointIt->y);

						point *= scale;
						point += trans;

						contour.addPoint(point);
						contour.addDepth(channel.getValue(point));

						//fill shape strucutre

						if (smoothing == 0 || i % 5 == 0) {
							if (i == 0){
								contour.mShape.moveTo(point);
								contour.mNumPointsShape++;
							}
							else{
								contour.mShape.lineTo(point);
								contour.mNumPointsShape++;
							}
						}
					}
				}

				// Add contour
				if (users[cont])
				{
					contours.push_back(contour);
				}
				

			}

		}

		return contours;
	}
}