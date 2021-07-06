using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SailAuditInterface
{
    public partial class ReloadAllAuditEventsDialog : Form
    {
        public ReloadAllAuditEventsDialog()
        {
            InitializeComponent();
        }

        /// <summary>
        /// 
        /// </summary>
        public static void ReloadAllAuditEvents()
        {
            ApiInterop.ReloadLoadAllAuditEvents("{00000000-0000-0000-0000-000000000000}");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="e"></param>
        protected override void OnShown(
            EventArgs e
            )
        {
            m_AuditLoadingThread = new System.Threading.Thread(ReloadAllAuditEventsDialog.ReloadAllAuditEvents);
            m_AuditLoadingThread.Start();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_WaitForAllAuditsToLoadTimer_Tick(
            object sender,
            EventArgs e
            )
        {
            if (m_AuditLoadingThread.ThreadState == System.Threading.ThreadState.Stopped)
            {
                this.Close();
            }
        }

        private System.Threading.Thread m_AuditLoadingThread;
    }
}
