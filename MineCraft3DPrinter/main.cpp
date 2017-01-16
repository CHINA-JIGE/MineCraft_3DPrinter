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
	std::cout << "��ӭʹ�ü����MineCraft 3D��ӡ��ң�" << std::endl << std::endl;
	std::cout << "˵����" << std::endl;
	std::cout << "��������ʵ���ǻ��ڰ���ģ���mc����̨��һ��С����" << std::endl;
	std::cout << "�ڼ���STL��ʽ��3Dģ���ļ�������һϵ�м���֮��" << std::endl;
	std::cout << "�ѽ������MineCraft����Ȼ��Ϳ�����mineCraftָ�������������ذ�3dģ������" << std::endl << std::endl;
	std::cout << "-----ע��1��ʹ�õ�MC������ /fill��Ŀ��mc�汾��1.9.4������֤���¼���" << std::endl;
	std::cout << "-----ע��2��MC��y��������ֱ���꣬�Ǵ�ֱ��XZˮƽ���" << std::endl;
	std::cout << "-----ע��3������ڴ���ģʽ��ʹ��" << std::endl;
	std::cout << "-----ע��4���ڼ�����ɺ�ᵯ��һ���Ի����ڰ��¶Ի���ȷ���Ժ���10�����ڻص�MC�Ľ���" << std::endl;
	std::cout << "-----ע��5���뽫���뷨������Ӣ�ģ�Сд" << std::endl;


	std::cout << "------������������� [STLģ���ļ�·��] [x] [y] [z] [size_X] [size_Y] [size_Z] [�������֣������У�stone] [�Ƿ���������ڲ�(yes|no)]" << std::endl;
	std::cout << "------ʾ����tree.STL 100 200 300 40 40 80 stone yes" << std::endl;

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
		std::cout << "���ڼ���STLģ���ļ�...." << std::endl;
		BOOL fileLoadSucceeded = slicer.Step1_LoadPrimitiveMeshFromSTLFile(filePath);
		if (!fileLoadSucceeded)
		{
			std::cout << "����STL �ļ�·������" << std::endl;
			system("pause");
			return 0;
		}
		std::cout << "STLģ���ļ����سɹ���" << std::endl;

		//step2 - intersection
		std::cout << "���ڼ����в�...." << std::endl;
		UINT layerCount = sizeY;
		if (layerCount > 200)
		{
			std::cout << "���󣺰�ȫ������߶Ȼ��ǲ�Ҫ����200�����飡" << std::endl;
			system("pause");
			return 0;
		}
		slicer.Step2_Intersection(layerCount);
		std::cout << "�в������ϣ�" << std::endl;

		//get line segement buffer and then rasterize every line segment, 
		//pad inner area in the end
		std::vector<N_LineSegment> lineSegmentList;
		slicer.GetLineSegmentBuffer(lineSegmentList);
		BOUNDINGBOX bbox;
		slicer.GetBoundingBox(bbox);
		VECTOR2 layerMin = { bbox.min.x,bbox.min.z };
		VECTOR2 layerMax = { bbox.max.x,bbox.max.z };

		//step3 - rasterization
		std::cout << "���ڹ�դ��...." << std::endl;
		ILayerRasterizer LR;
		LR.Init(sizeX, sizeZ, layerCount, layerMin, layerMax);
		LR.Rasterize(lineSegmentList, enablePadding);
		const std::vector<Layer>* pLayerGroup = LR.GetRasterizedLayerGroup();
		std::cout << "��դ������ɣ�" << std::endl;

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
			std::cout << "��������ɣ����ڰ���ȷ������10��֮�ڰ����뷨�л���Сд��Ӣ�ģ�"
				"Ȼ��ѽ����л���MineCraft����Ȼ��Ͳ�Ҫ�ٶ����̻��ߵ����ˣ�ֱ����ӡ������" << std::endl;
		}
		else
		{
			::MessageBoxW(0,
				L"��������ɣ����ڰ���ȷ������10��֮�ڰ����뷨�л���Сд��Ӣ�ģ�"
				"Ȼ��ѽ����л���MineCraft����Ȼ��Ͳ�Ҫ�ٶ����̻��ߵ����ˣ�ֱ����ӡ������", 0, 1);
		}


		Sleep(10000);

		//start simulation
		SimulateKeyboard(posX, posY, posZ, sizeX, sizeY, sizeZ, blockName, pLayerGroup);

	}

	 ::MessageBoxW(0,L"��ӡ������ɣ�", 0, 0);

	 std::cout << "����MineCraft 3D��ӡ����ɹ����!" << std::endl;
	 std::cout << "����ӡ��������" << printedBlockCount<<std::endl;

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