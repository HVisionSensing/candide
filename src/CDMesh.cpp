//
//  CDMesh.cpp
//  candide
//
//  Created by damian on 03/11/13.
//  Copyright (c) 2013 Damian Stewart. All rights reserved.
//

#include "CDMesh.h"
#include "CDUtilities.h"

#include <Fl/gl.h>

using namespace glm;
using namespace std;


void CDMesh::addFace( int v0, int v1, int v2 )
{
	assert(v0<vertices.size() && "v0 out of bounds");
	assert(v1<vertices.size() && "v1 out of bounds");
	assert(v2<vertices.size() && "v2 out of bounds");
	
	triangles.push_back(Triangle(v0,v1,v2));
	
}


void CDMesh::setupArrays() const
{
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer(3, GL_FLOAT, 0, &vertices[0][0] );
	
	bool doNormals = (vertexNormals.size()==vertices.size());
	if ( doNormals ) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnable(GL_NORMALIZE);
		glNormalPointer(GL_FLOAT, 0, &vertexNormals[0][0] );
		glEnable( GL_LIGHTING );
	} else {
		glDisable( GL_LIGHTING );
	}
	
	if ( textureCoordinates.size() == getNumVertices() ) {
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer( 2, GL_FLOAT, 0, &textureCoordinates[0][0] );
	}
}


void CDMesh::draw() const
{
	setupArrays();

	glDrawElements( GL_TRIANGLES, (GLsizei)triangles.size()*3, GL_UNSIGNED_SHORT, &triangles[0].v[0]);
	
	teardownArrays();
	
}

void CDMesh::teardownArrays() const
{
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
	bool doNormals = (vertexNormals.size()==vertices.size());
	if ( doNormals ) {
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable( GL_LIGHTING );
	}
}

void CDMesh::drawBoundingBox() const
{
	vec3 vertices[8];
	
	vec3 centre = getBoundingBoxCenter();
	vec3 bounds = getBoundingBoxSize();
	vec3 halfBounds = bounds*0.5f;
	for ( int i=0;i<2;i++ )
		for ( int j=0;j<2;j++ )
			for ( int k=0;k<2;k++ )
				vertices[i*4+j*2+k] = vec3(centre.x+halfBounds.x*(i==0?-1:1),centre.y+halfBounds.y*(j==0?-1:1),centre.z+halfBounds.z*(k==0?-1:1));
	
	short lines[12*2] = { 0,1, 0,4, 4,5, 5,1, 2,3, 2,6, 6,7, 3,7, 0,2, 1,3, 4,6, 5,7 };
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawElements( GL_LINES, 12*2, GL_UNSIGNED_SHORT, &lines[0] );
	
	glDisableClientState(GL_VERTEX_ARRAY);
	
}


void CDMesh::getBoundingBox( vec3& minCorner, vec3& maxCorner ) const
{
	for ( int i=0; i<vertices.size(); i++ ) {
		const vec3& v = vertices[i];
		if ( i==0 ) {
			minCorner = v;
			maxCorner = v;
		} else {
			minCorner.x = std::min(v.x,minCorner.x);
			minCorner.y = std::min(v.y,minCorner.y);
			minCorner.z = std::min(v.z,minCorner.z);
			maxCorner.x = std::max(v.x,maxCorner.x);
			maxCorner.y = std::max(v.y,maxCorner.y);
			maxCorner.z = std::max(v.z,maxCorner.z);
		}
	}
}

vec3 CDMesh::getBoundingBoxCenter() const
{
	vec3 minCorner, maxCorner;
	getBoundingBox(minCorner,maxCorner);
	return (minCorner+maxCorner)*0.5f;
}

vec3 CDMesh::getBoundingBoxSize() const
{
	vec3 minCorner, maxCorner;
	getBoundingBox(minCorner,maxCorner);
	return maxCorner-minCorner;
}

void CDMesh::updateNormals()
{
	vector<pair<vec3,int> > accumulatedVertexNormals;
	accumulatedVertexNormals.resize(vertices.size());
	
	// accumulate vertex normals
	for ( auto t: triangles ) {
		vec3 p0 = vertices[t.v[0]];
		vec3 p1 = vertices[t.v[1]];
		vec3 p2 = vertices[t.v[2]];
		
		vec3 normal = normalize(cross(p1-p0, p1-p2));

		for ( int i=0; i<3; i++ ) {
			auto& vn = accumulatedVertexNormals.at(t.v[i]);
			vn.first += normal;
			vn.second++;
		}
	}
	
	// apply
	vertexNormals.resize(vertices.size());
	for ( int i=0; i<accumulatedVertexNormals.size(); i++ )
	{
		if ( accumulatedVertexNormals[i].second>0 ) {
			vertexNormals.at(i) = normalize(accumulatedVertexNormals[i].first/(float)accumulatedVertexNormals[i].second);
		} else {
			vertexNormals.at(i) = vec3(0,0,1);
		}
	}
	
	
	
}


CDMesh CDMeshOperation::transform( const CDMesh& inputMesh, const glm::mat4 transform )
{
	CDMesh targetMesh = inputMesh;
	
	// transform vertices
	for ( size_t i=0; i<targetMesh.getNumVertices(); i++ ) {
		vec4 v = vec4(targetMesh.getVertex(i),1.0f);
		v = transform*v;
		targetMesh.setVertex(i, vec3(v.x,v.y,v.z) );
	}
	// recalculate normals
	if ( targetMesh.getNumNormals() ) {
		targetMesh.updateNormals();
	}
	
	return targetMesh;
}
