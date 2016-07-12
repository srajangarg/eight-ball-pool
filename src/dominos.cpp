/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

/* 
* Base code for CS 251 Software Systems Lab 
* Department of Computer Science and Engineering, IIT Bombay
* 
*/

/** \file */
#include "cs251_base.hpp"
#include "render.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include "GL/freeglut.h"
#endif

#include <cstring>
using namespace std;

#include "dominos.hpp"

/** \namespace cs251 */

namespace cs251
{
 
	b2Body* whiteball;
	b2Body* stick;
	b2Body* helpBody;


	/**  The is the constructor.This is the documentation block for the constructor */
	dominos_t::dominos_t()
	{
		
		//For setting user data for different type of circular objects : the pockets, game balls and the cue ball. 
		intptr_t w = 1;
		intptr_t x = 2;
		intptr_t y = 3;
	

		
		/// ## Pool Table Pockets
		//! pocket is a variable of type b2Body* and is created from the bodyDef and is attached to the fixtureDef
		b2Body* pocket;
		/** circle is a variable of type b2CircleShape with radius 1.5 unit and
		*  isSensor parmeter as false to allow overlap with other objects 
		*/
		b2CircleShape circle;
		for (int i = 0; i < 6; ++i)
		{	
			//! bodyDef is a static body with position set appropriately corressponding to 6 corners of pool table
			b2BodyDef bodyDef;
			if(i < 3)
			{
			   bodyDef.position.Set(-30.0f + i*(30), 35.0f);
			}
			else
			{
			   bodyDef.position.Set(-30.0f + (i-3)*(30), 5.0f);
			}
			/** fixtureDef is a variable of type b2FixtureDef and its shape is created from variable circle and
			*   the isSensor parameter is set to true so that it can overlap with other objects
			*/
			b2FixtureDef fixtureDef;
			pocket = m_world->CreateBody(&bodyDef);
			pocket->SetUserData((void*)x);
			fixtureDef.isSensor = true;
			circle.m_radius = (float32)(1.5f);
			fixtureDef.shape = &circle;
			pocket->CreateFixture(&fixtureDef);
		}

		/// ## Game Balls
		{
			//! Balls are created using a variable spherebody which is of type b2Body*
			//! spherebody is created using the fixture defition of ballbd and body definition of ballbd.
			b2Body* spherebody;
			float32 r = 1.0001f; 
			//! circle is a variable of type b2CircleShape with radius equal to 1 units (approximately)
			b2CircleShape circle;
			
			float32 x_pos = -15.0f;
			float32 y_pos = 20.0f;

			/** ballfd is a variable of type b2FixtureDef with parameters density = 1 kg/m^2 ,
			*   friction coefficient as 0.2 and restitution coefficient as 0.8
			*/
			b2FixtureDef ballfd;
			ballfd.density = 1.0f;
			ballfd.friction = 0.2f;
			ballfd.restitution = 0.8f;
			/** ballbd is a b2BodyDef variable which defines a dynamic body with both coefficients
			*   of linearDamping and angularDamping  set to 0.6. This is done to make a real world pool game!
			*/
			b2BodyDef ballbd;
			ballbd.linearDamping = 0.6f;
			ballbd.angularDamping = 0.6f;
			ballbd.type = b2_dynamicBody;

			//! The for loop creates the ball with appropriate orientation as in a original pool game 
			for (int i = 0; i < 5; ++i)
			{
				for(int j=0;j<=i;j++)
				{
					//! There are slight deviations in the radius so as to color the balls appropriately.
					//! There are seven balls of color blue, 7 of color light orange and one of color black.  
					if(j%2){
						r = 1.0001f;
						y = 3;
					}
					else{
						r = 1.0000f;
						y = 4;
					}

					/** Setting middle ball of the middle row to be black and others to be orange 
					*by seting their radius and data values appropriately. 
					*/
					if(i==2)
					{
						if(j!=1){
							r = 1.0001f;
							y = 3;
						}
						else{
							r = 1.0002f;
							y = 5;
						}
					}
					

					circle.m_radius = r;
					ballfd.shape = &circle;
	
					// setting the coordinates of a ball
					ballbd.position.Set(x_pos - i*1.74*r, y_pos + j*2*r - i*r);
					// creating a body in the box2d World
					spherebody = m_world->CreateBody(&ballbd);
					//! The user data of the ball is set to a intptr with value 3 or 4 or 5 according to its color
					spherebody->SetUserData((void*)y);
					spherebody->CreateFixture(&ballfd);

				}
			}
		}
		/// ## Cue ball

		/**Cue ball is created  using a variable whiteball of type b2BodyDef* whose shape is defined by
		 whcircle and it is attached to the fixture whiteballfd */

		float32 r = 1.00001f;
		//! whcircle is a variable of type b2CircleShape whose radius is approximately 1 unit and position is set to (15,20).
		b2CircleShape whcircle;
		whcircle.m_radius = r;
		float32 x_pos = -15.0f;
		float32 y_pos = 20.0f;

		/** whiteballfd is a b2FixtureDef with shape asigned by whcircle and parameters density = 1 kg/m^2 and
			coefficients of friction = 0.2 and restitution = 0.8  respectively*/
		b2FixtureDef whiteballfd;
		whiteballfd.shape = &whcircle;
		whiteballfd.density = 1.0f;
		whiteballfd.friction = 0.2f;
		whiteballfd.restitution = 0.8f;

		/** whiteballbd is the definiton of a dyanmic body and is of type b2BodyDef 
		*   with coefficients of linearDamping and angularDamping both set to 0.6 similar to the game balls
		*/
		b2BodyDef whiteballbd;
		whiteballbd.linearDamping = 0.6f;
		whiteballbd.angularDamping = 0.6f;
		
		whiteballbd.type = b2_dynamicBody;
		whiteballbd.position.Set(x_pos +30, y_pos);
		whiteball = m_world->CreateBody(&whiteballbd);
		//! The user data of cue ball is set to a intptr with value 3.
		whiteball->SetUserData((void*)w);
		whiteball->CreateFixture(&whiteballfd);

		/// ## Cue Stick 
		{
		//!Cue stick is created using a variable the body definiton of bd and has length 16 units and thickness 0.4 units. 
		//! shape assigns the length and width of the cue stick.
		b2PolygonShape shape;
		shape.SetAsBox(8.0f, 0.2f);

		//! bd is a variable of type b2BodyDef whose position is set at (14,14)  and defiens a dynamic body 
		b2BodyDef bd;
		bd.position.Set(14.0f, 14.0f);
		bd.type = b2_dynamicBody;
		stick = m_world->CreateBody(&bd);
		//! fd is a fixtureDef with parameters density = 1 kg/m^2 and isSensor Set as true. 
		b2FixtureDef *fd = new b2FixtureDef;
		fd->density = 1.0f;
		fd->isSensor = true;
		fd->shape = new b2PolygonShape;
		fd->shape = &shape;
		stick->CreateFixture(fd);


		//! Projected ray is created using the body helpBody and setting it appropriately
		b2BodyDef helpBodyDef;
		helpBodyDef.position.Set(0,0);
		helpBody = m_world->CreateBody(&helpBodyDef);

		}

		/// ## Pool table 
		{
		/** Defined a pool table using a rectangular box which can overlap with any other object 
		*and 4 edges for collision detection with the wall.
		*/
		b2BodyDef tableBodyDef,tableEdgeDef;
		tableEdgeDef.position.Set(0,0);
		//! tableBodyDef is a variable of type b2BodyDef and its center is set to (0,20)
		tableBodyDef.position.Set(0.0f,20.0f);

		b2Body* tableBody = m_world->CreateBody(&tableBodyDef);
		b2Body* table = m_world->CreateBody(&tableEdgeDef);
		


		//! shape is a axis aligned box with width 60 and height 30.
		b2PolygonShape shape;
		
		shape.SetAsBox(30.0f, 15.0f);
		//! fd is a fixtureDefinition of type b2FixtureDef and isSensor is set to true and its parameter shape is defined by shape above.
		b2FixtureDef *fd = new b2FixtureDef;
		fd->isSensor = true;
		fd->friction = 1.0f;
		fd->shape = new b2PolygonShape;
		fd->shape = &shape;
		// tableBody is attached to the fixture fd
		tableBody->CreateFixture(fd);

		//! 4 table edges are defined using the tableBox which is a variable of type b2EdgeShape by resetting its position.
		b2EdgeShape tableBox;
		// top edge
		tableBox.Set(b2Vec2(-30.0f, 35.0f),b2Vec2(30.0f, 35.0f));
		table->CreateFixture(&tableBox,0);

		// bottom edge
		tableBox.Set(b2Vec2(-30.0f, 5.0f),b2Vec2(30.0f, 5.0f));
		table->CreateFixture(&tableBox,0);

		//left side edge 
		tableBox.Set(b2Vec2(-30.0f, 35.0f),b2Vec2(-30.0f, 5.0f));
		table->CreateFixture(&tableBox,0);

		// right side edge
		tableBox.Set(b2Vec2(30.0f, 35.0f),b2Vec2(30.0f, 5.0f));
		table->CreateFixture(&tableBox,0);

		}

	}

	sim_t *sim = new sim_t("Dominos", dominos_t::create);  /*!< a new sim class using create method */

}
