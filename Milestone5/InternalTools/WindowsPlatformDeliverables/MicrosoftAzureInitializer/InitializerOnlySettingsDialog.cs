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
    public partial class InitializerOnlySettingsDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public InitializerOnlySettingsDialog()
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
            m_DigitalContractsComboBox.SelectedIndex = 0;
            // See if the Azure settings were cached in the registry
            // Load default settings from the registry of they exist
            Microsoft.Win32.RegistryKey registryKey = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
            string[] registryKeyValues = registryKey.GetValueNames();
            if (true == registryKeyValues.Contains("DefaultDatasetFilename"))
            {
                m_DatasetFilenameTextBox.Text = registryKey.GetValue("DefaultDatasetFilename").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultIpAddress"))
            {
                m_IpAddressTextBox.Text = registryKey.GetValue("DefaultIpAddress").ToString();
            }
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
        public string IpAddress
        {
            get
            {
                return m_IpAddressTextBox.Text;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_BrowseLink_LinkClicked(
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
        private void m_NextButton_Click(
            object sender,
            EventArgs e
            )
        {
            // Persist some of the settings to the registry to make it easier to restart the
            // application later.
            RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
            registryKey.SetValue("DefaultDatasetFilename", m_DatasetFilenameTextBox.Text);
            registryKey.SetValue("DefaultIpAddress", m_IpAddressTextBox.Text);
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
            if ((0 < m_DatasetFilenameTextBox.Text.Length) && (0 < m_IpAddressTextBox.Text.Length) )
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
