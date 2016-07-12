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


#ifndef _CS251BASE_HPP_
#define _CS251BASE_HPP_
#include <iostream>
#include "render.hpp"
#include <Box2D/Box2D.h>
#include <cstdlib>
#include <string>
 #include <map>
#define RAND_LIMIT 32767

using namespace std;
namespace cs251
{

	//! What is the difference between a class and a struct in C++?
	class base_sim_t;
	struct settings_t;
	
	//! Why do we use a typedef
	typedef base_sim_t* sim_create_fcn(); 

	//! Simulation settings. Some can be controlled in the GUI.
	struct settings_t
	{
		//! Notice the initialization of the class members in the constructor
		//! How is this happening?
		settings_t() :
			view_center(0.0f, 20.0f),
			hz(60.0f),
			velocity_iterations(8),
			position_iterations(3),
			draw_shapes(1),
			draw_joints(1),
			draw_AABBs(0),
			draw_pairs(0),
			draw_contact_points(0),
			draw_contact_normals(0),
			draw_contact_forces(0),
			draw_friction_forces(0),
			draw_COMs(0),
			draw_stats(0),
			draw_profile(0),
			enable_warm_starting(1),
			enable_continuous(1),
			enable_sub_stepping(0),
			pause(0),
			single_step(0)
		{}
		
		b2Vec2 view_center;
		float32 hz;
		int32 velocity_iterations;
		int32 position_iterations;
		int32 draw_shapes;
		int32 draw_joints;
		int32 draw_AABBs;
		int32 draw_pairs;
		int32 draw_contact_points;
		int32 draw_contact_normals;
		int32 draw_contact_forces;
		int32 draw_friction_forces;
		int32 draw_COMs;
		int32 draw_stats;
		int32 draw_profile;
		int32 enable_warm_starting;
		int32 enable_continuous;
		int32 enable_sub_stepping;
		int32 pause;
		int32 single_step;
	};
	
	struct sim_t
	{
		const char *name;
		sim_create_fcn *create_fcn;

		sim_t(const char *_name, sim_create_fcn *_create_fcn): 
			name(_name), create_fcn(_create_fcn) {;}
	};
	
	extern sim_t *sim;
	extern b2Body *whiteball;
	extern b2Body *stick;
	extern b2Body *helpBody;
	
	const int32 k_max_contact_points = 2048; 

	struct contact_point_t
	{
		b2Fixture* fixtureA;
		b2Fixture* fixtureB;
		b2Vec2 normal;
		b2Vec2 position;
		b2PointState state;
	};

	class QueryCallback : public b2QueryCallback
	{
	public:
		QueryCallback(const b2Vec2& point)
		{
			m_point = point;
			m_fixture = NULL;
		}

		bool ReportFixture(b2Fixture* fixture)
		{
			b2Body* body = fixture->GetBody();
			if (body->GetType() == b2_dynamicBody)
			{
				bool inside = fixture->TestPoint(m_point);
				if (inside)
				{
					m_fixture = fixture;

					// We are done, terminate the query.
					return false;
				}
			}

			// Continue the query.
			return true;
		}

		b2Vec2 m_point;
		b2Fixture* m_fixture;
	};

	//! class derived from b2ContactListener class
	class base_sim_t : public b2ContactListener
	{
		//! Represents the player (1 or 2) with current turn to play.
		int turn_no;
		//! Integer variable storing score of player 1
		int score1;
		//! Integer variable storing score of player 2
		int score2;
		//! Integer variable storing the number of fouls done by player 1
		int ball[2];
		int fouls1;
		//! Integer variable storing the number of fouls done by player 2
		int fouls2;
		//! Integer variable storing previous score of player 1 i.e score before his/her current turn
		int prev1;
		//! Integer varible storing previous score of player 2 i.e score before his/her current turn
		int prev2;
		//! Boolean variable indicating whether the current player did a foul or not
		bool isFoul;
		//! Boolean variable indicating whether was foul done by player 1 in last turn
		bool wasFoul1;
		//! Boolean variable indicating whether was foul done by player 1 in last turn
		bool wasFoul2;
		//! A boolean represent whether if all the balls on table have stopped moving or not.
		bool ismoving;
		//! A boolean variable whether the black ball has been pocketed or not
		bool gameEnded;
		//! A variable indicating the winner after the game has ended
		int winner;
		//! Character storing the number of the player who did the foul, '0' if no one did the foul 
		char s;
		int firstContact;
		bool firstPocketed;
		
	public:
		
		base_sim_t();
		b2MouseJoint* currJoint;
		b2Vec2 currMousePos;
		//! Virtual destructors - amazing objects. Why are these necessary?
		virtual ~base_sim_t();
		
		void set_text_line(int32 line) { m_text_line = line; }
		void draw_title(int x, int y, const char *string);
		
		virtual void step(settings_t* settings);

		virtual void keyboard(unsigned char key) { B2_NOT_USED(key); }
		virtual void keyboard_up(unsigned char key) { B2_NOT_USED(key); }

		void shift_mouse_down(const b2Vec2& p) { B2_NOT_USED(p); }
		void mouse_down(const b2Vec2& p);
		void mouse_up(const b2Vec2& p);
		void mouse_move(const b2Vec2& p);


		/* a member to update the game state by removing the pocketed balls and updating the balls
		* @see BeginContact(b2Contact* contact) 
		*/
		void updateGame();
		
		// Let derived tests know that a joint was destroyed.
		virtual void joint_destroyed(b2Joint* joint) { B2_NOT_USED(joint); }
		
		// Callbacks for derived classes.

		/** Called when two fixtures begin to touch.Since 
		 *  there is a member function with same name in base class b2ContactListener, it's 
		 *  declared as virtual to call it instead of the memeber function of derived from base class
		 */
		virtual void BeginContact(b2Contact* contact)
		{

			//! x1,x2 are the pointers to the two bodies in contact
			b2Body * x1 = contact->GetFixtureA()->GetBody();
			b2Body * x2 = contact->GetFixtureB()->GetBody();

			//! b1, b2 are integer varibales getting the user data of the two bodies 
			int b1 = (long) x1 ->GetUserData(); 
			int b2 = (long) x2 ->GetUserData(); 

			if(firstContact == 0 && ((b1 == 1 && b2 >=3) || (b1 >= 3 && b2 ==1)))
			{
				firstContact = (b1>b2)?b1:b2;
			}

			/** if the values obtained are 2,3 or 2,4 or 2,5 then a game ball i.e. either a
			  *  black or blue or orange ball came in contact in pocket
			  */ 
			if((b1 == 2 && b2 >= 3) || (b1 >= 3 && b2 == 2))
			{
				// x is the data of the game ball
				int x  = (b1>b2)?b1:b2;

				/** Update the user data of the ball to a void pointer with
				 *  value 7 if it was 4 or with 6 if it was 3 respectively
				 */
				if(firstPocketed && x<=4)
				{
					ball[turn_no - 1] = x+3;
					ball[2-turn_no] = (x+3)^1;		// Since 1 = 6^7  	
					firstPocketed = false;
				}
				if(b1 == 2)
				{
					if(x == 3)
						x2->SetUserData((void*)6);
					else if(x==4)
						x2->SetUserData((void*)7);
					else if(x==5)
						x2->SetUserData((void*)8);
				}
				else
				{
					if(x == 3)
						x1->SetUserData((void*)6);
					else if(x==4)
						x1->SetUserData((void*)7);
					else if(x==5)
						x1->SetUserData((void*)8);
				}
			}
			//! Else if a pocket and cue ball were in contact, then update the user data of the cue ball
			else if((b1 == 2 && b2 == 1) || (b1==1 && b2 ==2))
			{
				if(b1 == 2)
					x2->SetUserData((void*)9);
				else
					x1->SetUserData((void*)9);
			}
		}

		/** Updates the variable turn_no appropriately
		 * @see turn_no
		 */
		void update_turn();

		/** Checks whether all the balls on the pool table ha stoped moving or not 
		  * @see update_turn()
		  * @see BeginContact(b2Contact* contact)
		  * @see ismoving
		  */
		void checkmoving();

		bool isBlackLast();
		virtual void end_contact(b2Contact* contact) { B2_NOT_USED(contact); }
		
		virtual void pre_solve(b2Contact* contact, const b2Manifold* oldManifold);
		virtual void post_solve(const b2Contact* contact, const b2ContactImpulse* impulse)
		{
			B2_NOT_USED(contact);
			B2_NOT_USED(impulse);
		}

	//!How are protected members different from private memebers of a class in C++ ?
	protected:

		/** contact_listener can access the private members and functions of the class base_sim_t
		 * @see base_sim_t
		 */
		friend class contact_listener_t;
		
		b2Body* m_ground_body;
		b2AABB m_world_AABB;
		contact_point_t m_points[k_max_contact_points];
		int32 m_point_count;

		debug_draw_t m_debug_draw;
		int32 m_text_line;
		b2World* m_world;

		int32 m_step_count;
		
		b2Profile m_max_profile;
		b2Profile m_total_profile;
	};
}

#endif
