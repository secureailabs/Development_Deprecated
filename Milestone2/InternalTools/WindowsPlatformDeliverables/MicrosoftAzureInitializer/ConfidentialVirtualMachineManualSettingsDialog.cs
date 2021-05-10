using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Win32;
using System.Windows.Forms;

namespace MicrosoftAzureVirtualMachineInitializer
{
    public partial class ConfidentialVirtualMachineManualSettingsDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public ConfidentialVirtualMachineManualSettingsDialog()
        {
            InitializeComponent();

            m_ExceptionsDialog = new ExceptionsDialog();

            // Get a list of all of the digital contracts
            uint numberOfDigitalContracts = SailWebApiPortalInterop.LoadDigitalContracts();
            for (uint index = 0; index < numberOfDigitalContracts; index++)
            {
                string digitalContractIdentifier = SailWebApiPortalInterop.GetDigitalContractIdentifierAtIndex(index);
                m_DigitalContractsComboBox.Items.Add(digitalContractIdentifier);
            }
            // See if the Azure settings were cached in the registry
            // Load default settings from the registry of they exist
            Microsoft.Win32.RegistryKey registryKey = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
            string[] registryKeyValues = registryKey.GetValueNames();
            if (true == registryKeyValues.Contains("DefaultDatasetFilename"))
            {
                m_DatasetFilenameTextBox.Text = registryKey.GetValue("DefaultDatasetFilename").ToString();
            }
            if (true == registryKeyValues.Contains("CvmDefaultMicrosoftAzureSubscriptionIdentifier"))
            {
                m_SubscriptionIdentifierTextBox.Text = registryKey.GetValue("CvmDefaultMicrosoftAzureSubscriptionIdentifier").ToString();
            }
            if (true == registryKeyValues.Contains("CvmDefaultMicrosoftAzureResourceGroup"))
            {
                m_ResourceGroupTextBox.Text = registryKey.GetValue("CvmDefaultMicrosoftAzureResourceGroup").ToString();
            }
            if (true == registryKeyValues.Contains("CvmCDefaultMicrosoftAzureLocation"))
            {
                m_LocationTextBox.Text = registryKey.GetValue("CvmCDefaultMicrosoftAzureLocation").ToString();
            }
            if (true == registryKeyValues.Contains("CvmDefaultMicrosoftVirtualNetwork"))
            {
                m_VirtualNetworkTextBox.Text = registryKey.GetValue("CvmDefaultMicrosoftVirtualNetwork").ToString();
            }
            if (true == registryKeyValues.Contains("CvmDefaultMicrosoftNetworkSecurityGroup"))
            {
                m_NetworkSecurityGroupTextBox.Text = registryKey.GetValue("CvmDefaultMicrosoftNetworkSecurityGroup").ToString();
            }
            if (true == registryKeyValues.Contains("CvmDefaultMicrosoftOsDiskUrl"))
            {
                m_VmOsDiskUrlTextBox.Text = registryKey.GetValue("CvmDefaultMicrosoftOsDiskUrl").ToString();
            }
            if (true == registryKeyValues.Contains("CvmDefaultMicrosoftOsDiskVgmsUrl"))
            {
                m_VmOsDiskVgmsUrlTextBox.Text = registryKey.GetValue("CvmDefaultMicrosoftOsDiskVgmsUrl").ToString();
            }
            if (true == registryKeyValues.Contains("CvmDefaultMicrosoftOsDiskStorageAccount"))
            {
                m_OsDiskStorageAccountIdTextBox.Text = registryKey.GetValue("CvmDefaultMicrosoftOsDiskStorageAccount").ToString();
            }
            if (true == registryKeyValues.Contains("CvmDefaultMicrosoftVirtualMachineSize"))
            {
                m_VirtualMachineSizeComboBox.Text = registryKey.GetValue("CvmDefaultMicrosoftVirtualMachineSize").ToString();
            }
            registryKey.Close();

            m_DigitalContractsComboBox.SelectedIndex = 0;
        }

        /// <summary>
        /// 
        /// </summary>
        public string DigitalContractIdentifier
        {
            get
            {
                return m_DigitalContractsComboBox.SelectedItem.ToString();
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string DatasetFilename
        {
            get
            {
                return m_DatasetFilenameTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public uint VirtualMachineCount
        {
            get
            {
                return (uint) m_VirtualMachineCountNumericUpDown.Value;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureSubscriptionIdentifier
        {
            get
            {
                return m_SubscriptionIdentifierTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureResourceGroup
        {
            get
            {
                return m_ResourceGroupTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureLocation
        {
            get
            {
                return m_LocationTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureVirtualNetwork
        {
            get
            {
                return m_VirtualNetworkTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureNetworkSecurityGroup
        {
            get
            {
                return m_NetworkSecurityGroupTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureVmOsDiskUrl
        {
            get
            {
                return m_VmOsDiskUrlTextBox.Text;
            }
        }

        
        /// <summary>
        /// 
        /// </summary>
        public string AzureVmOsVgmsDiskUrl
        {
            get
            {
                return m_VmOsDiskVgmsUrlTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureOsDiskStorageAccount
        {
            get
            {
                return m_OsDiskStorageAccountIdTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureVirtualMachineSize
        {
            get
            {
                return m_VirtualMachineSizeComboBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_NextButton_Click(
            object sender,
            EventArgs e
            )
        {
            // Persist some of the settings to the registry to make it easier to restart the
            // application later.
            RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
            registryKey.SetValue("DefaultDatasetFilename", m_DatasetFilenameTextBox.Text);
            registryKey.SetValue("CvmDefaultMicrosoftAzureSubscriptionIdentifier", m_SubscriptionIdentifierTextBox.Text);
            registryKey.SetValue("CvmDefaultMicrosoftAzureResourceGroup", m_ResourceGroupTextBox.Text);
            registryKey.SetValue("CvmCDefaultMicrosoftAzureLocation", m_LocationTextBox.Text);
            registryKey.SetValue("CvmDefaultMicrosoftVirtualNetwork", m_VirtualNetworkTextBox.Text);
            registryKey.SetValue("CvmDefaultMicrosoftNetworkSecurityGroup", m_NetworkSecurityGroupTextBox.Text);
            registryKey.SetValue("CvmDefaultMicrosoftOsDiskUrl", m_VmOsDiskUrlTextBox.Text);
            registryKey.SetValue("CvmDefaultMicrosoftOsDiskVgmsUrl", m_VmOsDiskVgmsUrlTextBox.Text);
            registryKey.SetValue("CvmDefaultMicrosoftOsDiskStorageAccount", m_OsDiskStorageAccountIdTextBox.Text);
            registryKey.SetValue("CvmDefaultMicrosoftVirtualMachineSize", m_VirtualMachineSizeComboBox.Text);
            registryKey.Close();
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_PreviousButton_Click(
            object sender,
            EventArgs e
            )
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_HelpStripStatusLabel_Click(
            object sender,
            EventArgs e
            )
        {
            //MessageBox.Show(this, global::MicrosoftAzureInitializer.Properties.Resources..ToString(), "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_DatasetFilenameTextBox_TextChanged(
            object sender,
            EventArgs e
            )
        {
            if (true == System.IO.File.Exists(m_DatasetFilenameTextBox.Text))
            {
                if ((null != m_DigitalContractsComboBox.SelectedItem) && (0 < m_DigitalContractsComboBox.SelectedItem.ToString().Length))
                {
                    m_NextButton.Enabled = true;
                }
                else
                {
                    m_NextButton.Enabled = false;
                }
            }
            else
            {
                m_DatasetFilenameTextBox.Text = "";
                m_NextButton.Enabled = false;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_BrowseForDatasetFilenameLinkLabel_LinkClicked(
            object sender,
            LinkLabelLinkClickedEventArgs e
            )
        {
            if (DialogResult.OK == m_OpenFileDialog.ShowDialog(this))
            {
                m_DatasetFilenameTextBox.Text = m_OpenFileDialog.FileName;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ExceptionsToolStripStatusLabel_Click(
            object sender,
            EventArgs e
            )
        {
            m_ExceptionsDialog.ShowDialog(this);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_RefreshTimer_Tick(
            object sender,
            EventArgs e
            )
        {
            if ((0 < m_DatasetFilenameTextBox.Text.Length) && (0 < m_SubscriptionIdentifierTextBox.Text.Length) && (0 < m_ResourceGroupTextBox.Text.Length) && (0 < m_LocationTextBox.Text.Length) && (0 < m_VirtualNetworkTextBox.Text.Length) && (0 < m_NetworkSecurityGroupTextBox.Text.Length))
            {
                m_NextButton.Enabled = true;
            }
            else
            {
                m_NextButton.Enabled = false;
            }

            if (0 == m_ExceptionsDialog.ExceptionsCount)
            {
                m_ExceptionsToolStripStatusLabel.Text = "";
            }
            else
            {
                m_ExceptionsToolStripStatusLabel.Text = "Click here to view exceptions";
            }
        }

        private ExceptionsDialog m_ExceptionsDialog;
    }
}
