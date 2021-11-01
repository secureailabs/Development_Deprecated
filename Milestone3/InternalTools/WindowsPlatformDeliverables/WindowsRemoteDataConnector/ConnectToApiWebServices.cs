using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WindowsRemoteDataConnector
{
    public partial class ConnectToApiWebServices : Form
    {
        public ConnectToApiWebServices(
            string username,
            string password
            )
        {
            InitializeComponent();

            m_Username = username;
            m_Password = password;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_QuitTryingAndExitButton_Click(
            object sender,
            EventArgs e
            )
        {
            this.DialogResult = DialogResult.Abort;
            this.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_RetryTimer_Tick(
            object sender,
            EventArgs e
            )
        {
            if (true == SailWebApiPortalInterop.Login(m_Username, m_Password))
            {
                this.m_RetryTimer.Enabled = false;
                this.DialogResult = DialogResult.OK;
                this.Close();
            }
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
            
        }

        private string m_Username;
        private string m_Password;
    }
}
