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

namespace SailAuditInterface
{
    public partial class LoginDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public LoginDialog()
        {
            InitializeComponent();
            m_PasswordDialogOutcome = 0;

            // Load default settings from the registry of they exist
            RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
            string[] registryKeyValues = registryKey.GetValueNames();
            if (true == registryKeyValues.Contains("DefaultSailWebApiPortalIpAddress"))
            {
                m_IpAddressTextBox.Text = registryKey.GetValue("DefaultSailWebApiPortalIpAddress").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultSailWebApiPortalUsername"))
            {
                m_UsernameEditBox.Text = registryKey.GetValue("DefaultSailWebApiPortalUsername").ToString();
            }
            registryKey.Close();

            if ("" != m_IpAddressTextBox.Text)
            {
                if ("" != m_UsernameEditBox.Text)
                {
                    m_PasswordEditBox.Select();
                }
                else
                {
                    m_UsernameEditBox.Select();
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
            if (("" == m_IpAddressTextBox.Text) || ("" == m_UsernameEditBox.Text) || ("" == m_PasswordEditBox.Text))
            {
                MessageBox.Show(this, "Missing input!!!\r\nAll three input fields must be specified.", "Error!", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
            {
                if (true == ApiInterop.SetSailWebApiPortalIpAddress(m_IpAddressTextBox.Text))
                {
                    bool successfulLogin = ApiInterop.Login(m_UsernameEditBox.Text, m_PasswordEditBox.Text);
                    if (false == successfulLogin)
                    {
                        MessageBox.Show(this, "Invalid credentials provided. Try again!!", "AuthenticationFailure", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else
                    {
                        // Persist some of the settings to the registry to make it easier to restart the
                        // application later.
                        RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
                        registryKey.SetValue("DefaultSailWebApiPortalIpAddress", m_IpAddressTextBox.Text);
                        registryKey.SetValue("DefaultSailWebApiPortalUsername", m_UsernameEditBox.Text);
                        registryKey.Close();
                        m_PasswordDialogOutcome = 1;
                        this.Close();
                    }
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
            m_PasswordDialogOutcome = 2;
            this.Close();
        }

        public int m_PasswordDialogOutcome;
    }
}
