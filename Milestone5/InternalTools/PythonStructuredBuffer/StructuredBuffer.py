import PyStructuredBuffer
import ctypes
import re
import json

def PyGenerateParamterGuid():
    return PyStructuredBuffer.GenerateParamterGuid()

class StructuredBuffer():
    m_poStructuredBuffer = ctypes.c_void_p()
    def __init__(self, poStructuredBuffer=0, strFileName="", strJson="", serializedBuffer=b''):
        if (0 != len(serializedBuffer)):
            self.m_poStructuredBuffer = PyStructuredBuffer.FromSerializedBuffer(serializedBuffer)
        elif (0 != len(strJson)):
            self.m_poStructuredBuffer = PyStructuredBuffer.Init()
            self.FromJson(strJson)
        elif (0 != len(strFileName)):
            self.m_poStructuredBuffer = PyStructuredBuffer.LoadStructuredBuffer(strFileName)
        elif (0 != poStructuredBuffer):
            self.m_poStructuredBuffer = poStructuredBuffer
        else:
            self.m_poStructuredBuffer = PyStructuredBuffer.Init()

    def __del__(self):
        # print("self.m_poStructuredBuffer", self.m_poStructuredBuffer, type(self.m_poStructuredBuffer))
        PyStructuredBuffer.Delete(self.m_poStructuredBuffer)
        self.m_poStructuredBuffer = ctypes.c_void_p()

    # def Clear(self):
    #     pass

    # def GetSerializedBuffer(self):
    #     pass

    # def IsElementPresent(self, c_szElementName, bElementType):
    #     pass

    # def RemoveElement(self, c_szElementName):
    #     pass

    # def GetNamesOfElements(self):
    #     PyStructuredBuffer.GetNamesOfElements(self.m_poStructuredBuffer)

    def GetDescriptionOfElements(self):
        return PyStructuredBuffer.GetDescriptionOfElements(self.m_poStructuredBuffer)

    def PutString(self, c_szElementName, c_szValue):
        PyStructuredBuffer.PutString(self.m_poStructuredBuffer, c_szElementName, c_szValue)

    def PutInt32(self, c_szElementName, n32Value):
        PyStructuredBuffer.PutInt(self.m_poStructuredBuffer, c_szElementName, n32Value)

    # def PutFloat32(self, c_szElementName, fl32Value):
    #     pass

    def PutFloat64(self, c_szElementName, fl64Value):
        PyStructuredBuffer.PutFloat64(self.m_poStructuredBuffer, c_szElementName, fl64Value)

    # def PutInt8(self, c_szElementName, n8Value):
    #     pass

    # def PutInt16(self, c_szElementName, n16Value):
    #     pass

    # def GetNull(self, c_szElementName):
    #     pass

    def GetBoolean(self, c_szElementName):
        return PyStructuredBuffer.GetBoolean(self.m_poStructuredBuffer, c_szElementName)

    # def GetCharacter(self, c_szElementName):
    #     pass

    def GetString(self, c_szElementName):
        return PyStructuredBuffer.GetString(self.m_poStructuredBuffer, c_szElementName)

    # def GetFloat32(self, c_szElementName):
    #     pass

    def GetFloat64(self, c_szElementName):
        return PyStructuredBuffer.GetFloat64(self.m_poStructuredBuffer, c_szElementName)

    # def GetInt8(self, c_szElementName):
    #     pass

    # def GetInt16(self, c_szElementName):
    #     pass

    # def GetInt32(self, c_szElementName):
    #     pass

    # def GetInt64(self, c_szElementName):
    #     pass

    # def GetUnsignedInt8(self, c_szElementName):
    #     pass

    # def GetUnsignedInt16(self, c_szElementName):
    #     pass

    # def GetUnsignedInt32(self, c_szElementName):
    #     pass

    # def GetUnsignedInt64(self, c_szElementName):
    #     pass

    # def GetByte(self, c_szElementName):
    #     pass

    # def GetWord(self, c_szElementName):
    #     pass

    # def GetDword(self, c_szElementName):
    #     pass

    # def GetQword(self, c_szElementName):
    #     pass

    # def GetGuid(self, c_szElementName):
    #     pass

    # def GetBuffer(self, c_szElementName):
    #     pass

    def GetStructuredBuffer(self, c_szElementName):
        return PyStructuredBuffer.GetStructuredBuffer(self.m_poStructuredBuffer, c_szElementName)

    # def PutNull(self, c_szElementName):
    #     pass

    def PutBoolean(self, c_szElementName, fValue):
        PyStructuredBuffer.PutBoolean(self.m_poStructuredBuffer, c_szElementName, fValue)

    # def PutCharacter(self, c_szElementName, chValue):
    #     pass

    # def PutInt64(self, c_szElementName, n64Value):
    #     pass

    # def PutUnsignedInt8(self, c_szElementName, un8Value):
    #     pass

    # def PutUnsignedInt16(self, c_szElementName, un16Value):
    #     pass

    # def PutUnsignedInt32(self, c_szElementName, un32Value):
    #     pass

    # def PutUnsignedInt64(self, c_szElementName, un64Value):
    #     pass

    # def PutByte(self, c_szElementName, bValue):
    #     pass

    # def PutWord(self, c_szElementName, wValue):
    #     pass

    # def PutDword(self, c_szElementName, dwValue):
    #     pass

    # def PutQword(self, c_szElementName, qwValue):
    #     pass

    # def PutGuid(self, c_szElementName, c_oGuid):
    #     pass

    # def PutBuffer(self, c_szElementName, c_pbByteArray, unSizeInBytes):
    #     pass

    # def PutBuffer(self, c_szElementName, c_stlBuffer):
    #     pass

    def GetSerializedStructuredBuffer(self):
        return PyStructuredBuffer.GetSerializedStructuredBuffer(self.m_poStructuredBuffer)

    def PutStructuredBuffer(self, c_szElementName, c_oStructuredBuffer):
        PyStructuredBuffer.PutStructuredBuffer(self.m_poStructuredBuffer, c_szElementName, c_oStructuredBuffer.m_poStructuredBuffer)

    def ToString(self):
        return PyStructuredBuffer.ToString(self.m_poStructuredBuffer)

    def Dump(self, c_szFilename):
        return PyStructuredBuffer.DumpStructuredBuffer(self.m_poStructuredBuffer, c_szFilename)

    def ToJson(self):
        oJson = {}
        for strElement in self.GetDescriptionOfElements():
            strElementName = re.search('Name\[(.*?)\]', strElement).group(1)
            nElementType = re.search('Type\[(.*?)\]', strElement).group(1)
            strElementType = PyStructuredBuffer.GetElementTypeStringFromInt(int(nElementType))
            if strElementType == 'Bool':
                oJson[strElementName] = self.GetBoolean(strElementName)
            elif strElementType == 'String':
                oJson[strElementName] = self.GetString(strElementName)
            elif strElementType == 'float64_t':
                oJson[strElementName] = self.GetFloat64(strElementName)
            elif strElementType == 'StructuredBuffer':
                oNew = StructuredBuffer(poStructuredBuffer=self.GetStructuredBuffer(strElementName))
                oJson[strElementName] = oNew.ToJson()
            else:
                print('Unknown type')
        return oJson

    def FromJson(self, strJson):
        oJson = json.loads(strJson)
        for strElementName in oJson:
            if type(oJson[strElementName]) is bool:
                self.PutBoolean(strElementName, oJson[strElementName])
            elif type(oJson[strElementName]) is str:
                self.PutString(strElementName, oJson[strElementName])
            elif type(oJson[strElementName]) is float:
                self.PutFloat64(strElementName, oJson[strElementName])
            elif type(oJson[strElementName]) is int:
                self.PutFloat64(strElementName, oJson[strElementName])
            elif type(oJson[strElementName]) is dict:
                oNew = StructuredBuffer()
                oNew.FromJson(json.dumps(oJson[strElementName]))
                self.PutStructuredBuffer(strElementName, oNew)
            else:
                print('Unknown type')
