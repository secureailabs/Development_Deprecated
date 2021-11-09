using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SailAuditInterface
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            // Attempt to log in
            LoginDialog loginDialog = new LoginDialog();
            // Ask for credentials
            Application.Run(loginDialog);
            // Only continue if the password dialog outcome is 1
            if (1 == loginDialog.m_PasswordDialogOutcome)
            {
                // Do the first initial audit events load
                ReloadAllAuditEventsDialog reloadAllAuditEventsDialog = new ReloadAllAuditEventsDialog();
                reloadAllAuditEventsDialog.ShowDialog();
                // Okay now that we are logged in, let's start displaying the events
                AuditEventsViewer auditEventViewer = new AuditEventsViewer();
                Application.Run(auditEventViewer);
            }
        }
    }
}
