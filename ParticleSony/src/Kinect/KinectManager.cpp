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

			device.mSkeletonCallbackId = device.mKinect->addSkeletonTrackingCallback(&KinectManager::onSkeletonData, this);

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

	void KinectManager::setupPhysics(ci::vec2 gravity)
	{
		mBox2dManager = ParticleManager::create();
		mBox2dManager->setup(gravity);
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
		mCircleBatch = ci::gl::Batch::create(geom::Circle().radius(1).subdivisions(32), solidShader);

		//square
		mSquareBatch = ci::gl::Batch::create(geom::Rect(), solidShader);

		mSquareContourBatch  = ci::gl::Batch::create(geom::WireRoundedRect().cornerRadius(0.0), solidShader);

		mCircleContourBatch = ci::gl::Batch::create(geom::WireCircle().subdivisions(32), solidShader);

		mSeed = clock() & 65535;
		mOctaves = 8;
		mFrequency = 1 / 200.0f;
		mTimeSpeed = 0.85;
		mTime = 0.0;
	}

	void KinectManager::updateKinect(float scaleX1, float scaleY1, int transX1, int transY1, float scaleX2, float scaleY2, int transX2, int transY2, std::vector<bool> usersK1, std::vector<bool> usersK2)
	{
		mNumOfUsers = 0;
		for (uint32_t i = 0; i < mDevices.size(); i++) {
			Device& device = mDevices.at(i);
			if (device.mKinect->isCapturing()) {
				device.mKinect->update();

				//contour detail
				if (device.mChannel){
					if (i == 1)
					{
						vec2 scale1;
						vec2 trans1;
						scale1.x = mScaleContour.x * scaleX1;
						scale1.y = mScaleContour.y * scaleY1;
						trans1.x = (mKinectDims.x * i) + transX1;
						trans1.y = mKinectTranslateY + transY1;
						device.mContours = mContourFinder->findContours(ci::Channel8u(*device.mChannel), scale1, trans1, usersK1, 8);
						//device.mContours = mContourFinder->findContours(ci::Channel8u(*device.mChannel), mScaleContour, ci::vec2(mKinectDims.x * i, mKinectTranslateY), 8);
					}
					else
					{
						vec2 scale2;
						vec2 trans2;
						scale2.x = mScaleContour.x * scaleX2;
						scale2.y = mScaleContour.y * scaleY2;
						trans2.x = (mKinectDims.x * i) + transX2;
						trans2.y = mKinectTranslateY + transY2;
						device.mContours = mContourFinder->findContours(ci::Channel8u(*device.mChannel), scale2, trans2, usersK2, 8);
					//
						//device.mContours = mContourFinder->findContours(ci::Channel8u(*device.mChannel), mScaleContour, ci::vec2(mKinectDims.x * i, mKinectTranslateY), 8);
					}
					//device.mContours = mContourFinder->findContours(ci::Channel8u(*device.mChannel), mScaleContour, ci::vec2(mKinectDims.x * i, mKinectTranslateY), 8);

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

	//draw skeleton
	void KinectManager::drawSkeleton()
	{

		gl::setMatricesWindow(app::getWindowSize());
		for (uint32_t i = 0; i < mDevices.size(); i++) {
			Device & device = mDevices.at(i);
			// We're capturing
			if (device.mKinect->isCapturing()) {

				// Set up 3D view
				//gl::setMatrices(mCamera);
				gl::pushMatrices();
				//gl::scale(vec2(app::getWindowSize()) / vec2(mTexture.getSize()));

				// Iterate through skeletons
				uint32_t i = 0;
				for (std::vector<Skeleton>::const_iterator skeletonIt = device.mSkeletons.cbegin(); skeletonIt != device.mSkeletons.cend(); ++skeletonIt, i++) {

					// Set color
					Colorf color = device.mKinect->getUserColor(i);

					// Iterate through joints
					for (Skeleton::const_iterator boneIt = skeletonIt->cbegin(); boneIt != skeletonIt->cend(); ++boneIt) {

						// Set user color
						gl::color(color);

						const Bone& bone = boneIt->second;
						vec3 position = bone.getPosition();
						vec3 destination = skeletonIt->at(bone.getStartJoint()).getPosition();
						vec2 positionScreen = vec2(device.mKinect->getSkeletonColorPos(position));
						vec2 destinationSceen = vec2(device.mKinect->getSkeletonColorPos(destination));

						// Draw bone
						gl::drawLine(positionScreen, destinationSceen);

						// Draw joint
						gl::drawSolidCircle(positionScreen, 10.0f, 16);


					}

				}
			}
		}
	}

	void KinectManager::updateParticlesBox2d()
	{
		mBox2dManager->update();

		mBox2dManager->clean();
	}

	//------------RAIN
	void   KinectManager::setupRainParticles()
	{
		int binPower = 3;

		particleSystemBin.setup(ci::app::getWindowWidth(), ci::app::getWindowHeight(), binPower);

		kParticles = 2;
		float padding = 0;
		float maxVelocity = .35;
		for (int i = 0; i < kParticles * 1024; i++) {
			float x = ci::Rand::randFloat(padding, ci::app::getWindowWidth() - padding);
			float y = ci::Rand::randFloat(padding, ci::app::getWindowHeight() - padding);
			float xv = ci::Rand::randFloat(-maxVelocity, maxVelocity);
			float yv = ci::Rand::randFloat(-maxVelocity, maxVelocity);
			bin::Particle particle(x, y, xv, yv);
			particleSystemBin.add(particle);
		}

		timeStep = 1;
		lineOpacity = 0.12f;
		pointOpacity = 0.5f;
		slowMotion = false;
		particleNeighborhood = 4;
		particleRepulsion = 0.5;
		centerAttraction = .05;
	}

	void KinectManager::updateRainParticles()
	{
		particleSystemBin.setTimeStep(timeStep);
	}

	void KinectManager::updateContourBinParticles()
	{
	
		// Iterate through particles
		for (std::vector<contour::Particle>::iterator partIt = mParticles.begin(); partIt != mParticles.end(); ++partIt){
			contour::Particle& particle = *partIt;

			// Get particle position components
			float x = particle.getPosition().x;
			float y = particle.getPosition().y;

			// Iterate through contours
			for (uint32_t k = 0; k < mDevices.size(); k++) {
				Device& device = mDevices.at(k);
				if (device.mKinect->isCapturing()) {
					for (auto contourIt = device.mContours.cbegin(); contourIt != device.mContours.cend(); ++contourIt) {
						const contour::Contour& contour = *contourIt;

						// Iterate through outline to determine if particle is inside 
						// contour and which point in outline is the closest
						bool isInsideContour = false;
						float minDistance = kFloatMax;
						ci::vec2 closestPoint(kFloatMax, kFloatMax);

						ci::vec2 centroid = contour.getCentroid();
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
						}

						if (isInsideContour) {
							particleSystemBin.addRepulsionForce(x, y, 34, 0.6);
						}

					}
				}
			}
		}
		
	}


	void KinectManager::drawRain(float colorR, float colorG, float colorB)
	{
		ci::gl::enableAdditiveBlending();
		gl::color(ci::ColorA(1.0f, 1.0f, 1.0f, lineOpacity));

		particleSystemBin.setupForces();


		//	glBegin(GL_LINES);
		for (int i = 0; i < particleSystemBin.size(); i++) {
			bin::Particle& cur = particleSystemBin[i];
			// global force on other particles
			particleSystemBin.addRepulsionForce(cur, particleNeighborhood, particleRepulsion);
			// forces on this particle
			cur.throughWallSBottom(0, ci::app::getWindowWidth(), ci::app::getWindowHeight());
			//cur.bounceOffWalls(0, 0, getWindowWidth(), getWindowHeight());
			cur.addDampingForce();
		}

		updateContourBinParticles();
		//glEnd();
		// single global forces
		//particleSystem.addAttractionForce(getWindowWidth()/2, getWindowHeight()/2, getWindowWidth(), centerAttraction);
		if (isMousePressed)
			particleSystemBin.addRepulsionForce(mouse.x, mouse.y, 100, 10);

		particleSystemBin.update();
		gl::color(ci::ColorA(1.0f, 1.0f, 1.0f, pointOpacity));
		particleSystemBin.drawRain();
		ci::gl::disableAlphaBlending();

		gl::color(1, 1, 1);
		ci::gl::enableAdditiveBlending();
	}


	//DRAW METHODS
	void KinectManager::drawParticleSquareGrid(float colorR, float colorG, float colorB)
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

			outColor = ci::ColorA(colorR, colorG, colorB, partIt->getVelColor().r);


			//ci::ColorA col = mBackgroundImg->getPixel(pos);
			{
				gl::ScopedColor cdol(outColor);
				gl::translate(partIt->getPosition());
				gl::scale(vec2(1.0 + partIt->getVelocity() / 3.0 + val * 10));

				mSquareBatch->draw();
			}

			//contour
			{
				gl::ScopedColor cdol(ci::ColorA(1, 1, 1, 1));
				mSquareContourBatch->draw();
			}

			gl::popModelMatrix();

			if (i >= mGridDims.x){
				j++;
				i = 0;
			}
			i++;
		}

	}

	void KinectManager::drawParticlePointGrid(float colorR, float colorG, float colorB)
	{
		mTime += mTimeSpeed;

		mPerlin = Perlin(mOctaves, mSeed);
		int i = 0;
		int j = 0; 

		for (std::vector<contour::Particle>::iterator partIt = mParticles.begin(); partIt != mParticles.end(); ++partIt) {

			ci::ivec2 pos = ci::ivec2(partIt->getPosition());

			float v = (mPerlin.fBm(vec3(i, j, mTime) * mFrequency) + 1.0f) / 2.0f;

			v *= v*v;
			float val = v * 4;

			ci::ColorA perlinColor = ci::ColorA(val, val, val, val);
			partIt->setVelColor(perlinColor);

			gl::pushModelMatrix();
			ci::ColorA outColor = ci::ColorA(colorR, colorG, colorB, partIt->getVelColor().r);

			{
				//outer blue
				gl::translate(partIt->getPosition());
				gl::scale(vec2(3.0 + partIt->getVelocity() / 2.5 + partIt->getVelColor().r*22.0));

				{
					gl::ScopedColor cdol(outColor);
					mCircleBatch->draw();
				}

				//inser black
				gl::scale(vec2(0.6));
				{
					gl::ScopedColor dol(ci::ColorA(0, 0, 0, 0.8));
					mCircleBatch->draw();
				}


				//contour
				gl::scale(vec2(1.1));
				{
					gl::ScopedColor cdol(ci::ColorA(1, 1, 1, 1));
					mCircleContourBatch->draw();
				}
				
			}

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

		mTime += mTimeSpeed;
		mPerlin = Perlin(mOctaves, mSeed);


		for (int i = 0; i < mGridDims.y; i++){
			

			float v = (mPerlin.fBm(vec3(i / 2.0, i, mTime) * mFrequency) + 1.0f) / 2.0f;
			v *= v*v;
			float val = v * 4;

			gl::ScopedColor col(ci::ColorA(colorR, colorG, colorB, 1.0));
			gl::VertBatch vertLine(GL_LINE_STRIP);
			gl::ScopedLineWidth scpLineWidth(1.0f + v*8.0);

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

			float v = (mPerlin.fBm(vec3(i / 2.0, i, mTime) * mFrequency) + 1.0f) / 2.0f;
			v *= v*v;
			float val = v * 4;
			gl::ScopedLineWidth scpLineWidth(1.0f + v*9.0);


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

		mTime += mTimeSpeed;
		mPerlin = Perlin(mOctaves, mSeed);


		for (int i = 0; i < mGridDims.y; i++){
			gl::ScopedColor col(ci::ColorA(colorR, colorG, colorB, 1.0));
			gl::VertBatch vertLine(GL_LINE_STRIP);

			float v = (mPerlin.fBm(vec3(i/2.0, i, mTime) * mFrequency) + 1.0f) / 2.0f;
			v *= v*v;
			float val = v * 4;

			gl::ScopedLineWidth scpLineWidth(1.0 + val*9.0f);

			for (int j = limit; j < mGridDims.x - limit; j++){

				ci::vec2 pos = mParticles.at(i*mGridDims.x + j).getPosition();
				vertLine.vertex(pos);

			}
			vertLine.draw();
		}

	}

	void KinectManager::drawParticlesLineH(float colorR, float colorG, float colorB)
	{

		mTime += mTimeSpeed;
		mPerlin = Perlin(mOctaves, mSeed);

		for (int i = 0; i < mGridDims.x; i++){

			gl::VertBatch vertLine(GL_LINE_STRIP);
			gl::ScopedColor col(ci::ColorA(colorR, colorG, colorB, 1.0));

			float v = (mPerlin.fBm(vec3(i, i/2.0, mTime) * mFrequency) + 1.0f) / 2.0f;
			v *= v*v;
			float val = v * 4;

			gl::ScopedLineWidth scpLineWidth(1.0 + val*7.0f);

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

					mBox2dManager->addContourTriangulation(mesh);
				}
			}
		}
	}

	void KinectManager::drawParticlesBox2d(ci::ColorA col)
	{
		mBox2dManager->draw(col);
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

	void KinectManager::drawContoursAlter(float colorR, float colorG, float colorB)
	{

		ci::gl::ScopedColor col(ci::ColorA(colorR, colorG, colorB, 1));
		for (uint32_t i = 0; i < mDevices.size(); i++) {
			Device& device = mDevices.at(i);

			for (auto contourIt = device.mContours.begin(); contourIt != device.mContours.end(); ++contourIt) {
				ci::vec2 mostLeft = contourIt->getPoint(0);
				ci::vec2 mostRight = contourIt->getPoint(0);
				ci::vec2 top = contourIt->getPoint(0);
				ci::vec2 bottomRight = contourIt->getPoint(0);
				ci::vec2 bottomLeft = contourIt->getPoint(0);
				//ci::gl::VertBatch vertLine(GL_LINE_STRIP);
				ci::gl::VertBatch vertFill(GL_TRIANGLES);
				for (int i = 0; i < contourIt->getPoints().size(); i++) {
					ci::vec2 pos = contourIt->getPoint(i);
					if (pos.x < mostLeft.x)
					{
						mostLeft = pos;

					}
					else if (pos.x > mostRight.x)
					{
						mostRight = pos;
					}
					else if (pos.y < top.y)
					{
						top = pos;
					}
					else if (pos.y > bottomLeft.y && pos.x < top.x)
					{
						bottomLeft = pos;
					}
					else if (pos.y > bottomRight.y && pos.x > top.x)
					{
						bottomRight = pos;
					}
					
				}

				vertFill.vertex(top);
				vertFill.vertex(mostLeft);
				vertFill.vertex(bottomLeft);

				vertFill.vertex(top);
				vertFill.vertex(bottomLeft);
				vertFill.vertex(bottomRight);

				vertFill.vertex(top);
				vertFill.vertex(bottomRight);
				vertFill.vertex(mostRight);

				vertFill.draw();
			}
		}


	}




	void KinectManager::addParticle(ci::vec2 & pos)
	{
		mBox2dManager->addParticle(pos);
	}

	void KinectManager::deleteParticle()
	{
		mBox2dManager->deleteParticle();
	}

	int KinectManager::getParticleSize()
	{
		return mBox2dManager->getNumParticles();
	}

	//depth
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

	//skeleton
	void KinectManager::onSkeletonData(std::vector<Skeleton> skeletons, const DeviceOptions &deviceOptions)
	{
		int32_t index = deviceOptions.getDeviceIndex();
		for (size_t i = 0; i < mDevices.size(); ++i) {
			if (index == mDevices.at(i).mKinect->getDeviceOptions().getDeviceIndex()) {
				mDevices.at(i).mSkeletons = skeletons;
				break;
			}
		}
	}

	//clean
	void KinectManager::cleanUp()
	{
		for (uint32_t i = 0; i < mDevices.size(); i++) {
			Device & device = mDevices.at(i);
			device.mKinect->removeCallback(device.mCallbackId);
			device.mKinect->removeCallback(device.mSkeletonCallbackId);
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