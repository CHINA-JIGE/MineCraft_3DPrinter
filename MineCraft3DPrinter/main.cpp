#include "Printer.h"

//debug test param:
//sphere.stl 100 100 100 10 10 10 stone yes

int main()
{
	std::cout << "��ӭʹ�ü����MineCraft 3D��ӡ��ң�" << std::endl << std::endl;
	std::cout << "˵����" << std::endl;
	std::cout << "��������ʵ���ǻ��ڰ���ģ���mc����̨��һ��С����" << std::endl;
	std::cout << "�ڼ���STL��ʽ��3Dģ���ļ�������һϵ�м���֮��" << std::endl;
	std::cout << "�ѽ������MineCraft����Ȼ��Ϳ�����mineCraftָ�������������ذ�3dģ������" << std::endl << std::endl;
	std::cout << "-----ע��1��ʹ�õ�MC������ /setblock��Ŀ��mc�汾��1.9.4������֤���¼���" << std::endl;

	std::cout << "-----ע��2��MC��y��������ֱ���꣬�Ǵ�ֱ��XZˮƽ���" << std::endl;
	std::cout << "-----ע��3������ڴ���ģʽ��ʹ��" << std::endl;
	std::cout << "-----ע��4���ڼ�����ɺ�ᵯ��һ���Ի����ڰ��¶Ի���ȷ���Ժ���10�����ڻص�MC�Ľ���" << std::endl;

	std::cout << "------������������� [STLģ���ļ�·��] [x] [y] [z] [size_X] [size_Y] [size_Z] [�������֣������У�stone] [�Ƿ���������ڲ�(yes|no)]" << std::endl;
	std::cout << "------ʾ����tree.STL 100 200 300 40 40 80 stone yes" << std::endl;

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
	if(layerCount > 200)
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