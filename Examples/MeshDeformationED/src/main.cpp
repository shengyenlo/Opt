﻿#include "main.h"
#include "ImageWarping.h"
#include "OpenMesh.h"
#include "LandMarkSet.h"

int main(int argc, const char * argv[])
{
	std::string filename = "../MeshDeformationARAP/raptor_clean.stl";
	if (argc >= 2) {
		filename = argv[1];
	}

	// Load Constraints
	LandMarkSet markersMesh;
	markersMesh.loadFromFile("../MeshDeformationARAP/raptor.mrk");

	std::vector<int>				constraintsIdx;
	std::vector<std::vector<float>> constraintsTarget;

	for (unsigned int i = 0; i < markersMesh.size(); i++)
	{
		constraintsIdx.push_back(markersMesh[i].getVertexIndex());
		constraintsTarget.push_back(markersMesh[i].getPosition());
	}

	SimpleMesh* mesh = new SimpleMesh();
	if (!OpenMesh::IO::read_mesh(*mesh, filename))
	{
	        std::cerr << "Error -> File: " << __FILE__ << " Line: " << __LINE__ << " Function: " << __FUNCTION__ << std::endl;
		std::cout << "bunny.off" << std::endl;
		exit(1);
	}

	ImageWarping warping(mesh, constraintsIdx, constraintsTarget);
	SimpleMesh* res = warping.solve();

	if (!OpenMesh::IO::write_mesh(*res, "out.off"))
	{
	        std::cerr << "Error -> File: " << __FILE__ << " Line: " << __LINE__ << " Function: " << __FUNCTION__ << std::endl;
		std::cout << "out.off" << std::endl;
		exit(1);
	}
#ifdef _WIN32
	getchar();
#endif
	return 0;
}
