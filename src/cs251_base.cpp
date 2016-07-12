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

#include "cs251_base.hpp"
#include <cstdio>
#include <string>
using namespace std;
using namespace cs251;

base_sim_t::base_sim_t()
{
	b2Vec2 gravity;
	//! Set gravity to zero
	gravity.Set(0.0f, 0.0f);
	m_world = new b2World(gravity);

	m_text_line = 30; 
	m_point_count = 0;
	score1=score2=0;
	fouls1 = fouls2 = 0;
	turn_no =  1;
	isFoul = false;
	wasFoul1 = wasFoul2 =  false;
	ismoving = false;
	s = '0';
	prev1 = prev2 =0;
	gameEnded = false;
	firstContact  = 0;
	ball[0] = ball[1]  =0;
	firstPocketed  = true;

	m_world->SetDebugDraw(&m_debug_draw);
	m_world->SetContactListener(this);
	m_step_count = 0;

	b2BodyDef body_def;
	m_ground_body = m_world->CreateBody(&body_def);

	memset(&m_max_profile, 0, sizeof(b2Profile));
	memset(&m_total_profile, 0, sizeof(b2Profile));
}

base_sim_t::~base_sim_t()
{
	// By deleting the world, we delete the bomb, mouse joint, etc.
	delete m_world;
	m_world = NULL;
}

void base_sim_t::pre_solve(b2Contact* contact, const b2Manifold* oldManifold)
{
	const b2Manifold* manifold = contact->GetManifold();
	
	if (manifold->pointCount == 0)
	{
		return;
	}
	
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();
	
	b2PointState state1[b2_maxManifoldPoints], state2[b2_maxManifoldPoints];
	b2GetPointStates(state1, state2, oldManifold, manifold);
	
	b2WorldManifold world_manifold;
	contact->GetWorldManifold(&world_manifold);
	
	for (int32 i = 0; i < manifold->pointCount && m_point_count < k_max_contact_points; ++i)
	{
		contact_point_t* cp = m_points + m_point_count;
		cp->fixtureA = fixtureA;
		cp->fixtureB = fixtureB;
		cp->position = world_manifold.points[i];
		cp->normal = world_manifold.normal;
		cp->state = state2[i];
		++m_point_count;
	}
}
/** checks if black ball was the last ball left on the table */ 
bool base_sim_t::isBlackLast()
{
	//! node represents the head pointer to the list of all bodies present in our box2Dworld
	b2Body* node = m_world->GetBodyList();
	int status,ballStatus;
	b2Body* b;
	if(turn_no == 1)
		ballStatus = 3;
	else
		ballStatus = 4;
	while(node)
	{
		b = node;
	    node = node->GetNext();

	    status = (long)b->GetUserData();
	    //! check if there is a ball other than the black ball which should have been pocketed by current player 
		if(status == ballStatus)
	    {
	    	return false;
	    }
	}
	return true;

}
void base_sim_t::draw_title(int x, int y, const char *string)
{
		//m_debug_draw.DrawString(x, y, string);
}


void base_sim_t::updateGame()
{
	//! node represents the head pointer to the list of all bodies present in our box2Dworld
	b2Body* node = m_world->GetBodyList();
	int n = 0;
	//! While the list is not empty, check its user data and store it into status.
	while (node)
	{

	    b2Body* b = node;
	    node = node->GetNext();
	    int status = (long)b->GetUserData();
	    /** If status equals 6 or 7, then a game ball was pocketed and update
	    *   the ball accordingly and remove the body from the box2D world.
	    */
	    if (status == 6 || status == 7)
	    {
	        if(status == ball[0])
	        {
	        	score1++;
	        }

	        else if(status== ball[1])
	        {
	        	score2++;
	        }

	        m_world->DestroyBody(b);
	    }
	    /** Else if status equals four, then the cue ball was pocketed which represents a foul.
	    *   Place the cue on its initial original position at rest.
	    */
	    else if(status == 9)
	    {
	    		n++;
	    		b2Vec2 p(15.0f,20.0f);
	    		b->SetUserData((void*)1);
	    		b->SetLinearVelocity(b2Vec2(0,0));
	    		b->SetAngularVelocity(0.0f);
	    		b->SetTransform(p,b->GetAngle());
	    }
	    //! Else if the status was 5, then the black ball went into the pocket and the game is finished  
	    else if(status == 8)
	    {
	    	gameEnded = true;
	    	m_world->DestroyBody(b);
	    	if(!isBlackLast())
	    		winner = turn_no^3;
	    	else
	    		winner = turn_no;
	    }
	}
	 if(n > 0){
	 	isFoul = true;
	 }
}
/** A function to update the turn of the player properly 
 *  @see updateGame
 *  @see isFoul
 * 	@see prev2
 *  @see prev1
 *  @see turn_no 
*/
void base_sim_t::update_turn()
{
	s = '0';
	if(ball[turn_no-1] !=0 && firstContact != ball[turn_no-1] - 3)
	{
		isFoul = true;
	}
	firstContact = 0;
	if(turn_no == 1)
	{
		if(prev2 != score2 || isFoul )
		{
			s += 1; 
			turn_no ^= 3; 
			wasFoul1 = true;
			wasFoul2 = false;
			fouls1++;
			prev2 = score2;
			prev1 = score1;
			isFoul = false;
		}
		else if( prev1 != score1 ){
			prev1 = score1;
			 
		}
		else if(prev1 == score1 && wasFoul2){
			wasFoul2 = false;
		}
		else
			turn_no ^=3;
	}
	else{

		if(prev1 != score1 ||isFoul){
			s += 2;
			turn_no  ^=3;
			fouls2++; 
			wasFoul2 = true;
			wasFoul1 = false;
			prev1 = score1;
			prev2 = score2;
			isFoul  = 0;
		}
		else if(prev2 != score2){
			prev2 = score2;
		}
		else if(prev2 == score2 && wasFoul1){
			wasFoul1 = false;
		}
		else
			turn_no ^=3;
	}
}

void base_sim_t::checkmoving()
{	
	//! node represents the head pointer to the list of all bodies present in our box2Dworld
	b2Body* node = m_world->GetBodyList();
	//! isMoving represents if all the bodies have stopped or not
	ismoving = false;
	while(node)
	{
		b2Body* b = node;
	    node = node->GetNext();

	    int status = (long)b->GetUserData();
	    //! check whether that linear velocity of every ball is zero or not
		if(status == 3 || status == 4 || status == 5 || status == 1 )
	    {
	    	b2Vec2 ballvel = b->GetLinearVelocity();
	    	// b2Vec2 zero(0,0);
	    	if(ballvel.x != 0 || ballvel.y != 0)
	    	{
	    		ismoving =true;
	    		break;
	    	}
	    }

	}
	//! if no object is moving, call update_turn()
	   if(ismoving == false){
	   	update_turn();
	   }

}

void base_sim_t::step(settings_t* settings)
{
	if(!gameEnded){
		float32 time_step = settings->hz > 0.0f ? 1.0f / settings->hz : float32(0.0f);
		

		//! Make the scoreboard for player 1 using DrawString function and score1 variable
		m_debug_draw.DrawString(12, 200 , "Player 1");
		m_debug_draw.DrawString(12,215,"Score=%d",score1);
		m_debug_draw.DrawString(12,230,"Fouls Done=%d",fouls1);


		//! Make the scoreboard for player 2 using DrawString function and score2 variable
		m_debug_draw.DrawString(535,200,"Player 2");
		m_debug_draw.DrawString(535,215,"Score=%d",score2);
		m_debug_draw.DrawString(535,230,"Fouls Done=%d",fouls2);
		
		if(s == '1' || s == '2')
			m_debug_draw.DrawString(240,55,"Foul Done by Player %c",s);
		else
			m_debug_draw.DrawString(240,55,"Foul Done By No One");

		//! Update the turn of the player according to the turn_no
		if(ismoving == false)
			m_debug_draw.DrawString(240,70,"Turn of Player No. %d",turn_no);
		else
			m_debug_draw.DrawString(240,70,"Turn of Player No. %d in progress",turn_no);		
		
		uint32 flags = 0;
		flags += settings->draw_shapes      * b2Draw::e_shapeBit;
		flags += settings->draw_joints      * b2Draw::e_jointBit;
		flags += settings->draw_AABBs     * b2Draw::e_aabbBit;
		flags += settings->draw_pairs     * b2Draw::e_pairBit;
		flags += settings->draw_COMs        * b2Draw::e_centerOfMassBit;
		m_debug_draw.SetFlags(flags);
		
		m_world->SetWarmStarting(settings->enable_warm_starting > 0);
		m_world->SetContinuousPhysics(settings->enable_continuous > 0);
		m_world->SetSubStepping(settings->enable_sub_stepping > 0);
		
		m_point_count = 0;
		
		m_world->Step(time_step, settings->velocity_iterations, settings->position_iterations);
		updateGame();

		if(ismoving)
		{
			checkmoving();
		}
		m_world->DrawDebugData(); 
		
		
		if (time_step > 0.0f)
		{
			++m_step_count;
		}
		if (settings->draw_stats)
	    {
	      int32 body_count = m_world->GetBodyCount();
	      int32 contact_count = m_world->GetContactCount();
	      int32 joint_count = m_world->GetJointCount();
	      m_debug_draw.DrawString(5, m_text_line, "bodies/contacts/joints = %d/%d/%d", body_count, contact_count, joint_count);
	      m_text_line += 15;
	      
	      int32 proxy_count = m_world->GetProxyCount();
	      int32 height = m_world->GetTreeHeight();
	      int32 balance = m_world->GetTreeBalance();
	      float32 quality = m_world->GetTreeQuality();
	      m_debug_draw.DrawString(5, m_text_line, "proxies/height/balance/quality = %d/%d/%d/%g", proxy_count, height, balance, quality);
	      m_text_line += 15;
	    }
	  
	  // Track maximum profile times
		{
		const b2Profile& p = m_world->GetProfile();
		m_max_profile.step = b2Max(m_max_profile.step, p.step);
		m_max_profile.collide = b2Max(m_max_profile.collide, p.collide);
		m_max_profile.solve = b2Max(m_max_profile.solve, p.solve);
		m_max_profile.solveInit = b2Max(m_max_profile.solveInit, p.solveInit);
		m_max_profile.solveVelocity = b2Max(m_max_profile.solveVelocity, p.solveVelocity);
		m_max_profile.solvePosition = b2Max(m_max_profile.solvePosition, p.solvePosition);
		m_max_profile.solveTOI = b2Max(m_max_profile.solveTOI, p.solveTOI);
		m_max_profile.broadphase = b2Max(m_max_profile.broadphase, p.broadphase);

		m_total_profile.step += p.step;
		m_total_profile.collide += p.collide;
		m_total_profile.solve += p.solve;
		m_total_profile.solveInit += p.solveInit;
		m_total_profile.solveVelocity += p.solveVelocity;
		m_total_profile.solvePosition += p.solvePosition;
		m_total_profile.solveTOI += p.solveTOI;
		m_total_profile.broadphase += p.broadphase;
		}
	  
	  	if (settings->draw_profile)
	    {
			const b2Profile& p = m_world->GetProfile();

			b2Profile ave_profile;
			memset(&ave_profile, 0, sizeof(b2Profile));
			if (m_step_count > 0)
			{
			  float32 scale = 1.0f / m_step_count;
			  ave_profile.step = scale * m_total_profile.step;
			  ave_profile.collide = scale * m_total_profile.collide;
			  ave_profile.solve = scale * m_total_profile.solve;
			  ave_profile.solveInit = scale * m_total_profile.solveInit;
			  ave_profile.solveVelocity = scale * m_total_profile.solveVelocity;
			  ave_profile.solvePosition = scale * m_total_profile.solvePosition;
			  ave_profile.solveTOI = scale * m_total_profile.solveTOI;
			  ave_profile.broadphase = scale * m_total_profile.broadphase;
			}
	      
			m_debug_draw.DrawString(5, m_text_line, "step [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.step, ave_profile.step, m_max_profile.step);
			m_text_line += 15;
			m_debug_draw.DrawString(5, m_text_line, "collide [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.collide, ave_profile.collide, m_max_profile.collide);
			m_text_line += 15;
			m_debug_draw.DrawString(5, m_text_line, "solve [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solve, ave_profile.solve, m_max_profile.solve);
			m_text_line += 15;
			m_debug_draw.DrawString(5, m_text_line, "solve init [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveInit, ave_profile.solveInit, m_max_profile.solveInit);
			m_text_line += 15;
			m_debug_draw.DrawString(5, m_text_line, "solve velocity [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveVelocity, ave_profile.solveVelocity, m_max_profile.solveVelocity);
			m_text_line += 15;
			m_debug_draw.DrawString(5, m_text_line, "solve position [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solvePosition, ave_profile.solvePosition, m_max_profile.solvePosition);
			m_text_line += 15;
			m_debug_draw.DrawString(5, m_text_line, "solveTOI [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveTOI, ave_profile.solveTOI, m_max_profile.solveTOI);
			m_text_line += 15;
			m_debug_draw.DrawString(5, m_text_line, "broad-phase [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.broadphase, ave_profile.broadphase, m_max_profile.broadphase);
			m_text_line += 15;
	    }
	    
	   if (settings->draw_contact_points)
	    {
			//const float32 k_impulseScale = 0.1f;
			const float32 k_axis_scale = 0.3f;

			for (int32 i = 0; i < m_point_count; ++i)
			{
				contact_point_t* point = m_points + i;

				if (point->state == b2_addState)
				{
				  // Add
				  m_debug_draw.DrawPoint(point->position, 10.0f, b2Color(0.3f, 0.95f, 0.3f));
				}
				else if (point->state == b2_persistState)
				{
				  // Persist
				  m_debug_draw.DrawPoint(point->position, 5.0f, b2Color(0.3f, 0.3f, 0.95f));
				}

				if (settings->draw_contact_normals == 1)
				{
				  b2Vec2 p1 = point->position;
				  b2Vec2 p2 = p1 + k_axis_scale * point->normal;
				  m_debug_draw.DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.9f));
				}
				else if (settings->draw_contact_forces == 1)
				{
				  //b2Vec2 p1 = point->position;
				  //b2Vec2 p2 = p1 + k_forceScale * point->normalForce * point->normal;
				  //DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.3f));
				}

				if (settings->draw_friction_forces == 1)
				{
				  //b2Vec2 tangent = b2Cross(point->normal, 1.0f);
				  //b2Vec2 p1 = point->position;
				  //b2Vec2 p2 = p1 + k_forceScale * point->tangentForce * tangent;
				  //DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.3f));
				}
			}
		}
	}
	else
	   m_debug_draw.DrawString(320,220,"Player %d Won",winner);
}
  /**
   * a void member taking one constant argument .
   * @param p a b2Vec2 variable denoting the current position of mouse pointer .
   * @see mouse_up(const b2Vec2& p)
   * @see mouse_move(const b2Vec2& p)
   * @see ismoving
   * @see helpBody
   * @see whiteball
   * @see stick
   */
void base_sim_t::mouse_down(const b2Vec2& p)
{
	//! If ismoving is false, then:
	if(!ismoving)
	{
		/** Get the current position of the ball and the mouse pointer and 
		 *  calculate the difference b/w them and store it into diff
		 */

		b2Vec2 ballpos = whiteball->GetPosition();
		b2Vec2 diff = p - ballpos;
		//! Calculate the angle of the diff vector 
		float32 angle = atan(diff.y/diff.x);

		/** Create a new position vector using original ball pos and normalized diff 
		 *  with distance of cue ball from cue stick in the range of 9.1 to 14.
		 */ 
		b2Vec2 position = ballpos;
		float32 length = diff.Length()-2.0f;
		if(length>6.0f)
			length = 6.0f;
		else if(length<1.1)
			length = 1.1;
		length += 8.0f;
		diff.Normalize();
		diff*=length;
		position += diff;
		stick->SetTransform(position,angle);

		/** helpEdge represents the ray which shows the projected direction
		 *  of motion of the ball */ 
		b2EdgeShape helpEdge;
		diff*= -5.0f;
		diff+=ballpos;
		helpEdge.Set(ballpos,diff);

		//! edgeFixture is a b2FixtureDef whose shape is assigned by helpEdge
		b2FixtureDef *edgeFixture = new b2FixtureDef;
		edgeFixture->isSensor =true;
		edgeFixture->shape = &helpEdge;

		/** Destroy the original fixture of helpbody and create a new fixture from edgeFixture
		 *  pointing in the correct direction
		 */
		if(helpBody->GetFixtureList()!=NULL)
			helpBody->DestroyFixture(helpBody->GetFixtureList());
		helpBody->CreateFixture(edgeFixture);
	}
}
  /**
   * a void member taking one constant argument denoting what happens 
   * when clicked mouse is left.
   * @param p a b2Vec2 variable denoting the current position of mouse cursor.
   * @see mouse_down(const b2Vec2& p)
   * @see mouse_move(const b2Vec2& p)
   * @see ismoving
   * @see helpBody
   * @see whiteball
   * @see stick
   */
void base_sim_t::mouse_up(const b2Vec2& p)
{

	if(!ismoving)
	{
		//! Hit the cue-ball using the cue stick with power proportional to the distance b/w them 
		b2Vec2 ballpos = whiteball->GetPosition();
		b2Vec2 diff = ballpos-p;
		float32 power = diff.Length()-2;
		// update power accordingly
		if(power>15.0f)
			power = 15.0f;
		else if(power<1.1)
			power = 1.1;
		power -= 1;

		// Normalize the diff i.e set its length to 1 and multiply it by a factor of 8 
		diff.Normalize();
		diff*=8.0f;
		//stick->SetTransform(ballpos-diff,stick->GetAngle());

		//! Reset the position of the stick and place it outside the table 
		stick->SetTransform(b2Vec2(0.0f,2.0f),0);

		/** Update the linear velocity of the cue ball properly proportionate to power */
		whiteball->SetLinearVelocity(7.0f*power*diff);
		/** Delete the ray from the ball showing its projected direction */
		if(helpBody->GetFixtureList()!=NULL)
			helpBody->DestroyFixture(helpBody->GetFixtureList());
		//! Update ismoving to true 
		ismoving = true;
	}
}
/**
 * a void member taking one constant argument denoting what happens 
 * when mouse is kept pressed and the cursor is moved .
 * @param p a b2Vec2 variable denoting the current position of mouse cursor.
 * @see mouse_down(const b2Vec2& p)
 * @see mouse_up(const b2Vec2& p)
 * @see ismoving
 * @see helpBody
 * @see whiteball
 * @see stick
 */
void base_sim_t::mouse_move(const b2Vec2& p)
{   

	//! Dynamically update the position of the cue stick as the mouse cursor is moved
	if(!ismoving)
	{
		b2Vec2 ballpos = whiteball->GetPosition();
		b2Vec2 diff = p - ballpos;
		float32 angle = atan(diff.y/diff.x);
		b2Vec2 position = ballpos;
		float32 length = diff.Length()-2.0f;
		if(length>6.0f)
			length = 6.0f;
		else if(length<1.1)
			length = 1.1;
		length += 8.0f;
		diff.Normalize();
		diff*=length;
		position += diff;
		stick->SetTransform(position,angle);

		/** helpEdge represents the ray which shows the projected direction
		 *  of motion of the ball */ 
		b2EdgeShape helpEdge;
		diff.Normalize();
		diff*= -60.0f;
		diff+=ballpos;
		helpEdge.Set(ballpos,diff);

		//! edgeFixture is a b2FixtureDef whose shape is assigned by helpEdge
		b2FixtureDef *edgeFixture = new b2FixtureDef;
		edgeFixture->isSensor =true;
		edgeFixture->shape = &helpEdge;
		
		/** Destroy the original fixture of helpbody and create a new fixture from edgeFixture
		 *  pointing in the correct direction
		 */
		if(helpBody->GetFixtureList()!=NULL)
			helpBody->DestroyFixture(helpBody->GetFixtureList());
		helpBody->CreateFixture(edgeFixture) ;
	}
}
