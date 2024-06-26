#include "MigotoParseUtil.h"
#include "MMTLogUtils.h"
#include "MMTFormatUtils.h"
#include "MMTFileUtils.h"
#include <filesystem>


void cartesianProductHelper2(const std::vector<std::vector<std::wstring>>& data, std::vector<std::wstring>& current, std::vector<std::vector<std::wstring>>& result, size_t index) {
    if (index >= data.size()) {
        result.push_back(current);
        return;
    }

    for (const auto& str : data[index]) {
        current.push_back(str);
        cartesianProductHelper2(data, current, result, index + 1);
        current.pop_back();
    }
}

std::vector<std::vector<std::wstring>> cartesianProduct2(const std::vector<std::vector<std::wstring>>& data) {
    std::vector<std::vector<std::wstring>> result;
    std::vector<std::wstring> current;
    cartesianProductHelper2(data, current, result, 0);
    return result;
}

std::vector<std::unordered_map<std::wstring, std::wstring>> MigotoParseUtil_Get_M_Key_Combination(std::vector<M_Key> cycleKeyList) {
    LOG.Info(L"Start to calculateKeyCombination");
    std::vector<std::unordered_map<std::wstring, std::wstring>> keyCombinationDictList;

    std::vector<std::vector<std::wstring>> varValuesList;
    std::vector<std::wstring> varNameList;

    for (M_Key cycleKey : cycleKeyList) {

        if (cycleKey.CycleVariableName_PossibleValueList_Map.size() != 0) {

            for (const auto& pair : cycleKey.CycleVariableName_PossibleValueList_Map) {
                LOG.Info(L"Key: " + pair.first);

                std::vector<std::wstring> trueVarValueList;
                for (std::wstring varValue : pair.second) {
                    LOG.Info(L"Value: " + varValue);
                    trueVarValueList.push_back(varValue);
                }
                varValuesList.push_back(trueVarValueList);
                varNameList.push_back(pair.first);
            }
        }
    }

    std::vector<std::vector<std::wstring>> cartesianProductList = cartesianProduct2(varValuesList);
    for (std::vector<std::wstring> keyCombinationValueList : cartesianProductList) {
        std::unordered_map<std::wstring, std::wstring> tmpDict;
        for (int i = 0; i < keyCombinationValueList.size(); i++) {
            std::wstring cycleVarValue = keyCombinationValueList[i];
            std::wstring cycleVarName = varNameList[i];
            //LOG.LogOutput(cycleVarName + L" " + cycleVarValue);
            tmpDict[cycleVarName] = cycleVarValue;
        }
        keyCombinationDictList.push_back(tmpDict);
        //LOG.LogOutputSplitStr();
    }
    //LOG.LogOutputSplitStr();
    return keyCombinationDictList;
}



std::vector<M_DrawIndexed> MigotoParseUtil_GetActiveDrawIndexedListByKeyCombination(std::unordered_map<std::wstring, std::wstring> KeyCombinationMap, std::vector<M_DrawIndexed> DrawIndexedList) {
    std::vector<M_DrawIndexed> activitedDrawIndexedList;
    //这里要让DrawIndex的每个Condition都满足,才算是被激活了

    for (M_DrawIndexed drawIndexed : DrawIndexedList) {

        if (drawIndexed.AutoDraw) {
            //如果没有条件，说明直接就是已激活的
            activitedDrawIndexedList.push_back(drawIndexed);
            //LOG.Info(L"Detect [auto draw drawindexed]");
        }
        else {
            //LOG.Info(L"ActiveConditionList size: " + std::to_wstring(drawIndexed.ActiveConditionList.size()));
            int activateNumber = 0;
            for (M_Condition condition : drawIndexed.ActiveConditionList) {
                //LOG.Info(condition.ConditionVarName);
                //这里的condition需要获取一下NameSapced KeyName
                std::wstring conditionNameSpacedKeyName = condition.NameSpace + L"\\" + condition.ConditionVarName.substr(1);
                if (condition.ConditionVarValue == KeyCombinationMap[conditionNameSpacedKeyName]) {
                    activateNumber++;
                }
                else {
                    break;
                }

            }
            if (activateNumber == drawIndexed.ActiveConditionList.size()) {
                activitedDrawIndexedList.push_back(drawIndexed);
                //LOG.Info(L"Activated");
            }
        }
       
    }
    //LOG.Error(L"Stop");
    return activitedDrawIndexedList;
}


void MigotoParseUtil_OutputIBFileByDrawIndexedList(
    std::wstring OriginalIBPath,std::wstring TargetIBPath,
    std::wstring IBReadFormat,std::vector<M_DrawIndexed> ActiveDrawIndexedList,
    int MinNumber) 
{

    std::wstring IBReadDxgiFormat = IBReadFormat;
    boost::algorithm::to_lower(IBReadDxgiFormat);
    std::wstring IBFilePath = OriginalIBPath;
    //TODO 需要一个获取IB文件最小值和最大值的方法，后面有用，但是这里只需要用到最小值
    int minNumber = MinNumber;


    int readLength = 2;
    if (IBReadDxgiFormat == L"dxgi_format_r16_uint") {
        readLength = 2;
    }
    if (IBReadDxgiFormat == L"dxgi_format_r32_uint") {
        readLength = 4;
    }
    std::ifstream ReadIBFile(IBFilePath, std::ios::binary);

    std::vector<uint16_t> IBR16DataList = {};
    std::vector<uint32_t> IBR32DataList = {};

    char* data = new char[readLength];

    while (ReadIBFile.read(data, readLength)) {
        if (IBReadDxgiFormat == L"dxgi_format_r16_uint") {
            std::uint16_t value = MMTFormat_CharArrayToUINT16_T(data);
            value = value - static_cast<std::uint16_t>(minNumber);
            IBR16DataList.push_back(value);

        }
        if (IBReadDxgiFormat == L"dxgi_format_r32_uint") {
            std::uint32_t value = MMTFormat_CharArrayToUINT32_T(data);
            value = value - static_cast<std::uint32_t>(minNumber);
            IBR32DataList.push_back(value);
        }
    }

    ReadIBFile.close();
    std::wstring outputIBFileName = TargetIBPath;

    if (IBReadDxgiFormat == L"dxgi_format_r16_uint") {
        IBR32DataList = std::vector<uint32_t>(IBR16DataList.size());
        std::transform(IBR16DataList.begin(), IBR16DataList.end(), IBR32DataList.begin(),
            [](uint16_t value) { return static_cast<uint32_t>(value); });
    }

    //这里要根据ActiveDrawIndexed来截取出需要写出的部分而不是所有的部分
    std::vector<uint32_t> Output_UINT32T_DataList;

    for (M_DrawIndexed drawIndexed : ActiveDrawIndexedList) {
        //LOG.Info(drawIndexed.DrawNumber);
        //LOG.Info(drawIndexed.DrawOffsetIndex);
        boost::algorithm::trim(drawIndexed.DrawNumber);
        boost::algorithm::trim(drawIndexed.DrawOffsetIndex);

        int drawNumber = std::stoi(drawIndexed.DrawNumber);
        int offsetNumber = std::stoi(drawIndexed.DrawOffsetIndex);
        LOG.Info(L"drawNumber" + std::to_wstring(drawNumber));
        LOG.Info(L"offsetNumber" + std::to_wstring(offsetNumber));

        std::vector<uint32_t> tmpList = MMTFormat_GetRange_UINT32T(IBR32DataList, offsetNumber, offsetNumber + drawNumber);
        Output_UINT32T_DataList.insert(Output_UINT32T_DataList.end(), tmpList.begin(), tmpList.end());
    }

    LOG.Info(L"Output_UINT32T_DataList Size: " + std::to_wstring(Output_UINT32T_DataList.size()));

    LOG.Info(L"IB file format: " + IBReadDxgiFormat);
    LOG.Info(L"IB file length: " + std::to_wstring(Output_UINT32T_DataList.size()));
    std::ofstream file(outputIBFileName, std::ios::binary);
    for (const auto& data : Output_UINT32T_DataList) {
        uint32_t paddedData = data;
        file.write(reinterpret_cast<const char*>(&paddedData), sizeof(uint32_t));
    }
    file.close();

}


std::wstring MigotoParseUtil_Get_M_Key_Combination_String(std::unordered_map<std::wstring, std::wstring> KeyCombinationMap) {
    std::wstring combinationStr;
    int count = 1;
    for (const auto& pair: KeyCombinationMap) {
        if (!MMTFile_IsValidFilename(MMTString_ToByteString(pair.first))) {
            //如果用文件名无法使用的字符来对抗逆向，那这里就使用数值代替
            combinationStr = combinationStr + L"$key" + std::to_wstring(count) + L"_";
        }
        else {
            combinationStr = combinationStr + L"$" + pair.first + L"_";
        }
        combinationStr = combinationStr + pair.second + L"_";
        count++;
    }


    //有些人会用文件名中不能出现的字符比如\ /来作为按键来对抗自动逆向，这时候根据UUID生成文件夹名称(不合适)。
    //if (!MMTFile_IsValidFilename(MMTString_ToByteString(combinationStr))) {
    //    combinationStr = MMTString_GenerateUUIDW();
    //}
    return combinationStr;

}


std::vector<std::wstring> MigotoParseUtil_GetRecursiveActivedIniFilePathList(std::wstring IncludePath) {
    std::vector<std::wstring> includeFilePathList = MMTFile_GetFilePathListRecursive(IncludePath);
    std::vector<std::wstring> parseIniFilePathList;
    for (const auto& filePath : includeFilePathList)
    {
        std::filesystem::path filePathObject(filePath);
        std::wstring fileName = filePathObject.filename().wstring();

        std::wstring lowerFileName = MMTString_ToLowerCase(fileName);
        if (lowerFileName.starts_with(L"disabled")) {
            continue;
        }
        if (!lowerFileName.ends_with(L".ini")) {
            continue;
        }
        parseIniFilePathList.push_back(filePath);
        //LOG.Info(filePath);
    }
    //LOG.NewLine();
    return parseIniFilePathList;
}


std::vector<M_SectionLine> MigotoParseUtil_ParseMigotoSectionLineList(std::wstring iniFilePath) {
    std::vector<M_SectionLine> migotoSectionLineList;

    //首先把这个ini文件的每一行读取到列表里
    std::vector<std::wstring> readLineList = MMTFile_ReadIniFileLineList(iniFilePath);
    //初始化默认NameSpace为当前文件所在目录
    std::wstring defaultNameSpace = MMTString_GetFolderPathFromFilePath(iniFilePath);
    //如果读取到了强行设置的NameSpace，就放到这里供后续使用
    std::wstring specifiedNameSpace = L"";

    //之前的设计是每调用一个方法，解析一种数据类型，现在我们直接按Section来，
    //读取每一行，判断是否为[开头，是就进入Section读取区域，否则就读取是否为namespace
    //然后读取到Section之后呢，先不处理，先把此section所有的行放到一个section对象里，然后所有section对象放到列表，等待后续处理。


    std::vector<std::wstring> tmpSectionLineList;
    M_SectionLine lastMigotoSectionLine;
    bool inSection = false;
    for (std::wstring readLine : readLineList) {
        //LOG.LogOutput(L"Parsing: " + readLine);
        std::wstring lowerReadLine = boost::algorithm::to_lower_copy(readLine);
        boost::algorithm::trim(lowerReadLine);
        //跳过注释
        if (lowerReadLine.starts_with(L";")) {
            continue;
        }

        //在读取解析ini文件时，在所有的Section读取之前如果读取到了namespace，则该ini文件所有的namespace都设置为指定的namespace
        if (!inSection && lowerReadLine.starts_with(L"namespace")) {
            std::vector<std::wstring> readLineSplitList = MMTString_SplitString(readLine, L"=");
            if (readLineSplitList.size() < 2) {
                LOG.Error(L"Invalid namespace assign.");
            }
            //namespace = xxx，我们只考虑出现一个等号的情况，所以这里索引固定为1
            std::wstring RightStr = readLineSplitList[1];
            //去除两边空格
            boost::algorithm::trim(RightStr);
            specifiedNameSpace = RightStr;
        }
        else if (lowerReadLine.starts_with(L"[")) {
            //每遇到一个[都说明遇到了新的section
            inSection = true;

            //遇到新的Section要把旧的Section加到总的Section列表里
            if (tmpSectionLineList.size() != 0) {
                lastMigotoSectionLine.SectionLineList = tmpSectionLineList;
                //LOG.LogOutput(L"Add Size: " + std::to_wstring(lastMigotoSectionLine.SectionLineList.size()));
                migotoSectionLineList.push_back(lastMigotoSectionLine);
                //然后清空当前的列表准备读取新的。
                tmpSectionLineList.clear();
            }

            //然后添加新的
            lastMigotoSectionLine = M_SectionLine();
            lastMigotoSectionLine.NameSpace = defaultNameSpace;
            if (specifiedNameSpace != L"") {
                lastMigotoSectionLine.NameSpace = specifiedNameSpace;
            }
            std::wstring sectionName = lowerReadLine.substr(1, lowerReadLine.length() - 2);
            LOG.Info(L"SectionName: " + sectionName + L" NameSpace: " + lastMigotoSectionLine.NameSpace);
            lastMigotoSectionLine.SectionName = sectionName;
            //别忘了把当前的SectionName的行也加入进去
            //行添加之前必须判断不为空
            if (lowerReadLine != L"") {
                tmpSectionLineList.push_back(readLine);
            }
        }
        else if (inSection) {
            if (lowerReadLine != L"") {
                tmpSectionLineList.push_back(readLine);
            }
        }

    }

    // 最后结尾时把最后一个Section的line也添加进去
    //遇到新的Section要把旧的Section加到总的Section列表里
    if (lastMigotoSectionLine.SectionLineList.size() != 0) {
        lastMigotoSectionLine.SectionLineList = tmpSectionLineList;
        //LOG.LogOutput(L"Add Size: " + std::to_wstring(lastMigotoSectionLine.SectionLineList.size()));
        migotoSectionLineList.push_back(lastMigotoSectionLine);
        //然后清空当前的列表准备读取新的。
        tmpSectionLineList.clear();
    }

    LOG.Info(L"MigotoSectionLineList Size: " + std::to_wstring(migotoSectionLineList.size()));
    LOG.NewLine();
    return migotoSectionLineList;
}