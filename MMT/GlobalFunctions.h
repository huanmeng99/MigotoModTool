#pragma once
#include "GlobalConfigs.h"
//这个类里只允许放程序入口调用的方法


//鸣潮
void ExtractFromWW();
void ExtractFromBuffer_CS_WW_Body(std::wstring DrawIB);
void ExtractFromBuffer_VS_WW_Object(std::wstring DrawIB);
void ExtractFromBuffer_VS_WW_ObjectType2(std::wstring DrawIB);
void Generate_CS_WW_Body();

//原神,崩坏:星穹铁道,崩坏三,绝区零
void ExtractFromBuffer_VS();
void UnityGenerate();

//原神,崩坏:星穹铁道,崩坏三,绝区零		Dump式逆向提取Mod模型
void ExtractFromBuffer_VS_Reverse();

//原神,崩坏:星穹铁道,崩坏三,绝区零		脚本式逆向提取Mod模型
void ReverseOutfitCompilerCompressed();
void ReverseSingle();
void ReverseMerged();


