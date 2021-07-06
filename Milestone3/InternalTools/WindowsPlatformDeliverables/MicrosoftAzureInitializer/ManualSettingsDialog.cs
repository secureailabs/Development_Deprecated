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
    public partial class ManualSettingsDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public ManualSettingsDialog()
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
            if (true == registryKeyValues.Contains("DefaultMicrosoftAzureSubscriptionIdentifier"))
            {
                m_SubscriptionIdentifierTextBox.Text = registryKey.GetValue("DefaultMicrosoftAzureSubscriptionIdentifier").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultMicrosoftAzureResourceGroup"))
            {
                m_ResourceGroupTextBox.Text = registryKey.GetValue("DefaultMicrosoftAzureResourceGroup").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultMicrosoftAzureLocation"))
            {
                m_LocationTextBox.Text = registryKey.GetValue("DefaultMicrosoftAzureLocation").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultMicrosoftOsDiskUrl"))
            {
                m_OsDiskUrlTextBox.Text = registryKey.GetValue("DefaultMicrosoftOsDiskUrl").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultMicrosoftNetworkSecurityGroup"))
            {
                m_NetworkSecurityGroupTextBox.Text = registryKey.GetValue("DefaultMicrosoftNetworkSecurityGroup").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultMicrosoftOsDiskStorageAccount"))
            {
                m_osDiskStorageAccountTextBox.Text = registryKey.GetValue("DefaultMicrosoftOsDiskStorageAccount").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultMicrosoftVirtualMachineSize"))
            {
                m_VirtualMachineSizeTextBox.Text = registryKey.GetValue("DefaultMicrosoftVirtualMachineSize").ToString();
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
        public string OsDiskUrl
        {
            get
            {
                return m_OsDiskUrlTextBox.Text;
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
        public string OsDiskStorageAccount
        {
            get
            {
                return m_osDiskStorageAccountTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureVirtualMachineSize
        {
            get
            {
                return m_VirtualMachineSizeTextBox.Text;
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
            registryKey.SetValue("DefaultMicrosoftAzureSubscriptionIdentifier", m_SubscriptionIdentifierTextBox.Text);
            registryKey.SetValue("DefaultMicrosoftAzureResourceGroup", m_ResourceGroupTextBox.Text);
            registryKey.SetValue("DefaultMicrosoftAzureLocation", m_LocationTextBox.Text);
            registryKey.SetValue("DefaultMicrosoftVirtualNetwork", m_OsDiskUrlTextBox.Text);
            registryKey.SetValue("DefaultMicrosoftNetworkSecurityGroup", m_NetworkSecurityGroupTextBox.Text);
            registryKey.SetValue("DefaultMicrosoftBaseMachineName", m_osDiskStorageAccountTextBox.Text);
            registryKey.SetValue("DefaultMicrosoftVirtualMachineSize", m_VirtualMachineSizeTextBox.Text);
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
            if ((0 < m_DatasetFilenameTextBox.Text.Length) && (0 < m_SubscriptionIdentifierTextBox.Text.Length) && (0 < m_ResourceGroupTextBox.Text.Length) && (0 < m_LocationTextBox.Text.Length) && (0 < m_OsDiskUrlTextBox.Text.Length) && (0 < m_NetworkSecurityGroupTextBox.Text.Length))
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
