/*!***************************************************************************************
\file			Geometry.cpp
\project
\author         Davin Tan

\par			Course: GAM300
\date           28/09/2023

\brief
    This file contains the definitions of the following:
    1. Triangle3D class
        a. A 3D triangle class used for navigation mesh
        b. Getter functions
        c. Addition of neighbouring triangles
        d. Point check in triangle

All content � 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#include "Precompiled.h"

#include "Geometry.h"

Triangle3D::Triangle3D(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
    mPoints[0] = p1;
    mPoints[1] = p2;
    mPoints[2] = p3;
    mMidPoint = (p1 + p2 + p3) / 3.f;
    mNormal = glm::cross((p2 - p1), (p3 - p1));
}

Triangle3D::~Triangle3D()
{

}

std::vector<Triangle3D*> Triangle3D::GetNeighbours()
{
    return mNeighbours;
}

const glm::vec3 Triangle3D::GetMidPoint() const
{
    return mMidPoint;
}
const glm::vec3 Triangle3D::GetNormal() const
{
    return mNormal;
}

bool Triangle3D::ContainsPoint(const glm::vec3& mPoint) const
{
    glm::vec3 A = this->mPoints[0];
    glm::vec3 B = this->mPoints[1];
    glm::vec3 C = this->mPoints[2];

    glm::vec3 p_Normal = glm::cross((B - A), (C - A));

    if (glm::dot((mPoint - A), glm::cross(p_Normal, (B - A))) < 0.f)
    {
        return false;
    }
    if (glm::dot((mPoint - B), glm::cross(p_Normal, (C - B))) < 0.f)
    {
        return false;
    }
    if (glm::dot((mPoint - C), glm::cross(p_Normal, (A - C))) < 0.f)
    {
        return false;
    }
    return true;
}

void Triangle3D::AddNeighbour(Triangle3D* mTri)
{
    mNeighbours.push_back(mTri);
}

const int Triangle3D::GetID()
{
    return mTriID;
}

void Triangle3D::SetTriID(int id)
{
    mTriID = id;
}