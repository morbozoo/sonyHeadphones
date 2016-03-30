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

	//KINECT
	mKinectManagerRef = kinect::KinectManager::create();
	mKinectManagerRef->setupPhysics();
	mKinectManagerRef->setupKinect();
	mKinectManagerRef->setupParticleGrid();

	//SPOUT
	setupSpout();

	//BLOMM
	mBloom = shaders::Bloom::create();
	mBloom->setupBlur(getWindowSize());
	mBloom->compileShader();

	mDrawGUI = false;
	mBloomFactor = .6;

	//draw type
	mDrawMode = 0;

//	mTimeStep = SystemVars::getInstance().speed_w;

	rateParticles = 150;
	maxNumParticles = 300;

	mNumKinects = mKinectManagerRef->getNumKinects();
	CI_LOG_I("NUM KINECTS CREATED " << mNumKinects);
		 
	mParams = params::InterfaceGl::create(getWindow(), "App parameters", toPixels(ivec2(200, 400)));
	mParams->addParam("Avg Fps", &mAvgFps, "");
	mParams->addParam("Num Kinects", &mNumKinects, "");
	mParams->addSeparator();
	//mParams->addParam("draw depth", &mKinectManagerRef->mDrawDepths, "");
	mParams->addParam("draw contours", &mDrawContours, "");
	mParams->addParam("draw Type", &mDrawMode).step(1).min(0).max(2);
	

	//mParams->addParam("draw Grid", &mKinectManagerRef->mDrawGrid, "");
	mParams->addParam("Speed Background", &mTimeStep, "min=0 max=10 step=0.1");
	mParams->addParam("Bloom Att", &mBloomFactor, "min=0 max=2 step=0.01");
	mParams->addSeparator();

	mParams->addParam("Particle rate", &rateParticles, "min=1 max=150 step=1");
	mParams->addParam("Max number particles", &maxNumParticles, "min=300 max=1000 step=1");


	////	mParams->addParam("Scale Contour X", &mKinectManagerRef->mScaleContour.x, "min=0 max=15 step=0.01");
	////	mParams->addParam("Scale Contour Y", &mKinectManagerRef->mScaleContour.y, "min=0 max=15 step=0.01");
	//mParams->addParam("Kinect Translate Y", &mKinectManagerRef->mKinectTranslateY, "min=-450 max=450 step=1");

}

// Runs update logic

void ParticleSonyApp::updateMode()
{
	switch (mDrawMode){

	case 0:
		mKinectManagerRef->updateParticlesBox2d();

		numParticlesBox2d = mKinectManagerRef->getParticleSize();

		if (getElapsedFrames() % rateParticles == 0){
			float x = ci::randFloat(60, getWindowWidth() - 80);
			float y = 150;
			mKinectManagerRef->addParticle(ci::vec2(x, y));
		}

		while (mKinectManagerRef->getParticleSize() > maxNumParticles){
			mKinectManagerRef->deleteParticle();
		}

		break;
	case 1:

		mKinectManagerRef->updateParticleGrid();
		break;
	}
}

void ParticleSonyApp::update()
{

	mAvgFps = getAverageFps();

	mBloom->setAttenuation(mBloomFactor);

	mKinectManagerRef->updateKinect();
	

	updateMode();

	
	mNumKinects  = mKinectManagerRef->getNumKinects();



}



void ParticleSonyApp::drawMode()
{
	switch (mDrawMode){

	case 0:
		mKinectManagerRef->drawParticlesBox2d();
		mKinectManagerRef->drawUpdateTriangulated();

		if (mDrawContours){
			mKinectManagerRef->drawContours();
		}
		break;

	case 1:
		mKinectManagerRef->drawParticleGrid();
		break;
	}
}

// Render
void ParticleSonyApp::draw()
{

	gl::clear(Color(0, 0, 0));

	gl::ScopedViewport scpVp(ivec2(0), getWindowSize());
	gl::ScopedMatrices matrices;
	ci::gl::setMatricesWindow(getWindowSize(), true);
	ci::gl::setModelMatrix(ci::mat4());

	
	drawMode();

	if (mDrawGUI)
		mParams->draw();

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
	}
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

