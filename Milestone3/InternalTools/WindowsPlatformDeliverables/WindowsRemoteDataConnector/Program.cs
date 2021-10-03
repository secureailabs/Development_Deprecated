using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WindowsRemoteDataConnector
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
            // First we get the Sail credentials
            SailWebApiPortalLoginDialog sailWebApiPortalLoginDialog = new SailWebApiPortalLoginDialog();
            if (DialogResult.OK == sailWebApiPortalLoginDialog.ShowDialog())
            {
                SailRemoteDataConnectorDialog sailRemoteDataConnectorDialog = new SailRemoteDataConnectorDialog();
                sailRemoteDataConnectorDialog.ShowDialog();
            }
        }
    }
}
