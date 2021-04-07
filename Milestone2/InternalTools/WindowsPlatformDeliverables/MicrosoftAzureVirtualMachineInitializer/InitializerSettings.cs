using System;
using System.Collections.Generic;
using System.Text;

namespace MicrosoftAzureVirtualMachineInitializer
{
    public class InitializerSettings
    {
        /// <summary>
        /// Constructor used when instantiating InitializerSettings for
        /// manual configuration
        /// </summary>
        public InitializerSettings()
        {
            m_ListOfVirtualMachines = new Dictionary<uint, MicrosoftAzureVirtualMachine>();
            ManualSettingsDialog manualSettingsDialog = new ManualSettingsDialog();
            if (System.Windows.Forms.DialogResult.OK == manualSettingsDialog.ShowDialog())
            {
                string clusterIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
                string datasetIdentifier = SailWebApiPortalInterop.GetDigitalContractProperty(manualSettingsDialog.DigitalContractIdentifier, "DatasetGuid");
                for (uint index = 0; index < manualSettingsDialog.VirtualMachineCount; index++)
                {
                    m_ListOfVirtualMachines.Add(index, new MicrosoftAzureVirtualMachine(clusterIdentifier, manualSettingsDialog.DigitalContractIdentifier, datasetIdentifier, manualSettingsDialog.DatasetFilename, SailWebApiPortalInterop.GetIpAddress(), manualSettingsDialog.AzureSubscriptionIdentifier, manualSettingsDialog.AzureResourceGroup, manualSettingsDialog.AzureLocation, manualSettingsDialog.AzureVirtualNetwork, manualSettingsDialog.AzureNetworkSecurityGroup, manualSettingsDialog.AzureBaseMachineName, manualSettingsDialog.AzureVirtualMachineSize));
                }
                m_IsConfigured = true;
            }
            else
            {
                m_IsConfigured = false;
            }
        }

        /// <summary>
        /// Constructor used when instantiating InitializerSetting for
        /// automatic configuration from file
        /// </summary>
        /// <param name="settingsFilename"></param>
        public InitializerSettings(
            string settingsFilename
            )
        {

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
