/*!***************************************************************************************
\file			NavMesh.h
\project
\author         Davin Tan

\par			Course: GAM300
\date           28/09/2023

\brief
    This file contains the declarations of the following:
    1. NavMesh class
		a. Linking of triangles by initializing the neighbours
		b. Getter functions
		c. Run time update of the navmesh

All content © 2023 DigiPen Institute of Technology Singapore. All rights reserved.
******************************************************************************************/

#pragma once

#include <vector>

#include "Geometry.h"

struct NavMeshAgent;

class NavMesh
{
public:
	NavMesh(std::vector<Triangle3D> mTri) : mTriangles(mTri) {};
	~NavMesh() {};

	// Links triangles to their neighbours
	void LinkAllTriangles();

	// Getter functions
	// Returns the triangles of the navmesh
	const std::vector<Triangle3D> GetNavMeshTriangles() const;

	// Pathfinding stuff
	bool FindPath(NavMeshAgent& mAgent, const glm::vec3& mEnd);

	void ResetTriangles();

private:
	// Add neighbour to this triangle
	void LinkTriangles(Triangle3D* mTri1, Triangle3D* mTri2);

	// Get the triangle containing the point
	Triangle3D* TriangleContainingPoint(const glm::vec3& mPoint);

	std::vector<Triangle3D> mTriangles;
};