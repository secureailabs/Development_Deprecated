from PySide6.QtWidgets import *
from SafeObjectAPI import newguid, writeSafeObject
from functools import partial

typeList = ["<class 'bool'>",
"<class 'bytearray'>",
"<class 'bytes'>",
"<class 'complex'>",
"<class 'dict'>",
"<class 'enumerate'>",
"<class 'filter'>",
"<class 'float'>",
"<class 'frozenset'>",
"<class 'int'>",
"<class 'list'>",
"<class 'map'>",
"<class 'memoryview'>",
"<class 'object'>",
"<class 'property'>",
"<class 'range'>",
"<class 'reversed'>",
"<class 'set'>",
"<class 'slice'>",
"<class 'str'>",
"<class 'super'>",
"<class 'tuple'>",
"<class 'type'>",
"<class 'zip'>",
"<class 'numpy.bytes_'>",
"<class 'numpy.MachAr'>",
"<class 'numpy.str_'>",
"<class 'numpy.bool_'>",
"<class 'numpy.character'>",
"<class 'numpy.chararray'>",
"<class 'numpy.datetime64'>",
"<class 'numpy.dtype'>",
"<class 'numpy.finfo'>",
"<class 'numpy.flatiter'>",
"<class 'numpy.flexible'>",
"<class 'numpy.float128'>",
"<class 'numpy.float16'>",
"<class 'numpy.float32'>",
"<class 'numpy.float64'>",
"<class 'numpy.floating'>",
"<class 'numpy.format_parser'>",
"<class 'numpy.generic'>",
"<class 'numpy.iinfo'>",
"<class 'numpy.inexact'>",
"<class 'numpy.int64'>",
"<class 'numpy.int16'>",
"<class 'numpy.int32'>",
"<class 'numpy.int64'>",
"<class 'numpy.int8'>",
"<class 'numpy.integer'>",
"<class 'numpy.longlong'>",
"<class 'numpy.matrix'>",
"<class 'numpy.memmap'>",
"<class 'numpy.ndarray'>",
"<class 'numpy.ndenumerate'>",
"<class 'numpy.ndindex'>",
"<class 'numpy.nditer'>",
"<class 'numpy.number'>",
"<class 'numpy.object_'>",
"<class 'numpy.poly1d'>",
"<class 'numpy.recarray'>",
"<class 'numpy.record'>",
"<class 'numpy.signedinteger'>",
"<class 'numpy.timedelta64'>",
"<class 'numpy.uint8'>",
"<class 'numpy.uint64'>",
"<class 'numpy.uint16'>",
"<class 'numpy.uint32'>",
"<class 'numpy.ulonglong'>",
"<class 'numpy.unsignedinteger'>",
"<class 'numpy.vectorize'>",
"<class 'numpy.void'>",
"<class 'pandas.core.arrays.boolean.BooleanDtype'>",
"<class 'pandas.core.arrays.categorical.Categorical'>",
"<class 'pandas.core.dtypes.dtypes.CategoricalDtype'>",
"<class 'pandas.core.indexes.category.CategoricalIndex'>",
"<class 'pandas.core.frame.DataFrame'>",
"<class 'pandas._libs.tslibs.offsets.DateOffset'>",
"<class 'pandas.core.indexes.datetimes.DatetimeIndex'>",
"<class 'pandas.core.dtypes.dtypes.DatetimeTZDtype'>",
"<class 'pandas.core.arrays.floating.Float32Dtype'>",
"<class 'pandas.core.arrays.floating.Float64Dtype'>",
"<class 'pandas.core.indexes.numeric.Float64Index'>",
"<class 'pandas.core.groupby.grouper.Grouper'>",
"<class 'pandas.core.indexes.base.Index'>",
"<class 'pandas.core.arrays.integer.Int16Dtype'>",
"<class 'pandas.core.arrays.integer.Int32Dtype'>",
"<class 'pandas.core.arrays.integer.Int64Dtype'>",
"<class 'pandas.core.indexes.numeric.Int64Index'>",
"<class 'pandas.core.arrays.integer.Int8Dtype'>",
"<class 'pandas._libs.interval.Interval'>",
"<class 'pandas.core.dtypes.dtypes.IntervalDtype'>",
"<class 'pandas.core.indexes.interval.IntervalIndex'>",
"<class 'pandas.core.indexes.multi.MultiIndex'>",
"<class 'pandas._libs.tslibs.period.Period'>",
"<class 'pandas.core.dtypes.dtypes.PeriodDtype'>",
"<class 'pandas.core.indexes.period.PeriodIndex'>",
"<class 'pandas.core.indexes.range.RangeIndex'>",
"<class 'pandas.core.series.Series'>",
"<class 'pandas.core.arrays.sparse.dtype.SparseDtype'>",
"<class 'pandas.core.arrays.string_.StringDtype'>",
"<class 'pandas._libs.tslibs.timedeltas.Timedelta'>",
"<class 'pandas.core.indexes.timedeltas.TimedeltaIndex'>",
"<class 'pandas._libs.tslibs.timestamps.Timestamp'>",
"<class 'pandas.core.arrays.integer.UInt16Dtype'>",
"<class 'pandas.core.arrays.integer.UInt32Dtype'>",
"<class 'pandas.core.arrays.integer.UInt64Dtype'>",
"<class 'pandas.core.indexes.numeric.UInt64Index'>",
"<class 'pandas.core.arrays.integer.UInt8Dtype'>",
"<class 'xgboost.core.Booster'>",
"<class 'xgboost.core.DMatrix'>",
"<class 'xgboost.sklearn.XGBClassifier'>",
"<class 'xgboost.sklearn.XGBModel'>",
"<class 'xgboost.sklearn.XGBRFClassifier'>",
"<class 'xgboost.sklearn.XGBRFRegressor'>",
"<class 'xgboost.sklearn.XGBRanker'>",
"<class 'xgboost.sklearn.XGBRegressor'>",
"<class 'sklearn.preprocessing._data.Binarizer'>",
"<class 'sklearn.preprocessing._function_transformer.FunctionTransformer'>",
"<class 'sklearn.preprocessing._discretization.KBinsDiscretizer'>",
"<class 'sklearn.preprocessing._data.KernelCenterer'>",
"<class 'sklearn.preprocessing._label.LabelBinarizer'>",
"<class 'sklearn.preprocessing._label.LabelEncoder'>",
"<class 'sklearn.preprocessing._data.MaxAbsScaler'>",
"<class 'sklearn.preprocessing._data.MinMaxScaler'>",
"<class 'sklearn.preprocessing._label.MultiLabelBinarizer'>",
"<class 'sklearn.preprocessing._data.Normalizer'>",
"<class 'sklearn.preprocessing._encoders.OneHotEncoder'>",
"<class 'sklearn.preprocessing._encoders.OrdinalEncoder'>",
"<class 'sklearn.preprocessing._data.PolynomialFeatures'>",
"<class 'sklearn.preprocessing._data.PowerTransformer'>",
"<class 'sklearn.preprocessing._data.QuantileTransformer'>",
"<class 'sklearn.preprocessing._data.RobustScaler'>",
"<class 'sklearn.preprocessing._data.StandardScaler'>",
"<class 'collections.Counter'>",
"<class '_io.BytesIO'>",
"<class '_io.StringIO'>"]

class SafeObjectEditor(QDialog):

    def __init__(self, parent=None):
        super(SafeObjectEditor, self).__init__(parent)

        self.title = 0
        self.description = 0
        self.safeObjectGuid = newguid()

        top_layout = QVBoxLayout()

        self.titleArea = self.addTitle()
        top_layout.addLayout(self.titleArea)       
        
        parameter_layout = QHBoxLayout()
        self.inputParameterArea = ParameterWidget("Input parameters")
        self.outputParameterArea = ParameterWidget("Output parameters")
        parameter_layout.addLayout(self.inputParameterArea)
        parameter_layout.addLayout(self.outputParameterArea)
        top_layout.addLayout(parameter_layout)

        self.plainTextArea = self.addCode()
        codeLabel = QLabel("Safe Object Script")
        top_layout.addWidget(codeLabel)
        top_layout.addWidget(self.plainTextArea)

        self.buttonArea = self.addButton()
        top_layout.addLayout(self.buttonArea)

        self.setWindowTitle("SAIL SafeObject Editor")
        self.setLayout(top_layout)
        self.resize(600, 600)
    
    def addTitle(self):
        result = QVBoxLayout()
        titleLine = QLineEdit()
        titleLabel = QLabel("Safe Object Title")
        titleLine.setPlaceholderText("Safe Object Title")
        titleLine.editingFinished.connect(self.setSOTitle)
        desLine = QLineEdit()
        desLabel = QLabel("Safe Object Description")
        desLine.setPlaceholderText("Safe Object Description")
        desLine.editingFinished.connect(self.setSODescription)
        result.addWidget(titleLabel)
        result.addWidget(titleLine)
        result.addWidget(desLabel)
        result.addWidget(desLine)
        return result
    
    def setSOTitle(self):
        text = self.titleArea.itemAt(1).widget().text()
        self.title = text
    
    def setSODescription(self):
        text = self.titleArea.itemAt(3).widget().text()
        self.description = text
    
    def addCode(self):
        result = QPlainTextEdit()
        result.setPlaceholderText("Please input the script for the safe object")
        return result
    
    def addButton(self):
        result = QHBoxLayout()
        exitButton = QPushButton("Quit")
        exitButton.clicked.connect(self.handleQuit)
        confirmButton = QPushButton("Confirm")
        confirmButton.clicked.connect(self.handleConfirm)
        result.addWidget(exitButton)
        result.addWidget(confirmButton)
        return result
    
    def handleQuit(self):
        self.close()
    
    def handleConfirm(self):
        resultDict = {}

        resultDict["Title"] = self.title
        resultDict["Description"] = self.description
        print(self.description)
        resultDict["uuid"] = self.safeObjectGuid

        strCode = self.plainTextArea.toPlainText()
        resultDict["script"] = strCode

        inputTableLen = self.inputParameterArea.tableLen
        outputTableLen = self.outputParameterArea.tableLen
        resultDict["numberOfInput"] = inputTableLen
        resultDict["numberOfOutput"] = outputTableLen

        inputParams = []
        for i in range(inputTableLen):
            tmpTableArr = []
            tmpTableArr.append(newguid())
            for j in range(4):
                tmpTableArr.append(self.inputParameterArea.getTableContent(i, j))
            inputParams.append(tmpTableArr)
        resultDict["input"] = inputParams
        print(inputParams)
        
        outputParams = []
        for i in range(outputTableLen):
            tmpTableArr = []
            tmpTableArr.append(newguid())
            for j in range(4):
                tmpTableArr.append(self.outputParameterArea.getTableContent(i, j))
            outputParams.append(tmpTableArr)
        resultDict["output"] = outputParams
        print(outputParams)

        self.generateSafeObject(resultDict)
    
    def generateSafeObject(self, vardict):
        f = open("SafeObjectTemplate", "r")
        template = f.read()

        codeScript = vardict["script"]
        codeScript = codeScript.replace("\n", "\n        ")
        template = template.replace("{{code}}", codeScript)
        template = template.replace("{{safeObjectId}}", self.safeObjectGuid)

        for i in range(vardict["numberOfInput"]):
            strParamGuid = vardict["input"][i][0]
            template = template.replace(vardict["input"][i][1], "self.m_"+strParamGuid)
            strStringToGetParameterFromFile = "with open(oInputParameters[\""+ strParamGuid +"\"][\"0\"], 'rb') as ifp:\n            self.m_"+ strParamGuid + " = pickle.load(ifp)"
            if (i != (vardict["numberOfInput"]-1)):
                strStringToGetParameterFromFile += "\n        {{ParamterSet}}\n"
            template = template.replace("{{ParamterSet}}", strStringToGetParameterFromFile)

        for i in range(vardict["numberOfOutput"]):
            strOutputParamterGuid = vardict["output"][i][0]
            template = template.replace(vardict["output"][i][1], "self.m_" + strOutputParamterGuid)
            strStringToSetParameterFile = "with open(self.m_JobIdentifier+\"." + strOutputParamterGuid + "\",\"wb\") as ofp:\n            pickle.dump(self.m_" + strOutputParamterGuid + ", ofp)\n        with open(\"DataSignals/\" + self.m_JobIdentifier + \"." + strOutputParamterGuid + "\", 'w') as fp:\n            pass"
            if (i != (vardict["numberOfOutput"]-1)):
                strStringToSetParameterFile += "\n        {{WriteOutputToFile}}\n"
            template = template.replace("{{WriteOutputToFile}}", strStringToSetParameterFile)
        
        vardict["body"] = template
        #print(template)
        writeSafeObject(vardict)
        f.close()

class ParameterWidget(QVBoxLayout):
    def __init__(self, title, parent = None):
        super(ParameterWidget, self).__init__(parent)

        self.parameterBox = QLineEdit()
        lineLabel = QLabel("Number of "+title)
        self.parameterBox.setPlaceholderText("please set "+title +" number")
        self.parameterBox.editingFinished.connect(self.setTableWidget)
        self.addWidget(lineLabel)
        self.addWidget(self.parameterBox)
        self.table = 0
        self.tableLen = 0     
        
    def setTableWidget(self):
        self.table = QTableWidget()
        tableSize = self.parameterBox.text()
        self.tableLen = int(tableSize)
        self.table.setColumnCount(4)
        self.table.setHorizontalHeaderLabels(["Symbol", "Type", "Description", "Confidentiality"])
        self.table.setRowCount(self.tableLen)

        for i in range(self.tableLen):
            dropdown = QComboBox()
            dropdown.addItems(typeList)
            #dropdown.activated.connect(lambda i=i : self.setCellValue(i,1))
            self.table.setCellWidget(i, 1, dropdown)

        self.addWidget(self.table)
    
    # def setCellValue(self, i ,j):
    #     text = self.table.cellWidget(i, j).currentText()
    #     self.table.item(i, j).setText(text)
    
    def getTableContent(self, i, j):
        widget = self.table.cellWidget(i,j)
        text = 0
        if isinstance(widget, QComboBox):
            text = self.table.cellWidget(i, j).currentText()
        else:
            text = self.table.item(i,j).text()
        return text
    
