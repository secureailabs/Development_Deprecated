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
    public partial class MicrosoftAzureLoginDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public MicrosoftAzureLoginDialog()
        {
            InitializeComponent();

            m_MicrosoftAzureExceptionsDialog = new ExceptionsDialog();

            // Load default settings from the registry of they exist
            RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
            string[] registryKeyValues = registryKey.GetValueNames();
            if (true == registryKeyValues.Contains("DefaultMicrosoftAzureApiPortalApplicationIdentifier"))
            {
                m_ApplicationIdentifierTextBox.Text = registryKey.GetValue("DefaultMicrosoftAzureApiPortalApplicationIdentifier").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultMicrosoftAzureApiPortalTenantIdentifier"))
            {
                m_TenantIdentifierEditBox.Text = registryKey.GetValue("DefaultMicrosoftAzureApiPortalTenantIdentifier").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultMicrosoftAzureApiPortalSecret"))
            {
                m_SecretEditBox.Text = registryKey.GetValue("DefaultMicrosoftAzureApiPortalSecret").ToString();
            }
            registryKey.Close();

            // Figure out where to put the input focus based on which text box is pre-populated
            if ("" != m_ApplicationIdentifierTextBox.Text)
            {
                if ("" != m_TenantIdentifierEditBox.Text)
                {
                    m_SecretEditBox.Select();
                }
                else
                {
                    m_TenantIdentifierEditBox.Select();
                }
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_LoginButton_Click(
            object sender,
            EventArgs e
            )
        {
            if (("" == m_ApplicationIdentifierTextBox.Text) || ("" == m_SecretEditBox.Text) || ("" == m_TenantIdentifierEditBox.Text))
            {
                MessageBox.Show(this, "Missing input!!!\r\nAll three input fields must be specified.", "Error!", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
            {
                if (false == MicrosoftAzureApiPortalInterop.LoginToMicrosoftAzureApiPortal(m_ApplicationIdentifierTextBox.Text, m_SecretEditBox.Text, m_TenantIdentifierEditBox.Text))
                {
                    MessageBox.Show(this, "Invalid credentials provided. Try again!!", "AuthenticationFailure", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                else
                {
                    // Persist some of the settings to the registry to make it easier to restart the
                    // application later.
                    RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
                    registryKey.SetValue("DefaultMicrosoftAzureApiPortalApplicationIdentifier", m_ApplicationIdentifierTextBox.Text);
                    registryKey.SetValue("DefaultMicrosoftAzureApiPortalTenantIdentifier", m_TenantIdentifierEditBox.Text);
                    registryKey.SetValue("DefaultMicrosoftAzureApiPortalSecret", m_SecretEditBox.Text);
                    registryKey.Close();
                    this.m_RefreshTimer.Stop();
                    this.DialogResult = DialogResult.OK;
                    this.Close();
                }
            }
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
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ExceptionStripStatusLabel_Click(
            object sender,
            EventArgs e
            )
        {
            m_MicrosoftAzureExceptionsDialog.ShowDialog(this);
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
            if (0 < MicrosoftAzureApiPortalInterop.GetExceptionCount())
            {
                m_ExceptionStripStatusLabel.Text = "Click here to view exceptions";
            }
            else
            {
                m_ExceptionStripStatusLabel.Text = "";
            }
        }

        private ExceptionsDialog m_MicrosoftAzureExceptionsDialog;
    }
}
