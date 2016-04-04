#include "ParticleSonyApp.h"

// Imports
using namespace ci;
using namespace ci::app;

using namespace std;
using namespace contour;

// Prepare window
void ParticleSonyApp::prepareSettings(Settings *settings)
{
	// Read application settings from file.
	config().setOptions(Config::Options().setIfDefault(true));
	config().read(loadFile(findPath("config") / "ParticleSonyApp.cfg"));

	bool fullScreen = config().get("application", "start_full_screen", false);
	ivec2 canvasSize = config().get("application", "canvas_size", ivec2(640, 480));
	ivec2 startPosition = config().get("application", "start_position", ivec2(0, 0));
	int mNumKinects = config().get("application", "numKinects", 1);

	SystemVars::getInstance().numKinects = mNumKinects;

	ci::app::console() << startPosition << std::endl;
	ci::app::console() << canvasSize << std::endl;
	ci::app::console() << mNumKinects << std::endl;

	//settings->setWindowSize(KINECT_WIDTH * NUMKINECTS, HEIGHT);
	settings->setWindowSize(1024 * 2, 768);
	settings->disableFrameRate();
	settings->setResizable(false);
	settings->setWindowPos(startPosition.x, startPosition.y);
	settings->setBorderless(true);
}

void ParticleSonyApp::setupSpout()
{
	g_Width = getWindowWidth();
	g_Height = getWindowHeight();


	// -------- SPOUT -------------
	// Set up the texture we will use to send out
	// We grab the screen so it has to be the same size
	spoutTexture = gl::Texture2d::create(g_Width, g_Height);
	strcpy_s(SenderName, "CINDER Spout SDK Sender");
	// Initialize a sender
	bInitialized = spoutsender.CreateSender(SenderName, g_Width, g_Height);

	// Optionally test for texture share compatibility
	// bMemoryMode informs us whether Spout initialized for texture share or memory share
	bMemoryMode = spoutsender.GetMemoryShareMode();
}


// SETUP
///-----------------------------------------------
void ParticleSonyApp::setup()
{
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SIZE);
	glEnable(GL_LINE_WIDTH);

	//gl::disableVerticalSync();


	//variable to control gravity of the box2d
	//change this variable to have a different behavior 
	ci::vec2 gravity(0.6, 3.3);

	//KINECT
	mKinectManagerRef = kinect::KinectManager::create();
	mKinectManagerRef->setupPhysics(gravity);
	mKinectManagerRef->setupKinect();
	mKinectManagerRef->setupParticleGrid();

	//SPOUT
	setupSpout();

	//BLOMM
	mBloom = shaders::Bloom::create();
	mBloom->setAttenuation(1.5);
	mBloom->setupBlur(getWindowSize());
	mBloom->compileShader();

	mDrawGUI = true;
	mBloomFactor = .6;

	//draw type
	mDrawMode = 1;

	//init values
	mRateParticles	 = 15;
	mMaxNumParticles = 300;
	mChangeColor	 = 0;
	mDuration		 = 1.0f;
	mEnableBloom	 = true;

	mPerlinValue = 0.3;

	mNumKinects = mKinectManagerRef->getNumKinects();
	CI_LOG_I("NUM KINECTS CREATED " << mNumKinects);
		 
	mParams = params::InterfaceGl::create(getWindow(), "App parameters", toPixels(ivec2(200, 400)));
	mParams->addParam("Avg Fps", &mAvgFps, "");
	mParams->addParam("Num Kinects", &mNumKinects, "");
	mParams->addSeparator();
	//mParams->addParam("draw depth", &mKinectManagerRef->mDrawDepths, "");
	mParams->addParam("draw contours", &mDrawContours, "");
	mParams->addParam("draw Type", &mDrawMode).step(1).min(0).max(10);

	//mParams->addParam("draw Grid", &mKinectManagerRef->mDrawGrid, "");
	mParams->addParam("Speed Background", &mTimeStep, "min=0 max=10 step=0.1");

	mParams->addParam("Enable Bloom", &mEnableBloom);
	mParams->addParam("Bloom Att", &mBloomFactor, "min=0 max=2 step=0.01");
	mParams->addSeparator();


	mParams->addParam("mPerlinValue", &mPerlinValue).step(0.01).min(0.01).max(1.0);
	

	mParams->addParam("Particle rate", &mRateParticles, "min=1 max=150 step=1");
	mParams->addParam("Max number particles", &mMaxNumParticles, "min=3 max=1000 step=2");
	mParams->addParam("Change Color", &mChangeColor, "min=0 max=1 step=1");
	mParams->addParam("duration", &mDuration, "min=0.5 max=5.0 step=0.1");

	////	mParams->addParam("Scale Contour X", &mKinectManagerRef->mScaleContour.x, "min=0 max=15 step=0.01");
	////	mParams->addParam("Scale Contour Y", &mKinectManagerRef->mScaleContour.y, "min=0 max=15 step=0.01");
	//mParams->addParam("Kinect Translate Y", &mKinectManagerRef->mKinectTranslateY, "min=-450 max=450 step=1");

	colorR = 1.0f;
	colorG = 1.0f;
	colorB = 1.0f;

	setupOsc();

}

// Runs update logic

void ParticleSonyApp::updateBox2D()
{
	mKinectManagerRef->updateParticlesBox2d();

	mNumParticlesBox2d = mKinectManagerRef->getParticleSize();

	if (getElapsedFrames() % mRateParticles == 0){
		float x = ci::randFloat(60, getWindowWidth() - 80);
		float y = 50;
		mKinectManagerRef->addParticle(ci::vec2(x, y));
	}

	while (mKinectManagerRef->getParticleSize() > mMaxNumParticles){
		mKinectManagerRef->deleteParticle();
	}
}

void ParticleSonyApp::updateMode()
{
	switch (mDrawMode){

	case 0:
		updateBox2D();
		break;
	case 1:
		updateBox2D();
		break;
	case 2:
		updateBox2D();
		break;
	case 3:
		updateBox2D();
		break;


	case 4:
		mKinectManagerRef->updateParticleGrid();
		break;
	case 5:
		mKinectManagerRef->updateParticleGrid();
		break;
	case 6:
		mKinectManagerRef->updateParticleGrid();
		break;
	case 7:
		mKinectManagerRef->updateParticleGrid();
		break;
	case 8:
		mKinectManagerRef->updateParticleGrid();
		break;
	}
}


//--- offscreen rendering
void ParticleSonyApp::offScreenRendering()
{

	mBloom->bindFboScene();
	drawMode();
	mBloom->unbindFboScene();

	if (mEnableBloom){
		mBloom->updateBlur();
	}
}

void ParticleSonyApp::update()
{

	mAvgFps = getAverageFps();

	mBloom->setAttenuation(mBloomFactor);

	mKinectManagerRef->updateKinect();

	mKinectManagerRef->setTimePerlin(mPerlinValue);
	
	mNumKinects = mKinectManagerRef->getNumKinects();

	//update modes
	updateMode();

	//offscreen rendering
	offScreenRendering();
}


void ParticleSonyApp::timeColor(float colR, float colG, float colB) 
{
	timeline().apply(&colorR, colR, mDuration, EaseInCubic());
	timeline().apply(&colorG, colG, mDuration, EaseInCubic());
	timeline().apply(&colorB, colB, mDuration, EaseInCubic());
}


void ParticleSonyApp::drawBox2D()
{
	mKinectManagerRef->drawParticlesBox2d(ci::ColorA(colorR, colorG, colorB, 1.0));
	mKinectManagerRef->drawUpdateTriangulated(colorR, colorG, colorB);

	if (mDrawContours){
		mKinectManagerRef->drawContours();
	}
}

void ParticleSonyApp::drawMode()
{

	gl::ScopedViewport scpView(ci::vec2(0), mBloom->getSize());
	gl::setMatricesWindow(mBloom->getSize());
	ci::gl::setModelMatrix(ci::mat4());
	
	gl::clear();

	switch (mDrawMode){

	case 0:
		drawBox2D();
		mKinectManagerRef->setBox2dDrawMode(0);
		break;;
	case 1:
		drawBox2D();
		mKinectManagerRef->setBox2dDrawMode(1);
		break;
	case 2:
		drawBox2D();
		mKinectManagerRef->setBox2dDrawMode(2);
		break;

	case 3:
		drawBox2D();
		mKinectManagerRef->setBox2dDrawMode(2);
		break;

	case 4:
		mKinectManagerRef->drawParticleSquareGrid(colorR, colorG, colorB);
		break;
	case 5:
		mKinectManagerRef->drawParticlePointGrid(colorR, colorG, colorB);
		break;
	case 6:
		mKinectManagerRef->drawParticleGrid(colorR, colorG, colorB);
		break;
	case 7:
		mKinectManagerRef->drawParticlesLineH(colorR, colorG, colorB);
		break;
	case 8:
		mKinectManagerRef->drawParticlesLineV(colorR, colorG, colorB);
		break;
	}
}

// Render
void ParticleSonyApp::draw()
{

	gl::clear(Color(0, 0, 0));


	if (mEnableBloom){
		mBloom->drawBlur();
	}
	else{
		ci::gl::pushModelView();

		gl::ScopedViewport scpView(ci::vec2(0), getWindowSize());
		gl::setMatricesWindow(getWindowSize());
		ci::gl::setModelMatrix(ci::mat4());

		ci::gl::color(ci::Color::white());
		ci::gl::draw(mBloom->getFBOScene()->getColorTexture(), getWindowSize());

		ci::gl::popModelView();
	}

	if (mDrawGUI){
		mParams->draw();
	}

	sendSpout();

}

// -------- SPOUT ------------
void ParticleSonyApp::sendSpout()
{
	if (bInitialized) {

		// Grab the screen (current read buffer) into the local spout texture
		{
			gl::ScopedTextureBind scpTex(spoutTexture);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_Width, g_Height);
		}

		// Send the texture for all receivers to use
		// NOTE : if SendTexture is called with a framebuffer object bound,
		// include the FBO id as an argument so that the binding is restored afterwards
		// because Spout uses an fbo for intermediate rendering
		spoutsender.SendTexture(spoutTexture->getId(), spoutTexture->getTarget(), g_Width, g_Height);
	}
}

// Handles key press
void ParticleSonyApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_1:
		//mKinectManagerRef->mDrawParticles = true;
		//mKinectManagerRef->mDrawGrid = false;
		break;
	case KeyEvent::KEY_2:
		//mKinectManagerRef->mDrawParticles = true;
		//mKinectManagerRef->mDrawGrid = true;
		break;
	case KeyEvent::KEY_q:
		quit();
		break;
	case KeyEvent::KEY_f:
		setFullScreen(!isFullScreen());
		break;
	case KeyEvent::KEY_g:
		mDrawGUI = !mDrawGUI;
		break;
	case KeyEvent::KEY_p:
		//mKinectManagerRef->tooglePulse = true;
		console() << "pulse" << std::endl;
		break;
	case KeyEvent::KEY_a:
		mDrawMode = 1;
		timeColor(1.0f, 1.0f, 0.0f);
		break;
	case KeyEvent::KEY_s:
		mDrawMode = 0;
		timeColor(1.0f, 0.0f, 1.0f);
		break;
	case KeyEvent::KEY_z:
		setMood(1);
		break;
	case KeyEvent::KEY_x:
		setMood(2);
		break;
	}
}

void ParticleSonyApp::setMood(int cual)
{
	ci::osc::Message message;
	message.setAddress("/setMood/");
	switch (cual) {
	case 1:
		colorR = 0;
		colorG = 0;
		colorB = 0;
		mDrawMode = 1;
		timeColor(1.0f, 1.0f, 0.0f);
		message.addIntArg(1);
		break;

	case 2:
		colorR = 0;
		colorG = 0;
		colorB = 0;
		mDrawMode = 0;
		timeColor(1.0f, 0.0f, 1.0f);
		message.addIntArg(2);
		break;
	}
	sender.sendMessage(message);
}

void ParticleSonyApp::setupOsc()
{
	sender.setup(destinationHost, destinationPort, true);
	listener.setup(12345);
}

void ParticleSonyApp::updateOsc()
{

}

// Called on exit
void ParticleSonyApp::shutdown()
{
	mKinectManagerRef->cleanUp();
}


fs::path ParticleSonyApp::findPath(const fs::path &folder)
{
	// first search the local directory, then its parent, up to root level
	// check at least the app path, even if it has no parent directory
	auto execPath = ci::app::App::get()->getAppPath();// getAppPath();// getExecutablePath();


	if (execPath.empty()){
		execPath = ci::app::getAppPath();
	}

	if (execPath.empty())
		return folder;

	size_t parentCt = 0;
	for (fs::path curPath = execPath; curPath.has_parent_path() || (curPath == execPath); curPath = curPath.parent_path(), ++parentCt) {
		const fs::path curDir = curPath / folder;
		if (fs::exists(curDir) && fs::is_directory(curDir)) {
			return curDir;
		}
	}

	// No dir found, create it next to executable.
	fs::path curPath = execPath / folder;
	fs::create_directories(curPath);

	return curPath;
}


// Run application
//RendererGl::Options().msaa(16))
CINDER_APP(ParticleSonyApp, RendererGl(RendererGl::Options().msaa(32)), &ParticleSonyApp::prepareSettings)

