from StructuredBuffer import StructuredBuffer
from flask import Flask, request
import json

app = Flask(__name__)

def GetJsonFromSerializedBuffer(serializedStructureBuffer):
    oSb = StructuredBuffer(serializedBuffer=serializedStructureBuffer)
    print(oSb.ToString())
    strJson = oSb.ToJson()
    return strJson

def GetSerializedBufferFromJson(strJson):
    oSb = StructuredBuffer(strJson=strJson)
    return oSb.GetSerializedStructuredBuffer()

@app.route('/StructuredBufferToJson', methods=['GET'])
def StructuredBufferToJson():
    return GetJsonFromSerializedBuffer(request.get_data())

@app.route('/JsonToStructuredBuffer', methods=['GET'])
def JsonToStructuredBuffer():
    return GetSerializedBufferFromJson(request.get_data())

