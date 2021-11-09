using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace MicrosoftAzureVirtualMachineInitializer
{
    public partial class VirtualMachineProvisionerDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public VirtualMachineProvisionerDialog(
            ref InitializerSettings initializerSettings
            )
        {
            InitializeComponent();

            m_ExceptionsDialog = new ExceptionsDialog();

            m_IsGoing = false;
            m_IsReady = false;
            m_ListOfMicrosoftAzureVirtualMachines = initializerSettings.MicrosoftAzureVirtualMachines; 
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void VirtualMachineProvisionerDialog_Load(
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
        private void m_GoButton_Click(
            object sender,
            EventArgs e
            )
        {
            foreach (MicrosoftAzureVirtualMachine microsoftAzureVirtualMachine in m_ListOfMicrosoftAzureVirtualMachines)
            {
                microsoftAzureVirtualMachine.ProvisionAndInitialize();
            }
            m_IsGoing = true;
            m_LoopIndex = 0;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ExceptionsStripStatusLabel_Click(
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
        private void m_ExitButton_Click(
            object sender,
            EventArgs e
            )
        {
            if (true == m_IsReady)
            {
                this.m_RefreshTimer.Stop();

                // Display some information on the way out
                MessageBox.Show(this, "A paramatrized summary of settings and virtual machine information will be automatically copied to the system clipboad in a comma delimited format.", "Success!!!", MessageBoxButtons.OK, MessageBoxIcon.Information);
                // Copy the results of the VM's in the clipboard
                string fullSummary = "";
                foreach (MicrosoftAzureVirtualMachine microsoftAzureVirtualMachine in m_ListOfMicrosoftAzureVirtualMachines)
                {
                    if (0 == fullSummary.Length)
                    {
                        // Write out a header with the Azure settings first. This happens before we print out the parameters of
                        // the first virtual machine
                        fullSummary += "-----------------------------------------------------------------------------------------------------------------------------\r\n";
                        fullSummary += System.DateTime.Now.ToString() + "\r\n";
                        fullSummary += "-----------------------------------------------------------------------------------------------------------------------------\r\n";
                        fullSummary += "Subscription, " + microsoftAzureVirtualMachine.AzureSubscriptionIdentifier + "\r\n";
                        fullSummary += "ResourceGroup, " + microsoftAzureVirtualMachine.AzureResourceGroup + "\r\n";
                        fullSummary += "Location ," + microsoftAzureVirtualMachine.AzureLocation + "\r\n";
                        fullSummary += "BaseMachineName, " + microsoftAzureVirtualMachine.AzureBaseMachineName + "\r\n";
                        fullSummary += "VirtualMachineSize, " + microsoftAzureVirtualMachine.AzureVirtualMachineSize + "\r\n";
                        fullSummary += "VirtualNetwork, " + microsoftAzureVirtualMachine.AzureVirtualNetwork + "\r\n";
                        fullSummary += "NetworkSecurityGroup, " + microsoftAzureVirtualMachine.AzureNetworkSecurityGroup + "\r\n";
                        fullSummary += "-----------------------------------------------------------------------------------------------------------------------------\r\n";
                        fullSummary += "VirtualMachineIdentifier, VirtualMachineIpAddress, VirtualMachineClusterIdentier\r\n";
                        fullSummary += "-----------------------------------------------------------------------------------------------------------------------------\r\n";
                    }
                    string virtualMachineSummary = microsoftAzureVirtualMachine.VirtualMachineIdentifier + ", " + microsoftAzureVirtualMachine.IpAddress + ", " + microsoftAzureVirtualMachine.ClusterIdentifier + "\r\n";
                    fullSummary += virtualMachineSummary;
                }
                fullSummary += "-----------------------------------------------------------------------------------------------------------------------------\r\n";
                System.Windows.Forms.Clipboard.SetText(fullSummary);
            }
            
            this.Close();
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
            m_ListBox.BeginUpdate();
            m_ListBox.Items.Clear();
            uint numberOfUnreadyNodes = 0;
            foreach (MicrosoftAzureVirtualMachine microsoftAzureVirtualMachine in m_ListOfMicrosoftAzureVirtualMachines)
            {
                string[] strSpinners = { ">     ", " >    ", "  >   ", "   >  ", "    > ", "     >", "     <", "    < ", "   <  ", "  <   ", " <    ", "<     " };
                string strToDisplay;
                
                if ("Ready..." == microsoftAzureVirtualMachine.VirtualMachineStatus)
                {
                    strToDisplay = string.Format("{0,-8}{1,-40}{2,-17}{3,-20}", "------", microsoftAzureVirtualMachine.VirtualMachineIdentifier.ToUpper(), microsoftAzureVirtualMachine.IpAddress, microsoftAzureVirtualMachine.VirtualMachineStatus);
                }
                else if ("Not Started" == microsoftAzureVirtualMachine.VirtualMachineStatus)
                {
                    strToDisplay = string.Format("{0,-8}{1,-40}{2,-17}{3,-20}", "      ", microsoftAzureVirtualMachine.VirtualMachineIdentifier.ToUpper(), microsoftAzureVirtualMachine.IpAddress, microsoftAzureVirtualMachine.VirtualMachineStatus);
                    numberOfUnreadyNodes++;
                }
                else
                {
                    strToDisplay = string.Format("{0,-8}{1,-40}{2,-17}{3,-20}", strSpinners[(m_LoopIndex + m_ListBox.Items.Count) % strSpinners.Length], microsoftAzureVirtualMachine.VirtualMachineIdentifier.ToUpper(), microsoftAzureVirtualMachine.IpAddress, microsoftAzureVirtualMachine.VirtualMachineStatus);
                    numberOfUnreadyNodes++;
                }

                m_ListBox.Items.Add(strToDisplay);
            }
            m_ListBox.EndUpdate();

            if (true == m_IsGoing)
            {
                if (0 == numberOfUnreadyNodes)
                {
                    m_IsReady = true;
                    m_ExitButton.Enabled = true;
                    m_IsGoing = false;
                }
                else
                {
                    m_LoopIndex++;
                    m_GoButton.Enabled = false;
                    m_ExitButton.Enabled = false;
                }
            }

            if (0 < m_ExceptionsDialog.ExceptionsCount)
            {
                m_ExceptionsStripStatusLabel.Text = "Click here to view exceptions";
            }
            else
            {
                m_ExceptionsStripStatusLabel.Text = "";
            }
        }

        // Private data members
        MicrosoftAzureVirtualMachine[] m_ListOfMicrosoftAzureVirtualMachines;
        ExceptionsDialog m_ExceptionsDialog;
        uint m_LoopIndex;
        bool m_IsGoing;
        bool m_IsReady;
    }
}
