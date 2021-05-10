using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace MicrosoftAzureVirtualMachineInitializer
{
    public partial class StartupDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public StartupDialog()
        {
            InitializeComponent();

            m_DialogResult = 0;
        }

        /// <summary>
        /// 
        /// </summary>
        public uint DialogResultValue
        {
            get
            {
                return m_DialogResult;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public bool IsConfidential
        {
            get
            {
                return m_ConfidentialVirtualMachine.Checked;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string SettingsFilename
        {
            get
            {
                return m_OpenSettingsFileDialog.FileName;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_LoadFromConfigurationButton_Click(
            object sender,
            EventArgs e
            )
        {
            this.Hide();
            if (DialogResult.OK == m_OpenSettingsFileDialog.ShowDialog(this))
            {
                m_DialogResult = 1;
                this.DialogResult = DialogResult.OK;
                this.Close();
            }
            else
            {
                this.Show();
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ManuallyConfigureButton_Click(
            object sender,
            EventArgs e
            )
        {
            m_DialogResult = 2;
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_StartWebPortalVirtualMachineButton_Click(
            object sender,
            EventArgs e
            )
        {
            MessageBox.Show(this, "Not implemented yet", "Information", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_InitializerButton_Click(
            object sender,
            EventArgs e
            )
        {
            m_DialogResult = 3;
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_CloseButton_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private uint m_DialogResult;

        private void m_WebPortalGroupBox_Click(object sender, EventArgs e)
        {

        }

        private void m_InitializerOnlyLabel_Click(object sender, EventArgs e)
        {

        }

        private void m_InstructionsLabel_Click(object sender, EventArgs e)
        {

        }
    }
}
