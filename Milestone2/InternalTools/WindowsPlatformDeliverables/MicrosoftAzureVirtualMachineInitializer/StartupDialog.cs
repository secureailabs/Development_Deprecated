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
                this.DialogResult = DialogResult.Yes;
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
            this.DialogResult = DialogResult.OK;
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
        private void m_CloseButton_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }
    }
}
