#include "ParticlesBox2D.h"

namespace physics{


	using namespace ci;
	using namespace ci::app;
	using namespace std;

	ParticleBox::ParticleBox(const b2Body* b) :
		b2b(b), mDead(false)
	{
		life = ci::randFloat(80, 140);
	}


	void ParticleBox::draw()
	{
		//ci::gl::drawSolidCircle()
	}

	void ParticleBox::update()
	{

		if (life <= 0.0){
			mDead = true;
		}
		else{
			life -= 0.04;
		}
	}

	///////////////////////////////////////////////////////////////

	void ParticleManager::setup()
	{

		//setup the world
		b2Vec2 gravity(0.5f, 3.3f);
		mWorld = new b2World(gravity);

		b2BodyDef groundBodyDef;
		groundBodyDef.position.Set(0, 0);// ci::app::getWindowWidth() / 2.0f, ci::app::getWindowHeight() / 2.0f); //, ci::app::getWindowHeight());//
		b2Body* groundBody = mWorld->CreateBody(&groundBodyDef);

		// Define the ground box shape.
		//b2PolygonShape groundBox;

		// The extents are the half-widths of the box.
		//groundBox.SetAsBox(ci::app::getWindowWidth(), ci::app::getWindowHeight());

		// Add the ground fixture to the ground body.
		//groundBody->CreateFixture(&groundBox, 0.0f);


		b2EdgeShape groundBox;
		// bottom
		groundBox.Set(b2Vec2(-BOX_SIZE, 0), b2Vec2(ci::app::getWindowWidth() + BOX_SIZE, 0));
		groundBody->CreateFixture(&groundBox, 0);

		// top
		groundBox.Set(b2Vec2(-BOX_SIZE, ci::app::getWindowHeight()),
			b2Vec2(ci::app::getWindowWidth() + BOX_SIZE, ci::app::getWindowHeight()));
		groundBody->CreateFixture(&groundBox, 0);

		// left
		groundBox.Set(b2Vec2(-BOX_SIZE, ci::app::getWindowHeight()), b2Vec2(-BOX_SIZE, 0));
		groundBody->CreateFixture(&groundBox, 0);

		// right
		groundBox.Set(b2Vec2(ci::app::getWindowWidth() + BOX_SIZE, ci::app::getWindowHeight()),
			b2Vec2(ci::app::getWindowWidth() + BOX_SIZE, 0));
		groundBody->CreateFixture(&groundBox, 0);


		try{
			mParticleTexture = ci::gl::Texture2d::create(ci::loadImage(ci::app::loadAsset("Copo_real.png")));
		}
		catch (std::exception e){
			CI_LOG_I("cannot load particle image");
		}
	}

	void ParticleManager::draw()
	{
		//iterate through the boxes
		for (auto & particleBox : mBoxes){
			//b2Body * b2b = particleBox->getB2Body();
			const b2Body * b2b = particleBox->getB2Body();

			particleBox->update();
			float vel = (abs(b2b->GetLinearVelocity().x) + abs(b2b->GetLinearVelocity().y));

			ci::gl::ScopedMatrices mat;
			ci::gl::ScopedModelMatrix model;
			ci::gl::translate(b2b->GetPosition().x, b2b->GetPosition().y);
			ci::gl::rotate(b2b->GetAngle());

			//ci::gl::ScopedColor color(ci::ColorA(0.7 + vel * 2, 0.7 + vel * 2, 0.7 + vel * 2, 0.8 + vel * 3));
			ci::gl::ScopedColor colo(ci::ColorA(0.95, 0.95, 0.95, 1.0));
			//ci::gl::drawSolidRect(ci::Rectf(-BOX_SIZE, -BOX_SIZE, BOX_SIZE, BOX_SIZE));

			//ci::gl::scale(4.5, 4.5);
			//ci::gl::draw(mTexture, ci::Rectf(0, 0, particleBox->getSize() / 2.2f, particleBox->getSize() / 2.2f));
			ci::gl::drawSolidCircle(ci::vec2(0), particleBox->getSize() + 2);
			//ci::gl::drawSolidCircle(ci::vec2(b2b->GetPosition().x, b2b->GetPosition().y), 5);
			//particleBox->draw();

			//ci::app::console() << b2b->GetPosition().x << b2b->GetPosition().y << std::endl;

		}
	}

	void ParticleManager::update()
	{
		for (int i = 0; i < 5; ++i){
			mWorld->Step(1 / 30.0f, 10, 10);
		}
	}

	void ParticleManager::clean()
	{

		//CI_LOG_I("START CLEANING");
		//delete only once every time step of the application
		for (auto & ids : mDeleteIndex) {
			int index = ids.second;
			if (index <= mBoxes.size() - 1){
				if (mBoxes.at(index)->isDead()){
					try{
						if (mBoxes.at(index)->getB2Body() != NULL){
							const b2Body * b2b = mBoxes.at(index)->getB2Body();
							mWorld->DestroyBody(const_cast<b2Body*>(b2b));
							mBoxes.erase(mBoxes.begin() + index);
							CI_LOG_V("Deleting " << index);
						}
						else{
							CI_LOG_I("Deleting pointer");
						}
					}
					catch (Exception & e){
						CI_LOG_I("deleted: " << e.what());
					}
				}
			}
			else{
				CI_LOG_I("Out of range " << mBoxes.size());
			}
		}
		mDeleteIndex.clear();

		//clear boxes for world
		try{
			for (const auto & polys : mPolygons) {
				mWorld->DestroyBody(polys);
			}

			mPolygons.clear();
		}
		catch (Exception & e){
			CI_LOG_I("deleted: " << e.what());
		}

		//CI_LOG_I("FINISH CLEANING");
	}

	void ParticleManager::addContourTriangulation(ci::TriMesh & mesh)
	{
		//CI_LOG_I("START CONTOUR");
		if (mesh.getNumVertices() >= 3){
			const size_t polycount = mesh.getNumTriangles();
			//CI_LOG_I(mesh.getNumVertices() <<" " << mesh.getNumTriangles());
			for (size_t i = 0; i < polycount; ++i) {
				try{

					// Get a single triangle from the mesh.

					vec2 v0(0), v1(0), v2(0);
					mesh.getTriangleVertices(i, &v0, &v1, &v2);

					b2Vec2 vertices[3];
					vertices[0].Set(v0.x, v0.y);
					vertices[1].Set(v1.x, v1.y);
					vertices[2].Set(v2.x, v2.y);

					b2BodyDef bodyDef;
					b2PolygonShape poly;

					bodyDef.type = b2_staticBody;
					poly.Set(vertices, 3);

					//CI_LOG_I("MID CONTOUR");

					b2Body	* b2PolyBody = mWorld->CreateBody(&bodyDef);
					if (b2PolyBody != NULL){
						b2FixtureDef fixtureDef;
						fixtureDef.shape = &poly;
						fixtureDef.density = 0.0f;
						fixtureDef.friction = 0.00f;
						fixtureDef.restitution = 0.0f;

						b2PolyBody->CreateFixture(&fixtureDef);

						mPolygons.push_back(b2PolyBody);
					}
				}
				catch (Exception & e){
					CI_LOG_I("Polygon: " << e.what());
				}
			}

		}
	}

	void ParticleManager::addParticle(ci::vec2 & pos)
	{
		try{
			b2BodyDef bodyDef;
			bodyDef.type = b2_dynamicBody;
			bodyDef.position.Set(pos.x, pos.y);

			const  b2Body * body = mWorld->CreateBody(&bodyDef);
			if (body != NULL){
				ParticleBoxRef  particle = ParticleBox::create(mWorld->CreateBody(&bodyDef));

				particle->setInitPos(pos);
				float size = ci::randInt(6, 6);
				particle->setSize(size);


				b2PolygonShape dynamicBox;
				dynamicBox.SetAsBox(size, size);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &dynamicBox;
				fixtureDef.density = ci::randFloat(0.5, 0.9);
				fixtureDef.friction = ci::randFloat(0.06, 0.09);
				fixtureDef.restitution = ci::randFloat(0.23, 0.4); // bounce

				b2Body * body = const_cast<b2Body*>(particle->getB2Body());

				if (body != NULL){

					body->CreateFixture(&fixtureDef);

					mBoxes.push_back(particle);
				}
			}
			else{
				CI_LOG_I("cannot add: ");
			}
		}
		catch (Exception & e){
			CI_LOG_I("creating box: " << e.what());
		}
	}

}