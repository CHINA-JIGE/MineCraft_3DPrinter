/*********************************************************

									Layer Rasterizer

********************************************************/

#include "Printer.h"

void ILayerRasterizer::Init(UINT pixelWidth, UINT pixelDepth, UINT layerCount, const VECTOR2& XZ_min, const VECTOR2& XZ_max)
{
	//exploit enough memory space for all blocks
	mLayerGroup.resize(layerCount);//[y]

	mLayerPixelWidth = pixelWidth;
	mLayerPixelHeight = pixelDepth;

	for (UINT  L = 0;L < layerCount;L++)
	{
		mLayerGroup[L].pixelArray.resize(pixelDepth);//[x]

		for (int i = 0;i < pixelDepth;++i)
		{
			mLayerGroup[L].pixelArray[i].resize(pixelWidth, 0);//[z]
		}
	}

	mLayerPosMin = XZ_min;
	mLayerPosMax = XZ_max;
	mLayerRealWidth = mLayerPosMax.x - mLayerPosMin.x;
	mLayerRealDepth = mLayerPosMax.y - mLayerPosMin.y;
}

void ILayerRasterizer::Rasterize(const std::vector<N_LineSegment>& lineSegList,  bool padInsideArea)
{
	for (auto& line : lineSegList)
	{
		mFunction_RasterizeLine(line);
	}

	if (padInsideArea)
	{
		for (auto& layer : mLayerGroup)mFunction_PadInnerArea(layer);
	}
}

const std::vector<Layer>* ILayerRasterizer::GetRasterizedLayerGroup()
{
	return &mLayerGroup;
}

/*******************************************************

									PRIVATE

*********************************************************/

void ILayerRasterizer::mFunction_RasterizeLine(const N_LineSegment & line)
{
	//v1,v2 are transformed into NORMALIZED space, valued in [0,1]
	//the point with minimum x,y coord is the origin point
	VECTOR2 v1 = { (line.v1.x - mLayerPosMin.x)/mLayerRealWidth , (line.v1.z - mLayerPosMin.y)/mLayerRealDepth };
	VECTOR2 v2 = { (line.v2.x - mLayerPosMin.x) / mLayerRealWidth , (line.v2.z - mLayerPosMin.y) / mLayerRealDepth };

	//line segment could come from any layer (with different Y coordinate)
	Layer& targetLayer = mLayerGroup.at(line.LayerID);

	//slope not exist
	if (v1.x == v2.x)
	{
		//FLOOR() truncation
		UINT x = UINT(v1.x *mLayerPixelWidth);//same as v2.x * pixelWidth
		UINT startY = UINT(v1.y * mLayerPixelHeight);
		UINT endY = UINT(v2.y * mLayerPixelHeight);
		if (startY == mLayerPixelHeight)startY = mLayerPixelHeight-1;
		if (endY == mLayerPixelHeight)endY = mLayerPixelHeight-1;

		//rasterize a line parallel with Y axis
		//NOTE: don't worry that byte data overflow caused by too many self-increase
		//because (byte % 2) is what really matters in padding
		for (int i = startY;i <= endY;++i) ++(targetLayer.pixelArray[x][i]);
	}

	//compute slope
	float k = (v2.y - v1.y) / (v2.x - v1.x);

	
	//---1.   if abs(slope)>=1.0f, then every y coord must be checked
	if (k >= -1.0f || k <= 1.0f )
	{
		UINT startX = UINT(v1.x * mLayerPixelWidth);
		UINT endX = UINT(v2.x * mLayerPixelWidth);
		UINT startY = UINT(v1.y * mLayerPixelHeight);
		if (startX == mLayerPixelWidth)startX = mLayerPixelWidth - 1;
		if (endX == mLayerPixelWidth)endX = mLayerPixelWidth - 1;
		if (startY == mLayerPixelHeight)startY = mLayerPixelHeight - 1;

		// delta_y = k * delta_x
		for (UINT i = startX; i <= endX;++i)
		{
			int pixelY = int(k * (i - startX));
			++(targetLayer.pixelArray[i][startY + pixelY]);
		}
	}

	//---2.   if abs(slope)>=1.0f, then every x coord must be checked
	if ( k < -1.0f || k > 1.0f )
	{
		UINT startY = UINT(v1.y * mLayerPixelHeight);
		UINT endY = UINT(v2.y * mLayerPixelHeight);
		UINT startX = UINT(v1.x * mLayerPixelWidth);
		if (startY == mLayerPixelHeight)startY = mLayerPixelHeight - 1;
		if (endY == mLayerPixelHeight)endY = mLayerPixelHeight - 1;
		if (startX == mLayerPixelWidth)startX = mLayerPixelWidth - 1;

		// delta_y = k * delta_x
		for (UINT i = startY; i <= endY;++i)
		{
			int pixelX = startX + int(1.0f / k * (i - startY));
			++(targetLayer.pixelArray[pixelX][i]);
		}
	}

}

void ILayerRasterizer::mFunction_PadInnerArea(Layer& layer)
{
	//Scan Line Padding , horizontal line scan from top to bottom
    for (UINT i = 0;i < mLayerPixelHeight;++i)
	{
		int padState = 0;

		//for every pixel row
		for (UINT j = 0;j < mLayerPixelWidth;++j)
		{
			//NOTE: when using a scan line padding method
			//each row of pixels will be scanned from left to right
			//when encounter a pixel on which odd number of lines lie,
			//padding start, UNTIL another pixel on which odd number of lines lie is encountered

			//FINALLY, all pixel with value larger than 0 is occupied by a minecraft block

			//NOTE: +2 can remain the ODEVITY of the value of that pixel
			if (padState % 2 == 1) layer.pixelArray[j][i] += 2;

			padState += layer.pixelArray[j][i];
		}

	}

}
