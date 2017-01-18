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
		mLayerGroup[L].pixelArray.resize(pixelWidth);//[x]
		mLayerGroup[L].rasterizeIntersectXList.resize(pixelDepth);//totally 'pixelDepth' number of rows

		for (UINT i = 0;i < pixelWidth;++i)
		{
			mLayerGroup[L].pixelArray[i].resize(pixelDepth, 0);//[z]
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
		for (UINT i = 0;i < mLayerPixelHeight;++i)
		{
			// layer pixel height = rows in a layer
			float normalized_scanlineY = ((float(i) + 0.5f) / mLayerPixelHeight);
			mFunction_LineSegment_Scanline_Intersect(line,i, normalized_scanlineY);
		}
	}

	//intersect X coord should be sorted in order to use scan line padding algorithm
	for (auto& layer : mLayerGroup)
	{
		for (auto & row : layer.rasterizeIntersectXList)
			std::sort(row.begin(),row.end());
	}

	//for (auto& layer : mLayerGroup)mFunction_PadInnerArea(layer,padInsideArea);

}

const std::vector<Layer>* ILayerRasterizer::GetRasterizedLayerGroup()
{
	return &mLayerGroup;
}

/*******************************************************

									PRIVATE

*********************************************************/

void ILayerRasterizer::mFunction_LineSegment_Scanline_Intersect(const N_LineSegment & line, UINT scanlineRowID,float y)
{

	//v1,v2 are transformed into NORMALIZED space, valued in [0,1]
	//the point with minimum x,y coord is the origin point
	VECTOR2 v1 = { (line.v1.x - mLayerPosMin.x)/mLayerRealWidth , (line.v1.z - mLayerPosMin.y)/mLayerRealDepth };
	VECTOR2 v2 = { (line.v2.x - mLayerPosMin.x) / mLayerRealWidth , (line.v2.z - mLayerPosMin.y) / mLayerRealDepth };


	//line segment could come from any layer (with different Y coordinate)
	Layer& targetLayer = mLayerGroup.at(line.LayerID);

	if ((v1.y > y && v2.y > y) || (v1.y < y && v2.y <y ))
	{
		//there is no way scan line and line segment can intersect like this
		return;
	}



	// now y valued between v1.y and v2.y can be assured
	if (v1.y == v2.y)
	{
		//this is a very special case

		//v1.x and v2.x serve as the X region that can pad the line segment
		targetLayer.rasterizeIntersectXList.at(scanlineRowID).push_back(v1.x);
		targetLayer.rasterizeIntersectXList.at(scanlineRowID).push_back(v2.x);

		//this v2.x serve as the start X of next horizontal padding area
		targetLayer.rasterizeIntersectXList.at(scanlineRowID).push_back(v2.x);
		return;
	}

	//vector ratio coeffient t
	float t = (y - v1.y) / (v2.y - v1.y);
	if (t >= 0.0f && t < 1.0f)
	{
		targetLayer.rasterizeIntersectXList.at(scanlineRowID).push_back(v1.x + t * (v2.x - v1.x));
	}
	return;

}

//currently not in use (2017.1.19)
void ILayerRasterizer::mFunction_PadInnerArea(Layer& layer, bool padInsideArea)
{
	//Scan Line Padding , horizontal line scan from top to bottom
    for (UINT y = 0;y < layer.rasterizeIntersectXList.size();++y)
	{
		auto& XCoordRow = layer.rasterizeIntersectXList.at(y);
		//for every X coord row
		for (UINT j = 0;j < XCoordRow.size(); j += 2)
		{
			//I don't know why some rows have odd intersect points....
			if (XCoordRow.size() - j == 1)break;

			//NOTE: accurate X coordinate of intersect points derived from
			//each scan line had been computed. now we only need quantize
			//these X coord and start padding from odd index to even index X coord

			UINT startX = UINT(XCoordRow.at(j)  * float(mLayerPixelWidth));
			UINT endX = UINT(XCoordRow.at(j + 1)*float(mLayerPixelWidth));

			if (padInsideArea)
			{
				//pad from left to right
				for (UINT x = startX;x <= endX;++x)
				{
					layer.pixelArray[x][y] = 1;
				}
			}
			else
			{
				//disable padding , only mark edge pixels
				 layer.pixelArray[startX][y] = layer.pixelArray[endX][y] = 1;
			}
		}

	}

}
