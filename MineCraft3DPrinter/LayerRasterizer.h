/*********************************************************

			Layer Rasterizer aims to rasterize line segments
		at the beginning, then pad the inner area enclosed
		by closed line

		author : Jige

********************************************************/

#pragma once

struct Layer
{
	//when an analytical line segment is rasterized,
	//the value of pixels covered by rasterized line will increase by 1
	std::vector<std::vector<byte>> pixelArray;

	//use scan line algorithm, every line segments will intersect with
	//each scan line , then there will many intersect points derived by each scan line
	//because scan line is parallel with x axis, thus only x coordinate need to be stored
	std::vector<std::vector<float>>	rasterizeIntersectXList;
};

class ILayerRasterizer
{
public:

	void Init(UINT pixelWidth, UINT pixelDepth, UINT layerCount,const VECTOR2& XZ_min, const VECTOR2& XZ_max);

	void Rasterize(const std::vector<N_LineSegment>& lineSegList,bool padInsideArea);

	const std::vector<Layer>* GetRasterizedLayerGroup();

private:

	void mFunction_LineSegment_Scanline_Intersect(const N_LineSegment& line,UINT scanlineRowID, float y);

	// optional process after line rasterization(pad the inside area of closed lines)
	void mFunction_PadInnerArea(Layer& layer, bool padInsideArea);

	std::vector<Layer> mLayerGroup;



	UINT mLayerPixelWidth;//how many blocks "wide"

	UINT mLayerPixelHeight;

	VECTOR2 mLayerPosMin;

	VECTOR2 mLayerPosMax;

	float	mLayerRealWidth;

	float mLayerRealDepth;

};
