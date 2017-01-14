#include "Printer.h"

//debug test param:
//sphere.stl 100 100 100 10 10 10 stone yes

int main()
{
	std::cout << "欢迎使用鸡哥的MineCraft 3D打印外挂！" << std::endl << std::endl;
	std::cout << "说明：" << std::endl;
	std::cout << "这个外挂其实就是基于按键模拟和mc控制台的一个小程序，" << std::endl;
	std::cout << "在加载STL格式的3D模型文件，经过一系列计算之后，" << std::endl;
	std::cout << "把焦点给回MineCraft程序，然后就可以在mineCraft指定区域生成像素版3d模型啦！" << std::endl << std::endl;
	std::cout << "-----注意1：使用的MC命令是 /setblock，目标mc版本是1.9.4，不保证向下兼容" << std::endl;

	std::cout << "-----注意2：MC里y坐标是竖直坐标，是垂直于XZ水平面的" << std::endl;
	std::cout << "-----注意3：务必在创造模式里使用" << std::endl;
	std::cout << "-----注意4：在计算完成后会弹出一个对话框，在按下对话框确定以后在10秒以内回到MC的界面" << std::endl;

	std::cout << "------请输入参数串： [STL模型文件路径] [x] [y] [z] [size_X] [size_Y] [size_Z] [方块名字，例如有：stone] [是否填充物体内部(yes|no)]" << std::endl;
	std::cout << "------示例：tree.STL 100 200 300 40 40 80 stone yes" << std::endl;

	std::cout << "--->";

	UINT posX, posY, posZ, sizeX, sizeY, sizeZ;
	std::string filePath,blockName,strEnablePadding;
	//after rasterizing lines, there is an optional steps which pad the pixel area inside closed lines

	//input strings
	/*std::cin >> filePath >> posX >> posY >> posZ  >> 
		sizeX >> sizeY >> sizeZ >> blockName >>strEnablePadding;*/
	filePath = "sphere.stl";
	posX = posY = posZ = sizeX = sizeY = sizeZ = 20;
	blockName = "stone";
	strEnablePadding = "false";

	bool enablePadding = strEnablePadding == "yes" ? true : false;


	//slice mesh
	IMeshSlicer slicer;

	//step1 - load file
	std::cout << "正在加载STL模型文件...." << std::endl;
	BOOL fileLoadSucceeded = slicer.Step1_LoadPrimitiveMeshFromSTLFile(filePath);
	if (!fileLoadSucceeded) 
	{
		std::cout << "错误：STL 文件路径错误！" << std::endl; 
		system("pause");
		return 0;
	}
	std::cout << "STL模型文件加载成功！" << std::endl;

	//step2 - intersection
	std::cout << "正在计算切层...." << std::endl;
	UINT layerCount = sizeY;
	if(layerCount > 200)
	{ 
		std::cout << "错误：安全起见，高度还是不要多于200个方块！" << std::endl; 	
		system("pause");
		return 0;
	}
	 slicer.Step2_Intersection(layerCount);
	 std::cout << "切层计算完毕！" << std::endl;

	 //get line segement buffer and then rasterize every line segment, 
	 //pad inner area in the end
	 std::vector<N_LineSegment> lineSegmentList;
	 slicer.GetLineSegmentBuffer(lineSegmentList);
	 BOUNDINGBOX bbox;
	 slicer.GetBoundingBox(bbox);
	 VECTOR2 layerMin = { bbox.min.x,bbox.min.z };
	 VECTOR2 layerMax = { bbox.max.x,bbox.max.z };

	 //step3 - rasterization
	 ILayerRasterizer LR;
	 LR.Init(sizeX, sizeZ, layerCount, layerMin, layerMax);
	 LR.Rasterize(lineSegmentList,enablePadding);
	 auto layerGroup = LR.GetRasterizedLayerGroup();

	 for (UINT i = 0;i < sizeY;++i)
	 {
		 std::cout << "Rasterized result : layer " << i << std::endl;
		 for (UINT j = 0;j < sizeX;++j)
		 {
			 for (UINT k = 0;k < sizeZ;++k)
			 {
				 std::cout << UINT(layerGroup->at(i).pixelArray[j][k]);
			 }
			 std::cout << std::endl;
		 }
		 			 std::cout << std::endl;
	 }

	system("pause");
	return 0;
}