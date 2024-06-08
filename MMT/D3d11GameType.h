#pragma once
#include <string>
#include <vector>
#include <unordered_map>


class D3D11Element {
public:
    std::string SemanticName = "";
    std::string SemanticIndex = "";
    std::string Format = "";
    std::string InputSlot = "0";
    std::string InputSlotClass = "per-vertex";
    std::string InstanceDataStepRate = "0";
    int ByteWidth = 0;
    std::string ExtractSlot = "";
    std::string ExtractTechnique = "";
    std::string Category = "";

    //��Ҫ��̬����
    int ElementNumber = 0;
    int AlignedByteOffset = 0;
};


class D3D11GameType {
public:
    std::string GameType;
    std::string Engine;

    //4D��normal��ȡʱȥ�����һλ����������Modʱ���ӻ���
    bool Normal4Dimension = false;

    //�Ƿ���Ҫ����blendweights��Ĭ�ϲ���Ҫ
    bool PatchBLENDWEIGHTS = false;

    std::string RootComputeShaderHash;
    //ԭ���������ini�ļ����滻ʱд����draw��λ�����ÿһ�������ﶼ��Ҫ�ֶ�ָ��
    std::unordered_map <std::string, std::string> CategoryDrawCategoryMap;
    std::vector<std::string> OrderedFullElementList;

    //��Ҫ��������ó�������
    std::unordered_map<std::string, D3D11Element> ElementNameD3D11ElementMap;
    std::unordered_map <std::string, std::string> CategorySlotMap;
    std::unordered_map <std::string, std::string> CategoryTopologyMap;

    D3D11GameType();

    //�����ṩ��ElementList��ȡ�ܵ�Stride
    int getElementListStride(std::vector<std::string>);

    //��ȡCategory Stride Map
    std::unordered_map<std::string, int>  getCategoryStrideMap(std::vector<std::string> inputElementList);

    //��ȡCategory List
    std::vector<std::string>  getCategoryList(std::vector<std::string> inputElementList);

    //��ȡCategory��ElementList

    std::vector<std::string> getCategoryElementList(std::vector<std::string> inputElementList,std::string category);

    std::vector<std::string> getReorderedElementList(std::vector<std::string> elementList);
};