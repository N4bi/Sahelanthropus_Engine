#include "PhysBody3D.h"
#include "Bullet\include\btBulletDynamicsCommon.h"



// ---------------------------------------------------------
PhysBody3D::PhysBody3D(btRigidBody* body) : body(body)
{}

// ---------------------------------------------------------
PhysBody3D::~PhysBody3D()
{}

// ---------------------------------------------------------
void PhysBody3D::GetTransform(float* matrix) const
{
	if(body != NULL && matrix != NULL)
	{
		body->getWorldTransform().getOpenGLMatrix(matrix);
	}
}

// ---------------------------------------------------------
void PhysBody3D::SetTransform(float* matrix) const
{
	if(body != NULL && matrix != NULL)
	{
		btTransform t;
		t.setFromOpenGLMatrix(matrix);
		body->setWorldTransform(t);
	}
}

// ---------------------------------------------------------
void PhysBody3D::SetPos(float x, float y, float z)
{
	btTransform t = body->getWorldTransform();
	t.setOrigin(btVector3(x, y, z));
	body->setWorldTransform(t);
}

vec PhysBody3D::GetPos()
{
	vec ret;
	ret.x = body->getWorldTransform().getOrigin().getX();
	ret.y = body->getWorldTransform().getOrigin().getY();
	ret.z = body->getWorldTransform().getOrigin().getZ();

	return ret;
}