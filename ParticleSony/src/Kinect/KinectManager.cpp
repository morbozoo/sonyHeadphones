#include "KinectManager.h"

namespace kinect {

	using namespace physics;
	using namespace KinectSdk;
	using namespace contour;
	using namespace ci;

	void KinectManager::setupKinect()
	{
		//KINECT
		DeviceOptions deviceOptions;
		deviceOptions.enableSkeletonTracking(true);
		deviceOptions.enableUserTracking(true);
		deviceOptions.setDepthResolution(ImageResolution::NUI_IMAGE_RESOLUTION_320x240); //NUI_IMAGE_RESOLUTION_80x60
		deviceOptions.enableColor(false);

		// Start all available devices
		int32_t count = Kinect::getDeviceCount();
		CI_LOG_I("Num Kinects: " << count);

		for (int32_t i = 0; i < count; i++) {
			deviceOptions.setDeviceIndex(i);

			Device device;
			device.mKinect = Kinect::create();
			device.mKinect->start(deviceOptions);
			device.mKinect->removeBackground(true);
			device.mKinect->enableBinaryMode(true);

			device.mCallbackId = device.mKinect->addDepthCallback(&KinectManager::onDepthData, this);

			device.mTexture = ci::gl::Texture::create(320, 240);

			mDevices.push_back(device);

			CI_LOG_I(device.mKinect->getDeviceOptions().getDeviceIndex() << ": " << device.mKinect->getDeviceOptions().getDeviceId());
		}

		mKinectDims = ci::vec2(ci::math<int>::floor(ci::app::getWindowWidth() / (float)count), ci::app::getWindowHeight());
		mScaleContour = ci::vec2(mKinectDims.x, mKinectDims.y) / ci::vec2(320, 240);
		mKinectTranslateY = 0;

		// Initialize contour finder
		mContourFinder = contour::ContourFinder::create();

		mDrawTriangulatedContour = true;

	}

	void KinectManager::setupPhysics()
	{
		mPhysicsManager = ParticleManager::create();
		mPhysicsManager->setup();
	}

	void KinectManager::setupParticleGrid()
	{
		//PARTICLES
		float height = (float)ci::app::getWindowHeight();
		float width = (float)ci::app::getWindowWidth();

		Colorf color = Colorf::white();
		vec2 position = vec2(0, kPointSpacing);
		while (position.y < height) {
			mGridDims.x = 0;
			while (position.x < width - kPointSpacing) {

				position.x += kPointSpacing;

				color = ci::ColorA(58.f / 255.0f, 58.f / 255.0f, 80.f / 255.0f, 1.0);
				contour::Particle particle(position, color);
				float inc = mGridDims.x * (0.007);
				particle.addIncrement(inc);
				mParticles.push_back(particle);

				mGridDims.x++;
			}
			position.x = 0;// kPointSpacing * 0.5f;
			position.y += kPointSpacing * 1.5f;
			mGridDims.y++;
		}

		//Circle Batch
		gl::GlslProgRef solidShader = gl::getStockShader(gl::ShaderDef().color());
		mCircleBatch = ci::gl::Batch::create(geom::Circle().radius(CIRCLE_RAD), solidShader);

	}

	void KinectManager::updateKinect()
	{
		mNumOfUsers = 0;
		for (uint32_t i = 0; i < mDevices.size(); i++) {
			Device& device = mDevices.at(i);
			if (device.mKinect->isCapturing()) {
				device.mKinect->update();

				//contour detail
				if (device.mChannel){
					device.mContours = mContourFinder->findContours(ci::Channel8u(*device.mChannel), mScaleContour, ci::vec2(mKinectDims.x * i, mKinectTranslateY), 8);

					if (device.mContours.size() > 0){
						mNumOfUsers++;
					}
				}
			}
			else {

				// If Kinect initialization failed, try again every 90 frames
				if (ci::app::getElapsedFrames() % 90 == 0) {
					device.mKinect->start();

					CI_LOG_E("ERROR KINECT " << i + 1);
				}
			}
		}
	}

	void KinectManager::updateParticlesBox2d()
	{
		mPhysicsManager->update();

		mPhysicsManager->clean();
	}



	void KinectManager::drawParticlePointGrid(float colorR, float colorG, float colorB)
	{
		mTime += mTimeSpeed;

		mPerlin = Perlin(mOctaves, mSeed);
		int i = 0;
		int j = 0;
		for (std::vector<contour::Particle>::iterator partIt = mParticles.begin(); partIt != mParticles.end(); ++partIt) {

			//if (!partIt->isActivated()){
			float v = (mPerlin.fBm(vec3(j, i, mTime) * mFrequency) + 1.0f) / 2.0f;

			v *= v*v;
			float val = v * 4;

			ci::ColorA perlinColor = ci::ColorA(val, val, val, val);
			partIt->setVelColor(perlinColor);
			//}

			gl::pushModelMatrix();

			ci::ivec2 pos = ci::ivec2(partIt->getPosition());
			ci::ColorA outColor;

			outColor = ci::ColorA(1, 1, 1, partIt->getVelColor().r);


			//ci::ColorA col = mBackgroundImg->getPixel(pos);

			gl::ScopedColor cdol(outColor);
			gl::translate(partIt->getPosition());
			gl::scale(vec2(1.0 + partIt->getVelocity() / 2.0));
			mCircleBatch->draw();
			gl::popModelMatrix();

			if (i >= mGridDims.x){
				j++;
				i = 0;
			}
			i++;
		}
	}


	void KinectManager::drawParticleGrid(float colorR, float colorG, float colorB)
	{
		int limit = 0;

		for (int i = 0; i < mGridDims.y; i++){
			gl::ScopedColor col(ci::ColorA(colorR, colorG, colorB, 1.0));
			gl::VertBatch vertLine(GL_LINE_STRIP);
			vertLine.color(ci::ColorA(1, 1, 1, 1.0));

			for (int j = limit; j < mGridDims.x - limit; j++){

				ci::vec2 pos = mParticles.at(i*mGridDims.x + j).getPosition();
				vertLine.vertex(pos);

			}
			vertLine.draw();
		}


		for (int i = 0; i < mGridDims.x; i++){

			gl::VertBatch vertLine(GL_LINE_STRIP);
			gl::ScopedColor col(ci::ColorA(colorR, colorG, colorB, 1.0));
			//gl::ScopedColor col(ci::ColorA(1, 1, 1, 1.0));
			for (int j = 0; j < mGridDims.y; j++){
				ci::vec2 pos = mParticles.at(i + j*mGridDims.x).getPosition();
				vertLine.vertex(pos);
			}
			vertLine.draw();
		}
	}


	void KinectManager::drawParticlesLineV(float colorR, float colorG, float colorB)
	{

		int limit = 0;

		for (int i = 0; i < mGridDims.y; i++){
			gl::ScopedColor col(ci::ColorA(colorR, colorG, colorB, 1.0));
			gl::VertBatch vertLine(GL_LINE_STRIP);
			vertLine.color(ci::ColorA(1, 1, 1, 1.0));

			for (int j = limit; j < mGridDims.x - limit; j++){

				ci::vec2 pos = mParticles.at(i*mGridDims.x + j).getPosition();
				vertLine.vertex(pos);

			}
			vertLine.draw();
		}

	}

	void KinectManager::drawParticlesLineH(float colorR, float colorG, float colorB)
	{


		for (int i = 0; i < mGridDims.x; i++){

			gl::VertBatch vertLine(GL_LINE_STRIP);
			gl::ScopedColor col(ci::ColorA(colorR, colorG, colorB, 1.0));
			for (int j = 0; j < mGridDims.y; j++){
				ci::vec2 pos = mParticles.at(i + j*mGridDims.x).getPosition();
				vertLine.vertex(pos);
			}
			vertLine.draw();
		}

	}

	void KinectManager::drawUpdateTriangulated(float colR, float colG, float colB)
	{
		//update contour with physics



		for (uint32_t i = 0; i < mDevices.size(); i++) {
			Device & device = mDevices.at(i);

			for (auto & contourIt : device.mContours) {

				if (contourIt.getShapeSize() >= 3 && contourIt.getPoints().size() >= 3){
					TriMesh mesh = Triangulator(contourIt.getShape()).calcMesh(Triangulator::WINDING_ODD);


					if (mesh.getNumVertices() > 3 && mDrawTriangulatedContour){
						ci::gl::VboMeshRef	mVboMesh = ci::gl::VboMesh::create(mesh);

						if (mVboMesh){
							ci::gl::ScopedColor col(ci::ColorA(colR, colG, colB, 1.0f));
							ci::gl::enableWireframe();
							ci::gl::draw(mVboMesh);
							ci::gl::disableWireframe();
						}
					}

					mPhysicsManager->addContourTriangulation(mesh);
				}
			}
		}
	}

	void KinectManager::drawRaind(float colorR, float colorG, float colorB)
	{

	}

	void KinectManager::drawParticlesBox2d()
	{
		mPhysicsManager->draw();
	}

	void KinectManager::drawContours()
	{

		ci::gl::ScopedColor col(ci::ColorA(0, 0.7, 0.8, 1));
		for (uint32_t i = 0; i < mDevices.size(); i++) {
			Device& device = mDevices.at(i);

			for (auto contourIt = device.mContours.begin(); contourIt != device.mContours.end(); ++contourIt) {

				ci::gl::VertBatch vertLine(GL_LINE_STRIP);
				for (int i = 0; i < contourIt->getPoints().size(); i++){
					ci::vec2 pos = contourIt->getPoint(i);
					vertLine.vertex(pos);
				}
				vertLine.draw();
			}
		}


	}



	void KinectManager::onDepthData(ci::Surface16u surface, const KinectSdk::DeviceOptions &deviceOptions)
	{
		int32_t index = deviceOptions.getDeviceIndex();
		for (size_t i = 0; i < mDevices.size(); ++i) {
			if (index == mDevices.at(i).mKinect->getDeviceOptions().getDeviceIndex()) {
				mDevices.at(i).mTexture = ci::gl::Texture::create(surface);
				mDevices.at(i).mChannel = ci::Channel16u::create(surface.getChannelRed());
				break;
			}
		}
	}


	void KinectManager::addParticle(ci::vec2 & pos)
	{
		mPhysicsManager->addParticle(pos);
	}

	void KinectManager::deleteParticle()
	{
		mPhysicsManager->deleteParticle();
	}

	int KinectManager::getParticleSize()
	{
		return mPhysicsManager->getNumParticles();
	}

	void KinectManager::cleanUp()
	{
		for (uint32_t i = 0; i < mDevices.size(); i++) {
			Device & device = mDevices.at(i);
			device.mKinect->removeCallback(device.mCallbackId);
			//device.mKinect->removeCallback(device.mSkeletonCallbackId);
			device.mKinect->stop();
		}
		mDevices.clear();
	}







	//PARTICLES Grid update
	void KinectManager::updateParticleGrid()
	{

		for (std::vector<contour::Particle>::iterator partIt = mParticles.begin(); partIt != mParticles.end(); ++partIt) {
			contour::Particle& particle = *partIt;


			// Iterate through contours
			for (uint32_t k = 0; k < mDevices.size(); k++) {
				Device& device = mDevices.at(k);
				if (device.mKinect->isCapturing()) {

					// Get particle position components
					float x = particle.getPosition().x;
					float y = particle.getPosition().y;

					for (std::vector<contour::Contour>::const_iterator contourIt = device.mContours.cbegin(); contourIt != device.mContours.cend(); ++contourIt) {
						const contour::Contour& contour = *contourIt;

						// Iterate through outline to determine if particle is inside 
						// contour and which point in outline is the closest
						bool isInsideContour = false;
						float minDistance = kFloatMax;
						vec2 closestPoint(kFloatMax, kFloatMax);

						vec2 centroid = contour.getCentroid();
						uint32_t count = contour.getPoints().size();
						uint32_t j = count - 1;
						for (uint32_t i = 0; i < count; i++) {

							ci::vec2 a = contour.getPoints().at(i);
							ci::vec2 b = contour.getPoints().at(j);
							float x1 = a.x;
							float y1 = a.y;
							float x2 = b.x;
							float y2 = b.y;

							// Apply ray-cast algorithm to determine if point is inside contour
							if (((y1 < y && y2 >= y) ||
								(y2 < y && y1 >= y)) &&
								(x1 <= x || x2 <= x)) {
								isInsideContour ^= (x1 + (y - y1) / (y2 - y1) * (x2 - x1) < x);
							}
							j = i;

							// Find closest point
							float distance = distance2(particle.getPosition(), a);
							if (distance < minDistance) {
								closestPoint = a;
								minDistance = distance;
							}
						}

						// Set particle acceleration away from center
						vec2 dir = particle.getPosition() - centroid;
						float dist = distance(particle.getPosition(), centroid);
						if (dist < kInteractiveRadius) {
							float amp = (1.0f - (dist / kInteractiveRadius)) * kInteractiveForce;
							particle.addAcceleration(dir * amp);
							particle.activateParticle();
						}
						

						// Particle is inside contour
						dir = closestPoint - particle.getPosition();
						float distance = length(dir);
						if (isInsideContour) {

							// Set particle acceleration towards closest point in outline
							distance = math<float>::max(kInteractiveRadius - distance, 0.0f);
							if (distance > 0.0f) {
								float amp = (distance / kInteractiveRadius) * kInteractiveForce;
								particle.addAcceleration(dir * amp);
								particle.activateParticle();
							}
						}

					}
				}
			}
			// Update particle
			partIt->update();
		}
	}

}