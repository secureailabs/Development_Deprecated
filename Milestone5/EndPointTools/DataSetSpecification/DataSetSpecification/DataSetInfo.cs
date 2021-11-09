using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using RestSharp;
using Newtonsoft.Json;

namespace DataSetSpecification
{
    public partial class DataSetInfo : UserControl
    {
        private FormLogin loginPage;
        public FormMain m_firstPage;

        public DataSetInfo()
        {
            InitializeComponent();
        }

        public void GetAuth()
        {
            // Get the final file location and name from the user to write the data
            if ("" != loginPage.m_authorizationTokken)
            {
                string authToken = loginPage.m_authorizationTokken;
                Console.WriteLine(authToken);
                // Open the file and start filling it up
                string finalFileName = textBoxOutputFile.Text;
                // Each UserControl1 has a TableInfo Object which will have all the data related to the file that we need. 
                // Iterate through all the UserControl1 objects and fetch their TableInfo object and get all the data.
                {
                    FileStream newFile = File.Open(finalFileName, FileMode.Create);
                    BinaryWriter outputFile = new BinaryWriter(newFile);

                    // Gather all the tables and change them to CSV let 
                    FlowLayoutPanel flowPanel = (FlowLayoutPanel)m_firstPage.Controls.Find("FlowLayoutPanelTables", true)[0];
                    Control[] listTableUserControl = flowPanel.Controls.Find("TableInfo", true);
                    int numberOfTables = listTableUserControl.Length;

                    // Header Marker Constant to mark the start of the file
                    const UInt64 markerHeaderFooter = 0xDEADBEEFDEADBEEF;

                    //Create a Structured Buffer with dummy values to get the header size hold up the header 
                    StructuredBuffer oHeaderStructuredBuffer = new StructuredBuffer();
                    oHeaderStructuredBuffer.PutUnsignedInt32("Version", 1);
                    oHeaderStructuredBuffer.PutUnsignedInt64("MetaDataOffset", 1);
                    oHeaderStructuredBuffer.PutUnsignedInt32("MetaDataSize", 1);
                    // Put a dummy string of signed hash size (96bytes hash is a 96*4/3=128byte string)
                    oHeaderStructuredBuffer.PutString("SignedHash", new String('a', 128));

                    UInt64[] tabledataOffset = new UInt64[numberOfTables];
                    UInt64[] metadataOffset = new UInt64[numberOfTables];
                    UInt64[] metadatSize = new UInt64[numberOfTables];

                    Byte[] tableAndMetadataBuffer = { };

                    // tablesMetaData to store the metadata for each table
                    StructuredBuffer[] tablesMetaData = new StructuredBuffer[numberOfTables];
                    int i = 0;
                    // Offset of Header Marker  + size of Header variable + size of Header Structure + size of ECDSA hash signed with 48byte private key
                    UInt64 currentTableOffset = (UInt64)sizeof(UInt64) + (UInt64)sizeof(UInt32) + (UInt64)oHeaderStructuredBuffer.GetSerializedBufferRawDataSizeInBytes() + (UInt64)128;

                    foreach (Control userControl in listTableUserControl)
                    {
                        TableInfo oUserControl1 = (TableInfo)userControl;
                        string table = oUserControl1.GetTableAsCSVString();

                        // Writing the tables to the buffer
                        tableAndMetadataBuffer = tableAndMetadataBuffer.Concat(Encoding.ASCII.GetBytes(table)).ToArray();

                        // Populate the Structured Buffer
                        tablesMetaData[i] = new StructuredBuffer();
                        tablesMetaData[i].PutGuid("Guid", oUserControl1.m_tableUUID);
                        tablesMetaData[i].PutString("Name", oUserControl1.m_tableName);
                        tablesMetaData[i].PutString("Description", oUserControl1.m_tableDescription);
                        tablesMetaData[i].PutString("Hashtags", oUserControl1.m_tableTags);
                        tablesMetaData[i].PutInt32("NumberColumns", (int)oUserControl1.m_numberOfColumns);
                        tablesMetaData[i].PutInt32("NumberRows", (int)oUserControl1.m_numberOfRows);
                        tablesMetaData[i].PutString("ColumnName", oUserControl1.m_columnNames);
                        tablesMetaData[i].PutString("ColumnType", oUserControl1.GetColumnType());
                        tablesMetaData[i].PutString("ColumnSensitivity", oUserControl1.GetColumnSensitivity());
                        tablesMetaData[i].PutUnsignedInt64("TableOffset", currentTableOffset);

                        tabledataOffset[i] = currentTableOffset;
                        metadatSize[i] = (UInt64)tablesMetaData[i].GetSerializedBufferRawDataSizeInBytes();
                        currentTableOffset += (UInt64)table.Length;
                        i++;

                        System.GC.Collect();
                    }

                    Byte[] byteArray;
                    for (i = 0; i < numberOfTables; i++)
                    {
                        metadataOffset[i] = currentTableOffset;
                        currentTableOffset += metadatSize[i];

                        // table Metadata
                        byteArray = new Byte[tablesMetaData[i].GetSerializedBufferRawDataSizeInBytes()];
                        Marshal.Copy(tablesMetaData[i].GetSerializedBufferRawDataPtr(), byteArray, 0, tablesMetaData[i].GetSerializedBufferRawDataSizeInBytes());

                        // Writing the table MetaData to the buffer
                        tableAndMetadataBuffer = tableAndMetadataBuffer.Concat(byteArray).ToArray();

                        System.GC.Collect();
                    }

                    // dataset Metadata StructuredBuffer
                    StructuredBuffer oMetadataStructuredBuffer = new StructuredBuffer();
                    oMetadataStructuredBuffer.PutString("UUID", label5.Text);
                    oMetadataStructuredBuffer.PutInt64("Timestamp", DateTimeOffset.UtcNow.ToUnixTimeSeconds());
                    oMetadataStructuredBuffer.PutString("Name", textBox2.Text);
                    oMetadataStructuredBuffer.PutString("Description", textBox4.Text);
                    oMetadataStructuredBuffer.PutString("Tags", textBox3.Text);
                    oMetadataStructuredBuffer.PutInt32("NumberTables", numberOfTables);
                    oMetadataStructuredBuffer.PutUInt64Buffer("OffsetTables", metadataOffset, (uint)numberOfTables);
                    oMetadataStructuredBuffer.PutUInt64Buffer("SizeTables", metadatSize, (uint)numberOfTables);

                    // Writing the DataSet MetaData to the tableAndMetadataBuffer
                    byteArray = new Byte[oMetadataStructuredBuffer.GetSerializedBufferRawDataSizeInBytes()];
                    Marshal.Copy(oMetadataStructuredBuffer.GetSerializedBufferRawDataPtr(), byteArray, 0, oMetadataStructuredBuffer.GetSerializedBufferRawDataSizeInBytes());
                    tableAndMetadataBuffer = tableAndMetadataBuffer.Concat(byteArray).ToArray();

                    // Signing the tables + table Metadata + DataSet Metadata 

                    // The key array is a combination of
                    // 1. Magic uint 0x34534345 in big-endian
                    // 2. Size of private key (0x30 or 48bytes here)
                    // 3. X cordinate of public key
                    // 4. Y cordinate of public key
                    // 5. D private key
                    byte[] cngKeyImportArray = { 0x45, 0x43, 0x53, 0x34, 0x30, 0x00, 0x00, 0x00, 0xAA, 0x87, 0xCA, 0x22, 0xBE, 0x8B, 0x05, 0x37, 0x8E, 0xB1, 0xC7, 0x1E, 0xF3, 0x20, 0xAD, 0x74, 0x6E, 0x1D, 0x3B, 0x62, 0x8B, 0xA7, 0x9B, 0x98, 0x59, 0xF7, 0x41, 0xE0, 0x82, 0x54, 0x2A, 0x38, 0x55, 0x02, 0xF2, 0x5D, 0xBF, 0x55, 0x29, 0x6C, 0x3A, 0x54, 0x5E, 0x38, 0x72, 0x76, 0x0A, 0xB7, 0x36, 0x17, 0xDE, 0x4A, 0x96, 0x26, 0x2C, 0x6F, 0x5D, 0x9E, 0x98, 0xBF, 0x92, 0x92, 0xDC, 0x29, 0xF8, 0xF4, 0x1D, 0xBD, 0x28, 0x9A, 0x14, 0x7C, 0xE9, 0xDA, 0x31, 0x13, 0xB5, 0xF0, 0xB8, 0xC0, 0x0A, 0x60, 0xB1, 0xCE, 0x1D, 0x7E, 0x81, 0x9D, 0x7A, 0x43, 0x1D, 0x7C, 0x90, 0xEA, 0x0E, 0x5F, 0x4D, 0x2D, 0xFE, 0x18, 0x01, 0x45, 0x79, 0x35, 0x52, 0x15, 0x08, 0xDF, 0x22, 0xA0, 0x8C, 0x55, 0x1D, 0xEC, 0xAA, 0x51, 0x41, 0x4C, 0xB8, 0x2C, 0x24, 0x96, 0xB3, 0x76, 0x9A, 0x96, 0x80, 0xBC, 0x29, 0x52, 0xE3, 0xC2, 0x2E, 0x66, 0x97, 0x78, 0x75, 0xDF, 0x45, 0x9F, 0xAF, 0x9D, 0xA7, 0x3D };

                    // Calculate hash and sign it with the above private key
                    SHA256Managed hash = new SHA256Managed();
                    byte[] tableAndMetadataBufferHash = hash.ComputeHash(tableAndMetadataBuffer);
                    Console.WriteLine("hashedData " + Convert.ToBase64String(tableAndMetadataBufferHash));
                    CngKey cngKey = CngKey.Import(cngKeyImportArray, CngKeyBlobFormat.EccPrivateBlob);
                    ECDsaCng dsa = new ECDsaCng(cngKey)
                    {
                        HashAlgorithm = CngAlgorithm.Sha256
                    };
                    byte[] tableAndMetadataBufferSignedHash = dsa.SignHash(tableAndMetadataBufferHash);
                    Console.WriteLine("signedHash " + Convert.ToBase64String(tableAndMetadataBufferSignedHash));

                    oHeaderStructuredBuffer.PutUnsignedInt64("MetaDataOffset", currentTableOffset);
                    oHeaderStructuredBuffer.PutInt32("MetaDataSize", oMetadataStructuredBuffer.GetSerializedBufferRawDataSizeInBytes());
                    // Put a dummy string of signed hash size (96bytes hash is a 96*4/3=128byte string)
                    oHeaderStructuredBuffer.PutString("SignedHash", Convert.ToBase64String(tableAndMetadataBufferSignedHash));

                    byteArray = new Byte[oHeaderStructuredBuffer.GetSerializedBufferRawDataSizeInBytes()];
                    Marshal.Copy(oHeaderStructuredBuffer.GetSerializedBufferRawDataPtr(), byteArray, 0, oHeaderStructuredBuffer.GetSerializedBufferRawDataSizeInBytes());
                    byte[] HeaderStructuredBufferHash = hash.ComputeHash(byteArray);
                    byte[] HeaderStructuredBufferSignedHash = dsa.SignHash(HeaderStructuredBufferHash);

                    // Start Writing to the file
                    outputFile.Write(markerHeaderFooter);

                    // Size of the header
                    outputFile.Write(oHeaderStructuredBuffer.GetSerializedBufferRawDataSizeInBytes());

                    // Header Structured Buffer
                    outputFile.Write(byteArray);

                    // Signed Hash of the Structured Buffer
                    outputFile.Write(Encoding.ASCII.GetBytes(Convert.ToBase64String(HeaderStructuredBufferSignedHash)));

                    // Tables + MetaData
                    outputFile.Write(tableAndMetadataBuffer);

                    // End Of File Marker
                    outputFile.Write(markerHeaderFooter);

                    // Close the file
                    outputFile.Close();

                    // If the dataset has to be registered on the portal 
                    if (true == loginPage.m_isDatasetToBeRegistered)
                    {
                        var client = new RestClient("https://" + loginPage.m_strWebPortalUrl + ":6200/SAIL/DatasetManager/RegisterDataset?Eosb=" + authToken)
                        {
                            // ************* VERY VERY IMPORTANT ********************************************/
                            // TODO: this is temporary to ignore the certificate validation for the rest call 
                            // but should be removed in production.
                            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => true
                        };
                        client.Timeout = -1;
                        string strContent = "{\n   \"DatasetGuid\": \"" + label5.Text + "\"," +
                                "\n   \"DatasetData\": {" +
                                "\n   \"VersionNumber\": \"" + "0.1" + "\"," +
                                "\n   \"DatasetName\": \"" + textBox2.Text + "\"," +
                                "\n   \"Description\": \"" + textBox4.Text + "\"," +
                                "\n   \"Keywords\": \"" + textBox3.Text + "\"," +
                                "\n   \"PublishDate\": " + DateTimeOffset.UtcNow.ToUnixTimeSeconds().ToString() + "," + 
                                "\n   \"PrivacyLevel\": " + "1" + "," +
                                "\n   \"JurisdictionalLimitations\": \"" + "US,EU,AUS" + "\"" +
                                "\n   }" + 
                                "\n}";
                        var request = new RestRequest(Method.POST);
                        request.AddHeader("Content-Type", "application/json");
                        request.AddParameter("application/json", strContent, ParameterType.RequestBody);
                        IRestResponse response = client.Execute(request);
                        dynamic responseJsonObject = JsonConvert.DeserializeObject(response.Content);
                        Console.WriteLine("Json is "+responseJsonObject);
                    }
                }
            }
            else
            {
                MessageBox.Show("Login Error!!", "Error");
            }
            MessageBox.Show("Successfully created Dataset. Exiting!!", "Success");
            System.Windows.Forms.Application.Exit();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            // Verify all the data and throw error if something is missing
            if ("" == textBoxOutputFile.Text)
            {
                MessageBox.Show("Invalid File Name!!", "Error");
                return;
            }
            // Pop-up Login and get credentials. Store them in as a private member in case they don't already exist
            loginPage = new FormLogin
            {
                m_container = this
            };
            Console.WriteLine(loginPage.ToString());
            loginPage.Show();
        }

        private void FullDataInfo_Load(object sender, EventArgs e)
        {
            label5.Text = System.Guid.NewGuid().ToString();
        }

        private void buttonOutputFile_Click(object sender, EventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog
            {
                Filter = "CSVP | *.csvp",
            };
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                textBoxOutputFile.Text = dialog.FileName;
            }
        }
    }
}
