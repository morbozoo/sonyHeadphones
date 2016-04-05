#include "ParticlesBox2D.h"

namespace physics{


	using namespace ci;
	using namespace ci::app;
	using namespace std;

	ParticleBox::ParticleBox(const b2Body* b) :
		b2b(b), mDead(false), mType(0)
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

	void ParticleManager::setup(ci::vec2 gravityWind)
	{
		mParticleCounter = 0;
		//setup the world
		b2Vec2 gravity(gravityWind.x, gravityWind.y);
		mWorld = new b2World(gravity);

		b2BodyDef groundBodyDef;
		groundBodyDef.position.Set(0, 0);// ci::app::getWindowWidth() / 2.0f, ci::app::getWindowHeight() / 2.0f); //, ci::app::getWindowHeight());//
		b2Body* groundBody = mWorld->CreateBody(&groundBodyDef);

		// Define the ground box shape.
		//b2PolygonShape groundBox;

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

		//circle
		gl::GlslProgRef solidShader = gl::getStockShader(gl::ShaderDef().color());
		mCircleBatch = ci::gl::Batch::create(geom::Circle().radius(1).subdivisions(32), solidShader);

		mCiruferenceBatch = ci::gl::Batch::create(geom::WireCircle().radius(1).subdivisions(32), solidShader);

		//Square
		mSquareBatch = ci::gl::Batch::create(geom::Rect(), solidShader);

		mTriangleBatch = ci::gl::Batch::create(geom::Circle().radius(1).subdivisions(2), solidShader);

	}

	void ParticleManager::draw(ci::ColorA col)
	{
		//iterate through the boxes

		switch (mDrawMode){
	
		//only circle
		case 0:
		{
			int boxIndex = 0;
			for (auto & particleBox : mBoxes){
				const b2Body * b2b = particleBox->getB2Body();

				particleBox->update();
				float vel = (abs(b2b->GetLinearVelocity().x) + abs(b2b->GetLinearVelocity().y));

				float size = particleBox->getSize();
				float sizeM = size / 2.0;
				ci::gl::ScopedMatrices mat;
				ci::gl::ScopedModelMatrix model;
				ci::gl::translate(b2b->GetPosition().x + sizeM, b2b->GetPosition().y + sizeM);
				ci::gl::rotate(b2b->GetAngle());
				ci::gl::scale(ci::vec2(size * 2));

				//ci::gl::ScopedColor color(ci::ColorA(col.r + vel, col.g + vel * 2, col.b + vel * 2, 0.8));
				ci::gl::ScopedColor color(col);

				mSquareBatch->draw();

				if (particleBox->isDead()){
					mDeleteIndex.insert(std::pair<int, int>(particleBox->getId(), boxIndex));
				}
				boxIndex++;
			}
		}
		break;

		//only square
		case 1:
		{
			int boxIndex = 0;
			for (auto & particleBox : mBoxes){
				const b2Body * b2b = particleBox->getB2Body();

				particleBox->update();
				float vel = (abs(b2b->GetLinearVelocity().x) + abs(b2b->GetLinearVelocity().y));

				float size = particleBox->getSize();
				float sizeM = size / 2.0;
				ci::gl::ScopedMatrices mat;
				ci::gl::ScopedModelMatrix model;
				ci::gl::translate(b2b->GetPosition().x + sizeM, b2b->GetPosition().y + sizeM);
				ci::gl::rotate(b2b->GetAngle());
				ci::gl::scale(ci::vec2(size * 2));

				//ci::gl::ScopedColor color(ci::ColorA(col.r + vel, col.g + vel * 2, col.b + vel * 2, 0.8));

				//Circle
				{
					ci::gl::ScopedColor color(col);

					mCircleBatch->draw();
				}

				//wire circle
				{
					float white = 0.7 + vel;
					ci::gl::ScopedColor color(ci::ColorA(white, white, white, 1.0));
					mCiruferenceBatch->draw();
				}

				if (particleBox->isDead()){
					mDeleteIndex.insert(std::pair<int, int>(particleBox->getId(), boxIndex));
				}
				boxIndex++;
			}
		}
		break;

		//random figure
		case 2:
		{
			int boxIndex = 0;
			for (auto & particleBox : mBoxes){
				const b2Body * b2b = particleBox->getB2Body();

				particleBox->update();
				float vel = (abs(b2b->GetLinearVelocity().x) + abs(b2b->GetLinearVelocity().y)) / 3.0f;

				float size = particleBox->getSize();
				float sizeM = size / 2.0;
				ci::gl::ScopedMatrices mat;
				ci::gl::ScopedModelMatrix model;
				ci::gl::translate(b2b->GetPosition().x + sizeM, b2b->GetPosition().y + sizeM);
				ci::gl::rotate(b2b->GetAngle());
				ci::gl::scale(ci::vec2(size * 1.8));

				ci::gl::ScopedColor color(col);


				if (particleBox->getType() == 0){
					mCircleBatch->draw();
				}
				else if (particleBox->getType() == 1){
					mSquareBatch->draw();
				}
				else{
					mTriangleBatch->draw();
				}


				if (particleBox->isDead()){
					mDeleteIndex.insert(std::pair<int, int>(particleBox->getId(), boxIndex));
				}
				boxIndex++;
			}
		}
		break;

		}
	}

	//update particles
	void ParticleManager::update()
	{
		for (int i = 0; i < 5; ++i){
			mWorld->Step(1 / 30.0f, 10, 10);
		}
	}

	//clean particles
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

	//create a box2d mesh using as input a Trimesh format
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
				float size = ci::randInt(10, 24);
				particle->setSize(size);

				particle->setId(mParticleCounter);

				//set type of the particle, circle, rect or triange
				float rand = ci::randFloat(1);
				if (rand < 0.3){
					particle->setType(0);
				}
				else if (rand > 0.3 && rand < 0.6){
					particle->setType(1);
				}
				else{
					particle->setType(2);
				}

				b2PolygonShape dynamicBox;
				dynamicBox.SetAsBox(size, size);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &dynamicBox;

				//Change particle dynamics
				fixtureDef.density = ci::randFloat(0.4, 0.9);
				fixtureDef.friction = ci::randFloat(0.07, 0.09);
				fixtureDef.restitution = ci::randFloat(0.2, 0.5); // bounce

				b2Body * body = const_cast<b2Body*>(particle->getB2Body());

				if (body != NULL){
					body->CreateFixture(&fixtureDef);
					mBoxes.push_back(particle);

					mParticleCounter++;
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

	void ParticleManager::deleteParticle()
	{
		mBoxes.erase(mBoxes.begin());
	}

}