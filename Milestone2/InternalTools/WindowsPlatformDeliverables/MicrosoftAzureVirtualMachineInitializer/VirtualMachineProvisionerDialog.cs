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

            m_IsGoing = false;
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
        private void m_CancelButton_Click(
            object sender,
            EventArgs e
            )
        {
            this.m_RefreshTimer.Stop();
            this.Close();
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
            this.m_RefreshTimer.Stop();
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
            foreach (MicrosoftAzureVirtualMachine microsoftAzureVirtualMachine in m_ListOfMicrosoftAzureVirtualMachines)
            {
                string[] strSpinners = { ">     ", " >    ", "  >   ", "   >  ", "    > ", "     >", "     <", "    < ", "   <  ", "  <   ", " <    ", "<     " };
                string strToDisplay;
                
                if ("Ready!" == microsoftAzureVirtualMachine.VirtualMachineStatus)
                {
                    strToDisplay = string.Format("{0,-7}{1,-40}{2,-17}{3,-20}", "------", microsoftAzureVirtualMachine.VirtualMachineIdentifier.ToUpper(), microsoftAzureVirtualMachine.IpAddress, microsoftAzureVirtualMachine.VirtualMachineStatus);
                }
                if ("Not Started" == microsoftAzureVirtualMachine.VirtualMachineStatus)
                {
                    strToDisplay = string.Format("{0,-7}{1,-40}{2,-17}{3,-20}", "      ", microsoftAzureVirtualMachine.VirtualMachineIdentifier.ToUpper(), microsoftAzureVirtualMachine.IpAddress, microsoftAzureVirtualMachine.VirtualMachineStatus);
                }
                else
                {
                    strToDisplay = string.Format("{0,-7}{1,-40}{2,-17}{3,-20}", strSpinners[(m_LoopIndex + m_ListBox.Items.Count) % strSpinners.Length], microsoftAzureVirtualMachine.VirtualMachineIdentifier.ToUpper(), microsoftAzureVirtualMachine.IpAddress, microsoftAzureVirtualMachine.VirtualMachineStatus); 
                }

                m_ListBox.Items.Add(strToDisplay);
            }
            m_ListBox.EndUpdate();

            if (true == m_IsGoing)
            {
                m_LoopIndex++;
            }
        }

        // Private data members
        MicrosoftAzureVirtualMachine[] m_ListOfMicrosoftAzureVirtualMachines;
        uint m_LoopIndex;
        bool m_IsGoing;
    }
}
