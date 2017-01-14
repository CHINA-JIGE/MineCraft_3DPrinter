
/***********************************************************************

										class : Mesh Slicer

************************************************************************/
#include "Printer.h"

using namespace Math;

IMeshSlicer::IMeshSlicer()
{
	m_pPrimitiveVertexBuffer = new std::vector<VECTOR3>;
	m_pTriangleNormalBuffer = new std::vector<VECTOR3>	;
	m_pLineSegmentBuffer = new std::vector<N_LineSegment>;
	m_pLineStripBuffer = new std::vector<N_LineStrip>;
	m_pBoundingBox_Min = new VECTOR3(0,0,0);
	m_pBoundingBox_Max = new VECTOR3(0, 0, 0);
	m_pLayerList = new std::vector<N_Layer>;
}

BOOL IMeshSlicer::Step1_LoadPrimitiveMeshFromSTLFile(std::string pFilePath)
{
	// this function is used to load STL file , and Primitive Vertex Data has 3x more elements
	//than Triangle Normal Buffer , this is because 1 triangle consists of 3 vertices.

	std::vector<UINT>	tmpIndexBuffer;
	std::string					tmpHeaderString;
	BOOL isLoadSTLsucceeded = FALSE;

	//we must check if the STL has been loaded successfully
	isLoadSTLsucceeded =IFileLoader::ImportFile_STL(
		pFilePath, 
		*m_pPrimitiveVertexBuffer,
		tmpIndexBuffer,
		*m_pTriangleNormalBuffer, 
		tmpHeaderString);

	//assert
	if (!isLoadSTLsucceeded)
	{
		LOG_MSG("NoiseUtSliceGenerator: Load STL file failed!!");
		return FALSE;
	}

	//do it right after loading the file
	mFunction_ComputeBoundingBox();

	return TRUE;
}

void IMeshSlicer::Step2_Intersection(UINT iLayerCount)
{

	//Objective of this function is GENERALLY  to generate Line segments ,sometimes directly add triangles

	if (iLayerCount < 2)
	{
		LOG_MSG("MeshSlicer : Layer Count is too little !!");
	}

	//top/bottom y of the bounding box (AABB)
	float Y_min = m_pBoundingBox_Min->y;
	float Y_max = m_pBoundingBox_Max->y;


	//calculate  delta Y between layers , note that the TOP and BOTTOM are taken into consideration
	//thus ,  minus 1
	float  layerDeltaY = (Y_max - Y_min) / (float)(iLayerCount-1);

	//since in one intersection request , different thickness settings might be used for a Y region
	//so we must accumulate LayerCount of ALL intersection mission;
	UINT totalLayerCount = iLayerCount;

	//...
	UINT		currentLayerID = 0;
	UINT		currentTriangleID = 0;

	//the Y coord of current layer
	float		currentLayerY = 0;

	//how many vertex of a single triangle are on this layer ,valued 0,1,2,3.
	//if vertex count ==0 , we should see if the layer can proceed further intersection with edges
	//which is stored in the "N_IntersectionResult"
	N_IntersectionResult	tmpResult;

	//Total Triangle  Count (one Normal Vector correspond to one Triangle
	UINT		totalTriangleCount = m_pTriangleNormalBuffer->size();

	//........tmp var to store 3 vertex of triangle
	VECTOR3 v1 =VECTOR3(0,0,0);		VECTOR3 v2 = VECTOR3(0,0,0);	VECTOR3 v3 = VECTOR3(0,0,0);

	//.....
	N_LineSegment tmpLineSegment;

	//...tmp var  : used in  "switch (tmpResult.mVertexCount)"
	std::vector<VECTOR3> tmpIntersectPointList;
	BOOL canIntersect = FALSE;
	VECTOR3 tmpPoint(0, 0, 0);



	//start traverse all layers / triangles , and intersect
	for (currentLayerID = 0; currentLayerID < iLayerCount;currentLayerID++)
	{
		currentLayerY = Y_min + layerDeltaY * ((float)currentLayerID);


		//calculate how many vertex of this triangle are on this layer
		for (currentTriangleID = 0;currentTriangleID <totalTriangleCount;  currentTriangleID++)
		{
			 v1 = m_pPrimitiveVertexBuffer->at(currentTriangleID * 3 + 0);
			 v2 = m_pPrimitiveVertexBuffer->at(currentTriangleID * 3 + 1);
			 v3 = m_pPrimitiveVertexBuffer->at(currentTriangleID * 3 + 2);

			// tmpResult:"N_IntersectionResult" 
			tmpResult = mFunction_HowManyVertexOnThisLayer(currentLayerY,v1,v2,v3);

			//Category Discussion	(?��������?)
			switch (tmpResult.mVertexCount)
			{
			//-------------------------------
			case 0:
#pragma region VertexOnLayer : 0

				if (tmpResult.isPossibleToIntersectEdges)
				{
					//maybe some edges will intersect current Layer
					BOOL canIntersect = FALSE;
					VECTOR3 tmpPoint(0, 0, 0);

					//proceed a line --- layer intersection ,and return a bool
					//if they really intersect so we add the intersect point to a list
					canIntersect = mFunction_Intersect_LineSeg_Layer(v1, v2, currentLayerY, &tmpPoint);
					if (canIntersect){ tmpIntersectPointList.push_back(tmpPoint);}

					canIntersect = mFunction_Intersect_LineSeg_Layer(v1, v3, currentLayerY, &tmpPoint);
					if (canIntersect) { tmpIntersectPointList.push_back(tmpPoint); }

					canIntersect = mFunction_Intersect_LineSeg_Layer(v2, v3, currentLayerY, &tmpPoint);
					if (canIntersect) { tmpIntersectPointList.push_back(tmpPoint); }

					//theoretically , intersectPoint will only got 2 members , but maybe shit happens ??
					if (tmpIntersectPointList.size() == 2)
					{
						N_LineSegment tmpLineSegment;
						tmpLineSegment.v1 = tmpIntersectPointList.at(0);
						tmpLineSegment.v2 = tmpIntersectPointList.at(1);
						tmpLineSegment.LayerID = currentLayerID;
						tmpLineSegment.Dirty = FALSE;
						//triangle normal projection , look for tech doc for more detail
						m_pLineSegmentBuffer->push_back(tmpLineSegment);//add to disordered line segments buffer
					}


					// clear the tmpIntersectPoint Buffer
					tmpIntersectPointList.clear();
				}

#pragma endregion 
				break;

			//-------------------------------
			case 1:
#pragma region VertexOnLayer : 1

				//if one point is on the layer ,then the line segment composed of other 2 points -
				//will try to intersect the layer
				switch (tmpResult.mIndexList->at(0))
				{
					//see which point is on this layer
				case 0:
					tmpIntersectPointList.push_back(v1);
					canIntersect = mFunction_Intersect_LineSeg_Layer(v2, v3, currentLayerY, &tmpPoint);
					if (canIntersect) { tmpIntersectPointList.push_back(tmpPoint); }
					break;
					//see which point is on this layer

				case 1:
					tmpIntersectPointList.push_back(v2);
					canIntersect = mFunction_Intersect_LineSeg_Layer(v1, v3, currentLayerY, &tmpPoint);
					if (canIntersect) { tmpIntersectPointList.push_back(tmpPoint); }
					break;
					//see which point is on this layer

				case 2:
					tmpIntersectPointList.push_back(v3);
					canIntersect = mFunction_Intersect_LineSeg_Layer(v1, v2, currentLayerY, &tmpPoint);
					if (canIntersect) { tmpIntersectPointList.push_back(tmpPoint); }
					break;
				}

				//2 intersect point will make up a line segment
				if (tmpIntersectPointList.size() == 2)
				{
					N_LineSegment tmpLineSegment;
					tmpLineSegment.v1 = tmpIntersectPointList.at(0);
					tmpLineSegment.v2 = tmpIntersectPointList.at(1);
					tmpLineSegment.LayerID = currentLayerID;
					tmpLineSegment.Dirty = FALSE;
					//triangle normal projection , look for tech doc for more detail
					m_pLineSegmentBuffer->push_back(tmpLineSegment);//add to disordered line segments buffer
				}


				// clear the tmpIntersectPoint Buffer
				tmpIntersectPointList.clear();


#pragma endregion 
				break;

			//-------------------------------
			case 2:
#pragma region VertexOnLayer : 2
				//v1,v2 are on this layer ,so just directly add to line segment buffer
				//....the first vertex
				switch (tmpResult.mIndexList->at(0))
				{
				case 0:
					tmpIntersectPointList.push_back(v1);
					break;
				case 1:
					tmpIntersectPointList.push_back(v2);
					break;
				case 2:
					tmpIntersectPointList.push_back(v3);
					break;
				}

				//...the second vertex (use  index to determine which vertex should we add)
				switch (tmpResult.mIndexList->at(1))
				{
				case 0:
					tmpIntersectPointList.push_back(v1);
					break;
				case 1:
					tmpIntersectPointList.push_back(v2);
					break;
				case 2:
					tmpIntersectPointList.push_back(v3);
					break;
				}

				//theoretically , intersectPoint will only got 2 members , but maybe shit happens ??
				if (tmpIntersectPointList.size() == 2)
				{
					N_LineSegment tmpLineSegment;
					tmpLineSegment.v1 = tmpIntersectPointList.at(0);
					tmpLineSegment.v2 = tmpIntersectPointList.at(1);
					tmpLineSegment.LayerID = currentLayerID;
					tmpLineSegment.Dirty = FALSE;
					//triangle normal projection , look for tech doc for more detail
					m_pLineSegmentBuffer->push_back(tmpLineSegment);//add to disordered line segments buffer
				}

				// clear the tmpIntersectPoint Buffer
				//tmpIntersectPointList.erase(tmpIntersectPointList.begin(), tmpIntersectPointList.end());
				tmpIntersectPointList.clear();

#pragma endregion 
				break;

			//-------------------------------
			default:
				break;
			}
		}

	}

	//preparation for next step
	m_pLayerList->resize(totalLayerCount);
}

void	IMeshSlicer::Step3_GenerateLineStrip()
{

	//preprocess (generate layer tile information , optimization for linking line segment)
	mFunction_GenerateLayerTileInformation();

	//link numerous line segments into line strip
	//(and at the present  ignore the problem of  'multiple branches'

	N_LineStrip			tmpLineStrip;
	N_LineSegment		tmpLineSegment;
	VECTOR3			tmpLineStripTailPoint;

	//............
	UINT i = 0, j = 0;

	BOOL canFindNextPoint = FALSE;

	for (i = 0;i < m_pLineSegmentBuffer->size(); i++)
	{

		tmpLineSegment = m_pLineSegmentBuffer->at(i);
		//find the first line segment valid to be the head of line strip
		if (tmpLineSegment.Dirty == FALSE)
		{
				//we have found a "clean" line segment , then add 2 vertices to the current line strip
				//this is a new line strip spawned
				//v2 is the tail of the strip
				tmpLineStrip.LayerID = tmpLineSegment.LayerID;
				tmpLineStrip.pointList.push_back(tmpLineSegment.v1);
				tmpLineStrip.pointList.push_back(tmpLineSegment.v2);
				tmpLineStripTailPoint = tmpLineSegment.v2;
		}
		else
		{
			goto nextLineSegment;
		}

		//make the new  line strip grow longer until no more line segment can be added to the tail
		canFindNextPoint = mFunction_LineStrip_FindNextPoint(&tmpLineStripTailPoint, tmpLineStrip.LayerID, &tmpLineStrip);
		while (canFindNextPoint)
		{
			canFindNextPoint = mFunction_LineStrip_FindNextPoint(&tmpLineStripTailPoint, tmpLineStrip.LayerID, &tmpLineStrip);
		}

		//we have finished growing a line strip, so add it to line Strip Buffer;
		m_pLineStripBuffer->push_back(tmpLineStrip);

		//clear the tmp buffer so that we can continue processing new line strip
		tmpLineStrip.pointList.clear();

	nextLineSegment:;
	}//for i
}

UINT IMeshSlicer::GetLineSegmentCount()
{
	return m_pLineSegmentBuffer->size();
}

void IMeshSlicer::GetLineSegmentBuffer(std::vector<N_LineSegment>& outBuffer)
{
	outBuffer.assign(m_pLineSegmentBuffer->begin(), m_pLineSegmentBuffer->end());
}


void IMeshSlicer::GetBoundingBox(BOUNDINGBOX & outAABB)
{
	outAABB = BOUNDINGBOX(*m_pBoundingBox_Min, *m_pBoundingBox_Max);
}



/************************************************************************
											P R I V A T E
************************************************************************/
void		IMeshSlicer::mFunction_ComputeBoundingBox()
{
	//compute Bounding box : override 1

	UINT i = 0;
	VECTOR3 tmpV;
	//traverse all vertices, and get the biggest and smallest point in terms of x,y,z components
	//�������ж��㣬�����Χ��3�������� С/�� ����������
	for (i = 0;i < m_pPrimitiveVertexBuffer->size();i++)
	{
		tmpV = m_pPrimitiveVertexBuffer->at(i);
		if (tmpV.x <(m_pBoundingBox_Min->x)) { m_pBoundingBox_Min->x = tmpV.x; }
		if (tmpV.y <(m_pBoundingBox_Min->y)) { m_pBoundingBox_Min->y = tmpV.y; }
		if (tmpV.z <(m_pBoundingBox_Min->z)) { m_pBoundingBox_Min->z = tmpV.z; }

		if (tmpV.x >(m_pBoundingBox_Max->x)) { m_pBoundingBox_Max->x = tmpV.x; }
		if (tmpV.y >(m_pBoundingBox_Max->y)) { m_pBoundingBox_Max->y = tmpV.y; }
		if (tmpV.z >(m_pBoundingBox_Max->z)) { m_pBoundingBox_Max->z = tmpV.z; }
	}

}

BOOL	IMeshSlicer::mFunction_Intersect_LineSeg_Layer(VECTOR3 v1, VECTOR3 v2, float layerY, VECTOR3 * outIntersectPoint)
{

	//some obvious wrong input check
	if (!outIntersectPoint){return FALSE;}
	if (v1 == v2) { return FALSE; }

	//init the out result
	ZeroMemory(outIntersectPoint, sizeof(*outIntersectPoint));


	//...detect if it is possible to intersect , according to "Zero Point Existance Theorem(?)"
	if ((v1.y > layerY && v2.y > layerY) || (v1.y < layerY && v2.y < layerY))
	{
		//impossible to intersect ,so just return
		return FALSE;
	}

	//an interpolating ratio between these 2 points ,valued [-1,1]
	//vector start point is v2
	float lambda = (layerY - v2.y) / (v1.y - v2.y);

	//actually not an essential check,but of no harm = =
	if (lambda > -1.0f || lambda < 1.0f)
	{
		//interpolate
		*outIntersectPoint = v2 + lambda * (v1 - v2);

		//to avoid floating point number error
		outIntersectPoint->y = layerY;

		//...
		return TRUE;
	}

	return FALSE;
}

void		IMeshSlicer::mFunction_GenerateLayerTileInformation()
{
	//preprocess , generate line segment Info for each layer tile for optimization
	// if 1 or 2 of the line segment vertex is(are) right on the tile, then line seg ID will be recorded in a std::vector which belong to this tile

	for (UINT i = 0;i < m_pLineSegmentBuffer->size();i++)
	{
		UINT currentLayerID = m_pLineSegmentBuffer->at(i).LayerID;
		N_LineSegment currentLineSeg = m_pLineSegmentBuffer->at(i);
		N_LineSegmentVertex tmpLineSegmentVertex;
		UINT tileID_X = 0, tileID_Z = 0;

#pragma region Line Segment Vertex 1
		//generate layer tile info for vertex 1 of current line segment

		//get 2 array index integers from point
		mFunction_GetLayerTileIDFromPoint(currentLineSeg.v1, tileID_X, tileID_Z);

		//add vertex/line segment info to corresponding layer tile
		tmpLineSegmentVertex.lineSegmentID = i;
		tmpLineSegmentVertex.vertexID = 1;
		//note that this is a 2D array !!!!!! dont let [] become overloaded operation of VECTOR
		//m_pLayerList->at(currentLayerID).layerTile	.at(tileID_X).at(tileID_Z).push_back(tmpLineSegmentVertex);
		m_pLayerList->at(currentLayerID).layerTile[tileID_X][tileID_Z].push_back(tmpLineSegmentVertex);


#pragma endregion Line Segment Vertex 1

#pragma region Line Segment Vertex 2
		//generate layer tile info for vertex 2 of current line segment

		//get 2 array index integers from point
		mFunction_GetLayerTileIDFromPoint(currentLineSeg.v2, tileID_X, tileID_Z);

		//add vertex/line segment info to corresponding layer tile
		tmpLineSegmentVertex.lineSegmentID = i;
		tmpLineSegmentVertex.vertexID = 2;
		m_pLayerList->at(currentLayerID).layerTile[tileID_X][tileID_Z].push_back(tmpLineSegmentVertex);
	
#pragma endregion Line Segment Vertex 2
}

}

inline void		IMeshSlicer::mFunction_GetLayerTileIDFromPoint(VECTOR3 v, UINT & tileID_X, UINT & tileID_Z)
{
	//this function will return indices(2 integers) of a layer TILE according to coordinate of a point

	//bounding box of mesh has been computed in slicer- step1

	//axis-aligned bounding box (for the time being, we use bounding box to generate bounding sqare
	float MIN_X = m_pBoundingBox_Min->x;
	float MAX_X = m_pBoundingBox_Max->x;
	float MIN_Z = m_pBoundingBox_Min->z;
	float MAX_Z = m_pBoundingBox_Max->z;

	//map x to [0,1]first , then map to integer, and apply floot() operation
	 tileID_X = (UINT)floorf((v.x - MIN_X) / (MAX_X - MIN_X)*CONST_LayerTileStepCount);
	 tileID_Z = (UINT)floorf((v.z - MIN_Z) / (MAX_Z - MIN_Z)*CONST_LayerTileStepCount);

	 //...deal with boundary problem (the most right vertex on the edge)
	 if (tileID_X == CONST_LayerTileStepCount)tileID_X--;
	 if (tileID_Z == CONST_LayerTileStepCount)tileID_Z--;
};

N_IntersectionResult	IMeshSlicer::mFunction_HowManyVertexOnThisLayer( float currentlayerY, VECTOR3& v1, VECTOR3& v2, VECTOR3& v3)

{
	N_IntersectionResult outResult;

	//if all the vertex are beyond / below the layer
	BOOL b1 = (v1.y > currentlayerY) && (v2.y > currentlayerY) && (v3.y > currentlayerY);
	BOOL b2 = (v1.y < currentlayerY) && (v2.y < currentlayerY) && (v3.y < currentlayerY);
	if (b1 || b2 )
	{
		outResult.mVertexCount = 0;
		outResult.isPossibleToIntersectEdges = FALSE;
		return outResult;
	}

	//if none of the vertex is on this layer
	if ((v1.y != currentlayerY) && (v2.y != currentlayerY) && (v3.y != currentlayerY) )
	{
		outResult.mVertexCount = 0;
		outResult.isPossibleToIntersectEdges = TRUE;
		return outResult;
	}


	//note that error exist in float number , 0.001 is a threshold
	const float FLOAT_EQUAL_THRESHOLD = 0.001f;

	//count how many Vertices are on this layer
	if(abs(v1.y - currentlayerY) < FLOAT_EQUAL_THRESHOLD)
	{
		outResult.mVertexCount += 1;
		outResult.mIndexList->push_back(0);
	}

	if (abs(v2.y - currentlayerY) < FLOAT_EQUAL_THRESHOLD)
	{
		outResult.mVertexCount += 1;
		outResult.mIndexList->push_back(1);
	}

	if (abs(v3.y - currentlayerY) < FLOAT_EQUAL_THRESHOLD)
	{
		outResult.mVertexCount += 1;
		outResult.mIndexList->push_back(2);
	}

	return outResult;
}

BOOL IMeshSlicer::mFunction_LineStrip_FindNextPoint(VECTOR3*  tailPoint, UINT currentLayerID, N_LineStrip* currLineStrip)
{
	//........used to judge if two point can be weld together
	float						tmpPointDist = 0;
	const float			SAME_POINT_DIST_THRESHOLD = 0.001f;
	VECTOR3			tmpV;
	N_LineSegment		tmpLineSegment;

	UINT tileID_X = 0, tileID_Z = 0;

	//locate layer tile where the tail point lies in ( where it can find the next vertex to weld)
	mFunction_GetLayerTileIDFromPoint(*tailPoint, tileID_X, tileID_Z);

	//get layer tile
	auto currLayerTile = m_pLayerList->at(currentLayerID).layerTile[tileID_X][tileID_Z];

	for (UINT j = 0; j < currLayerTile.size();j++)
	{
		//read info from the tile , retrive and traverse vertices in this tile
		UINT lineSegID = currLayerTile.at(j).lineSegmentID;
		tmpLineSegment = m_pLineSegmentBuffer->at(lineSegID);

		//if this line segment has not been checked &&
		//the line segment is on the same layer as the stretching line strip
		if (tmpLineSegment.Dirty == FALSE )
		{

			//if we can weld v1 to line strip tail 
			tmpV = *tailPoint - tmpLineSegment.v1;
			if (tmpV.Length() < SAME_POINT_DIST_THRESHOLD)
			{
				currLineStrip->pointList.push_back(tmpLineSegment.v2);
				*tailPoint = tmpLineSegment.v2;
				//this line segment has been checked , so light up the DIRTY mark.
				m_pLineSegmentBuffer->at(lineSegID).Dirty = TRUE;
				return TRUE;
			}
			

			//if we can weld v2 to line strip tail 
			tmpV = *tailPoint - tmpLineSegment.v2;
			if (tmpV.Length() < SAME_POINT_DIST_THRESHOLD)
			{
				currLineStrip->pointList.push_back(tmpLineSegment.v1);
				*tailPoint = tmpLineSegment.v1;
				//this line segment has been checked , so light up the DIRTY mark.
				m_pLineSegmentBuffer->at(lineSegID).Dirty = TRUE;
				return TRUE;
			}
		}

	}//for j

	//didn't find qualified line segment to link
	return FALSE;
}

VECTOR3 IMeshSlicer::mFunction_Compute_Normal2D(VECTOR3 triangleNormal)
{
	//and you want to know why the projection is the normal , refer to the tech doc
	VECTOR3 outNormal(triangleNormal.x,0, triangleNormal.z);
	outNormal.Normalize();
	return outNormal;
}
