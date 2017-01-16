#include "Printer.h"

//debug test param:
//sphere.stl 100 100 100 10 10 10 stone yes
static int printedBlockCount = 0;

void SimulateKeyboard(
	UINT posX ,UINT posY, UINT posZ, 
	UINT sizeX,UINT sizeY, UINT sizeZ, 
	std::string blockName, const std::vector<Layer>* layerGroup);

int main(int argc, char* argv[])
{
	std::cout << "欢迎使用鸡哥的MineCraft 3D打印外挂！" << std::endl << std::endl;
	std::cout << "说明：" << std::endl;
	std::cout << "这个外挂其实就是基于按键模拟和mc控制台的一个小程序，" << std::endl;
	std::cout << "在加载STL格式的3D模型文件，经过一系列计算之后，" << std::endl;
	std::cout << "把焦点给回MineCraft程序，然后就可以在mineCraft指定区域生成像素版3d模型啦！" << std::endl << std::endl;
	std::cout << "-----注意1：使用的MC命令是 /fill，目标mc版本是1.9.4，不保证向下兼容" << std::endl;
	std::cout << "-----注意2：MC里y坐标是竖直坐标，是垂直于XZ水平面的" << std::endl;
	std::cout << "-----注意3：务必在创造模式里使用" << std::endl;
	std::cout << "-----注意4：在计算完成后会弹出一个对话框，在按下对话框确定以后在10秒以内回到MC的界面" << std::endl;
	std::cout << "-----注意5：请将输入法调至：英文，小写" << std::endl;


	std::cout << "------请输入参数串： [STL模型文件路径] [x] [y] [z] [size_X] [size_Y] [size_Z] [方块名字，例如有：stone] [是否填充物体内部(yes|no)]" << std::endl;
	std::cout << "------示例：tree.STL 100 200 300 40 40 80 stone yes" << std::endl;

	std::cout << "--->";

	UINT posX, posY, posZ, sizeX, sizeY, sizeZ;
	std::string filePath, blockName, strEnablePadding;
	//after rasterizing lines, there is an optional steps which pad the pixel area inside closed lines

	//if external command is passed in, then we won't use CIN is get command parameter
	bool isExternalCommandMode = false;
	//how many passes this printing task have
	UINT passCount = 1;

	const UINT paramCount = 9;

	if (argc <= 1)
	{
		isExternalCommandMode = false;
		std::cin >> filePath >> posX >> posY >> posZ >>
			sizeX >> sizeY >> sizeZ >> blockName >> strEnablePadding;
	}
	else
	{
		//params are passed through main() function
		passCount = (argc-1) / paramCount;
		isExternalCommandMode = true;
	}

	for(UINT i=0;i<passCount;++i) 
	{
	/*filePath = "sphere.stl";
	posX = posY = posZ = sizeX = sizeY = 15;sizeZ = 15;
	blockName = "stone";
	strEnablePadding = "yes";*/

		//there might be serveral passes
		if (isExternalCommandMode)
		{
			std::stringstream ss;
			//read all params of one pass
			for (int c = 0;c < paramCount;c++)ss << argv[i * paramCount + c +1]<<" ";

			//stream output to each variable
			ss >> filePath >> posX >> posY >> posZ >>
				sizeX >> sizeY >> sizeZ >> blockName >> strEnablePadding;
      		}

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
		if (layerCount > 200)
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
		std::cout << "正在光栅化...." << std::endl;
		ILayerRasterizer LR;
		LR.Init(sizeX, sizeZ, layerCount, layerMin, layerMax);
		LR.Rasterize(lineSegmentList, enablePadding);
		const std::vector<Layer>* pLayerGroup = LR.GetRasterizedLayerGroup();
		std::cout << "光栅化已完成！" << std::endl;

		/*//debug info for rasterization result
		for (UINT z = 0;z < pLayerGroup->size();++z)
		{
		   auto& layerPixelMatrix = pLayerGroup->at(z).pixelArray;
		   std::cout << "layer " << z << std::endl;
		   for (UINT x = 0;x < layerPixelMatrix.size();++x)
		   {
			   for (UINT y = 0;y < layerPixelMatrix[x].size();++y)
			   {
				std::cout << layerPixelMatrix[x][y];
			   }
			   std::cout << std::endl;
			}
		}*/


		//keyboard simulation preparation
		if (isExternalCommandMode)
		{
			std::cout << "计算已完成！请在按下确定后在10秒之内把输入法切换至小写的英文，"
				"然后把焦点切换回MineCraft程序！然后就不要再动键盘或者电脑了！直到打印结束！" << std::endl;
		}
		else
		{
			::MessageBoxW(0,
				L"计算已完成！请在按下确定后在10秒之内把输入法切换至小写的英文，"
				"然后把焦点切换回MineCraft程序！然后就不要再动键盘或者电脑了！直到打印结束！", 0, 1);
		}


		Sleep(10000);

		//start simulation
		SimulateKeyboard(posX, posY, posZ, sizeX, sizeY, sizeZ, blockName, pLayerGroup);

	}

	 ::MessageBoxW(0,L"打印任务完成！", 0, 0);

	 std::cout << "本次MineCraft 3D打印任务成功完成!" << std::endl;
	 std::cout << "共打印方块数：" << printedBlockCount<<std::endl;

	system("pause");
	return 0;
}



void SimulateKeyboard(
	UINT pxPosX, UINT pxPosY, UINT pxPosZ,
	UINT pxSizeX,UINT pxSizeY,UINT pxSizeZ, 
	std::string blockName, const std::vector<Layer>* layerGroup)
{


	auto key = [](byte vkcode)
	{
		::keybd_event(vkcode, 0, 0, 0);
		::keybd_event(vkcode, 0, KEYEVENTF_KEYUP, 0);
		::Sleep(80);
	};


	/* for (UINT z = 0;z < layerGroup->size();++z)
	 {
		 auto& layerPixelMatrix = layerGroup->at(z).pixelArray;
		 for (UINT x = 0;x < layerPixelMatrix.size();++x)
		 {
			 for (UINT y = 0;y < layerPixelMatrix[x].size();++y)
			 {
				 if (layerPixelMatrix[x][y] == 1)
				 {
					 key(VK_OEM_2);// a key with /?
					 key('F');	key('I');	key('L');	key('L');
					 key(VK_SPACE);

					 std::string strPosX = std::to_string(posX + x);
					 std::string strPosY = std::to_string(posY + y);
					 std::string strPosZ = std::to_string(posZ + z);

					 for (auto c : strPosX)key(c - '0' + VK_NUMPAD0);
					 key(VK_SPACE);
					 for (auto c : strPosY)key(c - '0' + VK_NUMPAD0);
					 key(VK_SPACE);
					 for (auto c : strPosZ)key(c - '0' + VK_NUMPAD0);
					 key(VK_SPACE);

					 for (auto c : blockName) key(toupper(c));

					 //confirm
					 key(VK_RETURN);
					 ++printedBlockCount;
				 }
			 }
		 }
	 }*/


	for (UINT y = 0;y < layerGroup->size();++y)
	{
		// because /fill command can fill a series of block
		//thus complexity is  immediately  reduced by one order of magnitude.
		auto& paddingRowList = layerGroup->at(y).rasterizeIntersectXList;
		for (UINT z = 0;z < paddingRowList.size();++z)
		{
			for (UINT i = 0;i < paddingRowList[z].size();i+=2)
			{
				if (paddingRowList[z].size() - i == 1)break;

				// a key with /? , open MC console
				key(VK_OEM_2);
				key('F');	key('I');	key('L');	key('L');
				key(VK_SPACE);


				UINT startPosY = pxPosY + y;
				UINT startPosZ = pxPosZ + z;
				UINT startPosX = pxPosX + UINT(paddingRowList[z][i] * float(pxSizeX));
				UINT endPosX = pxPosX + UINT(paddingRowList[z][i+1] * float(pxSizeX));

				std::string strStartPosX = std::to_string(startPosX);
				std::string strEndPosX = std::to_string(endPosX);
				std::string strPosY = std::to_string(startPosY);
				std::string strPosZ = std::to_string(startPosZ);

				for (auto c : strStartPosX) { if (c == '-')key(VK_OEM_MINUS);else key(c - '0' + VK_NUMPAD0); }
				key(VK_SPACE);
				for (auto c : strPosY) { if (c == '-')key(VK_OEM_MINUS);else key(c - '0' + VK_NUMPAD0); }
				key(VK_SPACE);
				for (auto c : strPosZ) { if (c == '-')key(VK_OEM_MINUS);else key(c - '0' + VK_NUMPAD0); }
				key(VK_SPACE);


				for (auto c : strEndPosX) { if (c == '-')key(VK_OEM_MINUS);else key(c - '0' + VK_NUMPAD0); }
				key(VK_SPACE);
				for (auto c : strPosY) { if (c == '-')key(VK_OEM_MINUS);else key(c - '0' + VK_NUMPAD0); }
				key(VK_SPACE);
				for (auto c : strPosZ) { if (c == '-')key(VK_OEM_MINUS);else key(c - '0' + VK_NUMPAD0); }
				key(VK_SPACE);

				for (auto c : blockName) key(toupper(c));

				//confirm
				key(VK_RETURN);
				++printedBlockCount;
			}
		}
	}


}