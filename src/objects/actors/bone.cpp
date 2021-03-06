/*--------------------------------------------//
Bone definition
This is used in conjunction with joints to 
create a skeleton for actors
//--------------------------------------------*/
#ifndef BONE
#define BONE

	/*--------------------------------------------//
	Includes
	//--------------------------------------------*/
		#include "bone.h"

//Bone
	/*--------------------------------------------//
	Default Constructor
	//--------------------------------------------*/
		bone::bone(){
			children = NULL;
			childCnt = 0;
		}

	/*--------------------------------------------//
	Add a joint as a child
	//--------------------------------------------*/
		void bone::addJoint(vec3 origin){
			joint** newChildren = (joint**) realloc(children, sizeof(joint*)*(childCnt+1));

			if (newChildren!=NULL) {
				children = newChildren;

				children[childCnt] = new joint(this, origin);
				childCnt++;
			}else{
				puts ("Error (re)allocating memory");
				exit (1);
			}
		};

	/*--------------------------------------------//
	Remove a joint as a child
	//--------------------------------------------*/
		void bone::remJoint(joint* &b){
			for (int i = 0; i < childCnt; i++){
				if (b == children[i]){
					//move last object to here
					children[i] = children[childCnt-1];
					//trim last object
					joint** newChildren = (joint**) realloc(children, sizeof(joint*)*(childCnt-1));
					//check if memory was allocated
					if (newChildren!=NULL){
						delete b;
						children = newChildren;
						childCnt--;
					}else{
						puts ("Error (re)allocating memory");
						exit (1);
					}
					return;
				}
			}
		};

	/*--------------------------------------------//
	Update function
	//--------------------------------------------*/
		void bone::updateAcc(){
			for (int i = 0; i < childCnt; i++){
				children[i]->updateAcc();
			}
		}
		void bone::updateVel(){
			for (int i = 0; i < childCnt; i++){
				children[i]->updateVel();
			}
		}
		void bone::updatePos(){
			for (int i = 0; i < childCnt; i++){
				children[i]->updatePos();
			}
		}

//Bone Parent
	/*--------------------------------------------//
	Default Constructor
	//--------------------------------------------*/
		boneParent::boneParent():bone(){
			parent = NULL;
		}

	/*--------------------------------------------//
	Default Destructor
	//--------------------------------------------*/
		boneParent::~boneParent(){
			for (int i = 0; i < childCnt; i++){
				delete children[i];
			}
		}

	/*--------------------------------------------//
	Get World Position
	A parent bone has the same position as its skeleton
	takes a bool for parity
	this decides whether to recalculate the wPos
	//--------------------------------------------*/
		vec3 boneParent::getPos(bool parity){
			if (wPosParity != parity)
				wPos = parent->getPosition();//note we dont pass the parity flag because the called function is trivial
			return wPos;
		}

	/*--------------------------------------------//
	Get Bone Direction
	takes a bool for parity
	this decides whether to recalculate the wPos
	//--------------------------------------------*/
		vec3 boneParent::getDirection(bool parity){
			if (dirParity != parity)
				dir = vec3(0,0,0);//root bone has no direction
			return dir;
		}

	/*--------------------------------------------//
	Get Bone Angle
	this angle is local to the bone it is attached to
	//--------------------------------------------*/
		angles boneParent::getAng(){
			return parent->getAngles();
		}

	/*--------------------------------------------//
	Set Bone Angle
	this angle is local to the bone it is attached to
	//--------------------------------------------*/
		void boneParent::setAng(angles a){
			parent->setAngles(a);
		}

//Bone Child
	/*--------------------------------------------//
	Default Constructor
	//--------------------------------------------*/
		boneChild::boneChild():bone(){
			parent = NULL;
			offset = vec3();
			ang = angles();
			angVel = angles();
			angAcc = angles();
			angFrc = angles();
		}

	/*--------------------------------------------//
	Default Destructor
	//--------------------------------------------*/
		boneChild::~boneChild(){
			for (int i = 0; i < childCnt; i++){
				delete children[i];
			}
		}

	/*--------------------------------------------//
	Get World Position
	A child bone has the same position as the joint
	it connects to
	takes a bool for parity
	this decides whether to recalculate the wPos
	//--------------------------------------------*/
		vec3 boneChild::getPos(bool parity){
			if (wPosParity != parity){
				//we need to recursively find where we attach to
				if (parent != NULL){
					//get the position we attach to parent at
					vec3 pos1 = parent->getPos(parity);

					//create quaternions
					quat p = quat(0, offset.x, offset.y, offset.z);
					quat r = quat(ang);//rotation quaternion
					quat t = quat(-r.x, -r.y, -r.z, r.w);//basically r'

					//now we have enough to get a new quaternion q which will be p rotated using hamilton product
					quat q = t.Hamilton(r.Hamilton(p));

					//finally we can extract a position from q and add our attaching world position
					wPos = vec3(q.x, q.y, q.z) + pos1;
					wPosParity = parity;
				}else{
					//no parents, our position is all that matters
					wPos = offset;
				}
			}
			return wPos;
		}

	/*--------------------------------------------//
	Get Bone Direction
	takes a bool for parity
	this decides whether to recalculate the wPos
	//--------------------------------------------*/
		vec3 boneChild::getDirection(bool parity){
			if (dirParity != parity){
				//where do we attach
				vec3 pos1 = parent->getPos(parity);
				//where does the thing we attach to attach
				vec3 pos2 = parent->parent->getPos(parity);
				//what is the directional vec3 if we were at angles 0,0,0
				vec3 forward = pos1 - pos2;

				//create quaternions
				quat p = quat(0, forward.x, forward.y, forward.z);
				quat r = quat(ang);//rotation quaternion
				quat t = quat(-r.x, -r.y, -r.z, r.w);//basically r'

				//now we have enough to get a new quaternion q which will be p rotated using hamilton product
				quat q = t.Hamilton(r.Hamilton(p));

				//finally we can extract a vec3 from q and normalize it into a directional vec3
				dir = vec3(q.x, q.y, q.z);
				dir.normalize();
				dirParity = parity;
			}
			return dir;
		}

	/*--------------------------------------------//
	Get Bone Angle
	this angle is local to the bone it is attached to
	//--------------------------------------------*/
		angles boneChild::getAng(){
			return ang;
		}

	/*--------------------------------------------//
	Set Bone Angle
	this angle is local to the bone it is attached to
	//--------------------------------------------*/
		void boneChild::setAng(angles a){
			ang = a;
		}

	/*--------------------------------------------//
	Update function
	//--------------------------------------------*/
		void boneChild::updateAcc(){
			angAcc = angFrc;
			for (int i = 0; i < childCnt; i++){
				children[i]->updateAcc();
			}
		}
		void boneChild::updateVel(){
			angVel += angAcc;
			for (int i = 0; i < childCnt; i++){
				children[i]->updateAcc();
			}
		}
		void boneChild::updatePos(){
			ang += angVel;
			for (int i = 0; i < childCnt; i++){
				children[i]->updateAcc();
			}
		}
#endif