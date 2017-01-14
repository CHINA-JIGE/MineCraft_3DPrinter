
/*********************************************************

			Mesh Slicer which computes line strip
			intersected by XZ planes and input model

********************************************************/

#pragma once


		const UINT CONST_LayerTileStepCount = 20;

		struct N_LineStrip
		{
			N_LineStrip() { ZeroMemory(this, sizeof(*this)); }//pointList = new std::vector<Math::VECTOR3>; }

			std::vector<Math::VECTOR3>	pointList;
			UINT		LayerID;
		};

		struct N_LineSegment
		{
			N_LineSegment() { ZeroMemory(this, sizeof(*this)); }
			Math::VECTOR3 v1;
			Math::VECTOR3 v2;
			UINT	LayerID;
			BOOL	Dirty;//check if this line segment has been reviewed,this flag can be reused
		};

		struct N_IntersectionResult
		{
			N_IntersectionResult() { ZeroMemory(this, sizeof(*this));mIndexList = new std::vector<UINT>; }

			UINT mVertexCount;
			BOOL isPossibleToIntersectEdges;//this bool will be used when (mVertexCount ==0)
			std::vector<UINT>* mIndexList;//which vertex (of a triangle) is on the layer
		};

		struct N_LineSegmentVertex
		{
			UINT lineSegmentID;
			UINT vertexID;//v1==1 or v2==2
		};

		//in an constrained XZ area with limited width and depth, this area will be
		//divided into many "tiles", which are smaller rectangles, in order to hold line segment vertex
		// and optimize line segment connecting process.
		typedef std::vector<N_LineSegmentVertex> N_LayerTile;

		struct N_Layer
		{
			//2D array, with element count of const_LayerTileStepCount^2
			//layerTile[x][z]
			N_Layer()
			{
				//create a new block with size of (const_LayerTileStepCount x const_LayerTileStepCount)
				layerTile.resize(CONST_LayerTileStepCount);
				for (UINT i = 0;i < CONST_LayerTileStepCount;i++)
				{
					layerTile.at(i).resize(CONST_LayerTileStepCount);
				}
			};

			std::vector<std::vector<N_LayerTile>> layerTile;
		};



class /*_declspec(dllexport)*/ IMeshSlicer : private IFileLoader
		{

		public:

			IMeshSlicer();

			BOOL	Step1_LoadPrimitiveMeshFromSTLFile(std::string  pFilePath);

			void		Step2_Intersection(UINT iLayerCount);

			void		Step3_GenerateLineStrip();

			UINT	GetLineSegmentCount();

			void		GetLineSegmentBuffer(std::vector<N_LineSegment>& outBuffer);

			void		GetBoundingBox(BOUNDINGBOX& outAABB);


		private:

			void		mFunction_ComputeBoundingBox();

			BOOL	mFunction_Intersect_LineSeg_Layer(Math::VECTOR3 v1, Math::VECTOR3 v2, float layerY, Math::VECTOR3* outIntersectPoint);

			void		mFunction_GenerateLayerTileInformation();

			void		mFunction_GetLayerTileIDFromPoint(Math::VECTOR3 v, UINT& tileID_X, UINT& tileID_Z);

			BOOL	mFunction_LineStrip_FindNextPoint(Math::VECTOR3* tailPoint, UINT currentLayerID, N_LineStrip* currLineStrip);

			Math::VECTOR3 mFunction_Compute_Normal2D(Math::VECTOR3 triangleNormal);

			N_IntersectionResult	mFunction_HowManyVertexOnThisLayer(float currentlayerY, Math::VECTOR3& v1, Math::VECTOR3& v2, Math::VECTOR3& v3);



			std::vector<Math::VECTOR3>*			m_pPrimitiveVertexBuffer;

			std::vector<Math::VECTOR3>*			m_pTriangleNormalBuffer;

			std::vector<N_LineSegment>*			m_pLineSegmentBuffer;

			std::vector<N_Layer>*						m_pLayerList;	//for every N_Layer , there are an 2D layer tile array

			std::vector<N_LineStrip>*					m_pLineStripBuffer;

			Math::VECTOR3*									m_pBoundingBox_Min;

			Math::VECTOR3*									m_pBoundingBox_Max;

		};
