using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;

namespace MicrosoftAzureVirtualMachineInitializer
{
    public class InitializerSettings
    {
        /// <summary>
        /// Constructor used when instantiating InitializerSettings for
        /// manual configuration
        /// </summary>
        public InitializerSettings(
            bool initializerOnly,
            bool isConfidential
            )
        {
            m_ListOfVirtualMachines = new Dictionary<uint, MicrosoftAzureVirtualMachine>();
            if (false == initializerOnly)
            {
                if (true == isConfidential)
                {
                    ConfidentialVirtualMachineManualSettingsDialog confidentialVmSettingsDialog = new ConfidentialVirtualMachineManualSettingsDialog();
                    if (System.Windows.Forms.DialogResult.OK == confidentialVmSettingsDialog.ShowDialog())
                    {
                        string clusterIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
                        string datasetIdentifier = SailWebApiPortalInterop.GetDigitalContractProperty(confidentialVmSettingsDialog.DigitalContractIdentifier, "DatasetGuid");
                        for (uint index = 0; index < confidentialVmSettingsDialog.VirtualMachineCount; index++)
                        {
                            m_ListOfVirtualMachines.Add(index, new MicrosoftAzureVirtualMachine(clusterIdentifier, confidentialVmSettingsDialog.DigitalContractIdentifier, datasetIdentifier, confidentialVmSettingsDialog.DatasetFilename, SailWebApiPortalInterop.GetIpAddress(), confidentialVmSettingsDialog.AzureSubscriptionIdentifier, confidentialVmSettingsDialog.AzureResourceGroup, confidentialVmSettingsDialog.AzureLocation, confidentialVmSettingsDialog.AzureVirtualNetwork, confidentialVmSettingsDialog.AzureNetworkSecurityGroup, confidentialVmSettingsDialog.AzureVmOsDiskUrl, confidentialVmSettingsDialog.AzureVmOsVgmsDiskUrl, confidentialVmSettingsDialog.AzureOsDiskStorageAccount, confidentialVmSettingsDialog.AzureVirtualMachineSize));
                        }
                        m_IsConfigured = true;
                    }
                    else
                    {
                        m_IsConfigured = false;
                    }
                }
                else
                {
                    ManualSettingsDialog manualSettingsDialog = new ManualSettingsDialog();
                    if (System.Windows.Forms.DialogResult.OK == manualSettingsDialog.ShowDialog())
                    {
                        string clusterIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
                        string datasetIdentifier = SailWebApiPortalInterop.GetDigitalContractProperty(manualSettingsDialog.DigitalContractIdentifier, "DatasetGuid");
                        for (uint index = 0; index < manualSettingsDialog.VirtualMachineCount; index++)
                        {
                            m_ListOfVirtualMachines.Add(index, new MicrosoftAzureVirtualMachine(clusterIdentifier, manualSettingsDialog.DigitalContractIdentifier, datasetIdentifier, manualSettingsDialog.DatasetFilename, SailWebApiPortalInterop.GetIpAddress(), manualSettingsDialog.AzureSubscriptionIdentifier, manualSettingsDialog.AzureResourceGroup, manualSettingsDialog.AzureLocation, manualSettingsDialog.OsDiskUrl, manualSettingsDialog.AzureNetworkSecurityGroup, manualSettingsDialog.OsDiskStorageAccount, manualSettingsDialog.AzureVirtualMachineSize));
                        }
                        m_IsConfigured = true;
                    }
                    else
                    {
                        m_IsConfigured = false;
                    }
                }
            }
            else
            {
                InitializerOnlySettingsDialog initializerOnlySettingsDialog = new InitializerOnlySettingsDialog();
                if (System.Windows.Forms.DialogResult.OK == initializerOnlySettingsDialog.ShowDialog())
                {
                    string clusterIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
                    string datasetIdentifier = SailWebApiPortalInterop.GetDigitalContractProperty(initializerOnlySettingsDialog.DigitalContractIdentifier, "DatasetGuid");
                    m_ListOfVirtualMachines.Add(0, new MicrosoftAzureVirtualMachine(clusterIdentifier, initializerOnlySettingsDialog.DigitalContractIdentifier, datasetIdentifier, initializerOnlySettingsDialog.DatasetFilename, SailWebApiPortalInterop.GetIpAddress(), initializerOnlySettingsDialog.IpAddress));
                    m_IsConfigured = true;
                }
                else
                {
                    m_IsConfigured = false;
                }
            }
        }

        /// <summary>
        /// Constructor used when instantiating InitializerSetting for
        /// automatic configuration from file
        /// </summary>
        /// <param name="settingsFilename"></param>
        public InitializerSettings(
            string settingsFilename, 
            bool isConfidential
            )
        {
            // Read the settings from file. The settings should be:
            // # SubscriptionIdentifier
            // # ResourceGroup
            // # Location
            // # VirtualNetwork
            // # NetworkSecurityGroup
            // <SubscriptionIdentifier>
            // <ResourceGroup>
            // <Location>
            // <VirtualNetwork>
            // <NetworkSecurityGroup>
            // # One or more virtual machine settings using the template. The number of virtual
            // # machines to instantiate is derived from the number of entries below
            // # BaseMachineName,VirtualMachineSize,DigitalContractIdentifier,DatasetFile
            // <BaseMachineName>,<VirtualMachineSize>,<DigitalContractIdentifier>,<DatasetFile>
            // ...

            try
            {
                m_ListOfVirtualMachines = new Dictionary<uint, MicrosoftAzureVirtualMachine>();
                string clusterIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
                uint numberOfDigitalContracts = SailWebApiPortalInterop.LoadDigitalContracts();
                string singleLineOfText;
                System.IO.StreamReader file = new System.IO.StreamReader(settingsFilename);
                // Line 1
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                if ("# SubscriptionIdentifier" != singleLineOfText) throw new FormatException("Line 1 is invalid. Expecting \"# SubscriptionIdentifier\"");
                // Line 2
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                if ("# ResourceGroup" != singleLineOfText) throw new FormatException("Line 2 is invalid. Expecting \"# ResourceGroup\"");
                // Line 3
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                if ("# Location" != singleLineOfText) throw new FormatException("Line 3 is invalid. Expecting \"# Location\"");
                // Line 4
                if (true == isConfidential)
                {
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    if ("# VirtualNetwork" != singleLineOfText) throw new FormatException("Line 4 is invalid. Expecting \"# VirtualNetwork\"");
                }   
                // Line 5
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                if ("# NetworkSecurityGroup" != singleLineOfText) throw new FormatException("Line 5 is invalid. Expecting \"# NetworkSecurityGroup\"");
                // Only needed if confidential Virtual Machine
                if (true == isConfidential)
                {
                    // Line 6
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    if ("# ConfidentialOsDisksUrl" != singleLineOfText) throw new FormatException("Line 5 is invalid. Expecting \"# ConfidentialOsDisksUrl\"");
                    // Line 7
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    if ("# ConfidentialOsDiskVgmsUrl" != singleLineOfText) throw new FormatException("Line 5 is invalid. Expecting \"# ConfidentialOsDiskVgmsUrl\"");
                    // Line 8
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    if ("# OsDiskStorageAccount" != singleLineOfText) throw new FormatException("Line 5 is invalid. Expecting \"# OsDiskStorageAccount\"");
                }
                else
                {
                    // Line 5
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    if ("# OsDiskUrl" != singleLineOfText) throw new FormatException("Line 5 is invalid. Expecting \"# OsDiskUrl\"");
                    // Line 6
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    if ("# OsDiskStorageAccountID" != singleLineOfText) throw new FormatException("Line 5 is invalid. Expecting \"# OsDiskStorageAccountID\"");
                }
                // Line 6
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                string subscriptionIdentifier = singleLineOfText;
                // Line 7
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                string resourceGroup = singleLineOfText;
                // Line 8
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                string location = singleLineOfText;
                string virtualNetwork = "";
                if (true == isConfidential)
                {
                    // Line 9
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    virtualNetwork = singleLineOfText;
                }
                // Line 10
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                string networkSecurityGroup = singleLineOfText;
                // Line 11
                // Only needed if confidential Virtual Machine
                string confidentialVmOsDiskUrl = "";
                string confidentialOsDiskVgmsUrl = "";
                string osDiskStorageAccount = "";
                string osDiskUrl = "";
                if (true == isConfidential)
                {
                    // Line 14
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    confidentialVmOsDiskUrl = singleLineOfText;
                    // Line 15
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    confidentialOsDiskVgmsUrl = singleLineOfText;
                }
                else 
                {
                    // Line 14
                    singleLineOfText = file.ReadLine();
                    singleLineOfText.Trim();
                    osDiskUrl = singleLineOfText;
                }
                // Line 16
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                osDiskStorageAccount = singleLineOfText;
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                if ("# One or more virtual machine settings using the template. The number of virtual" != singleLineOfText) throw new FormatException("Line 11 is invalid. Expecting \"# One or more virtual machine settings using the template. The number of virtual\"");
                // Line 12
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                if ("# machines to instantiate is derived from the number of entries below" != singleLineOfText) throw new FormatException("Line 12 is invalid. Expecting \"# machines to instantiate is derived from the number of entries below\"");
                // Line 13
                singleLineOfText = file.ReadLine();
                singleLineOfText.Trim();
                if ("# BaseMachineName,VirtualMachineSize,DigitalContractIdentifier,DatasetFile" != singleLineOfText) throw new FormatException("Line 13 is invalid. Expecting \"# BaseMachineName,VirtualMachineSize,DigitalContractIdentifier,DatasetFile\"");
                uint index = 0;
                do
                {
                    // First we read the line
                    singleLineOfText = file.ReadLine();
                    if (null != singleLineOfText)
                    {
                        singleLineOfText.Trim();
                        // Now we cut the line up in pieces
                        string[] singleLineOfTextElement = singleLineOfText.Split(',');
                        if (3 != singleLineOfTextElement.Length) throw new FormatException("Line " + Convert.ToString(14 + index) + " is invalid. Expecting 4 element, but found " + singleLineOfTextElement.Length);
                        string virtualMachineSize = singleLineOfTextElement[0];
                        if (true == isConfidential)
                        {
                            string[] strListOfVmSizes = { "Standard_DC2as_v4", "Standard_DC8as_v4", "Standard_DC16as_v4", "Standard_DC32as_v4", "Standard_DC48as_v4", "Standard_DC96as_v4" };
                            if (!strListOfVmSizes.Contains(virtualMachineSize)) throw new FormatException("Line " + Convert.ToString(14 + index) + " is invalid. VM Size " + virtualMachineSize + " is not permitted.");
                        }
                        string digitalContractIdentifier = singleLineOfTextElement[1];
                        string datasetFilename = singleLineOfTextElement[2];
                        if (false == System.IO.File.Exists(datasetFilename)) throw new FormatException("Line " + Convert.ToString(14 + index) + " contains an invalid dataset name. File " + datasetFilename + " not found");
                        string datasetIdentifier = SailWebApiPortalInterop.GetDigitalContractProperty(digitalContractIdentifier, "DatasetGuid");
                        if (0 == datasetIdentifier.Length) throw new FormatException("Line " + Convert.ToString(14 + index) + " contains an invalid digital contract identifier. DC " + digitalContractIdentifier + " not found");
                        // Now that we have all the information needed, create a new MicrosoftAzureVirtualMachine
                        if (true == isConfidential)
                        {
                            m_ListOfVirtualMachines.Add(index, new MicrosoftAzureVirtualMachine(clusterIdentifier, digitalContractIdentifier, datasetIdentifier, datasetFilename, SailWebApiPortalInterop.GetIpAddress(), subscriptionIdentifier, resourceGroup, location, virtualNetwork, networkSecurityGroup, confidentialVmOsDiskUrl, confidentialOsDiskVgmsUrl, osDiskStorageAccount, virtualMachineSize));
                        }
                        else
                        {
                            m_ListOfVirtualMachines.Add(index, new MicrosoftAzureVirtualMachine(clusterIdentifier, digitalContractIdentifier, datasetIdentifier, datasetFilename, SailWebApiPortalInterop.GetIpAddress(), subscriptionIdentifier, resourceGroup, location, osDiskUrl, networkSecurityGroup, osDiskStorageAccount, virtualMachineSize));
                        }
                        index++;
                    }
                }
                while (null != singleLineOfText);
                file.Close();
                m_IsConfigured = true;
            }

            catch(FormatException e)
            {
                System.Windows.Forms.MessageBox.Show(e.Message, "Configuration File Reading Error", System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Error);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public bool IsConfigured
        {
            get
            {
                return m_IsConfigured;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public MicrosoftAzureVirtualMachine[] MicrosoftAzureVirtualMachines
        {
            get
            {
                MicrosoftAzureVirtualMachine[] listOfMicrosoftAzureVirtualMachines = new MicrosoftAzureVirtualMachine[m_ListOfVirtualMachines.Count];
                m_ListOfVirtualMachines.Values.CopyTo(listOfMicrosoftAzureVirtualMachines, 0);
                return listOfMicrosoftAzureVirtualMachines;
            }
        }

        // Private data members
        private bool m_IsConfigured;
        private System.Collections.Generic.Dictionary<uint, MicrosoftAzureVirtualMachine> m_ListOfVirtualMachines;
    }
}
