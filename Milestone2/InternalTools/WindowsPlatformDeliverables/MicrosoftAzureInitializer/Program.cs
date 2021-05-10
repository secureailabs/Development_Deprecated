using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MicrosoftAzureVirtualMachineInitializer
{
    static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
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
                MicrosoftAzureLoginDialog microsoftAzureLoginDialog = new MicrosoftAzureLoginDialog();
                if (DialogResult.OK == microsoftAzureLoginDialog.ShowDialog())
                {
                    StartupDialog startupDialog = new StartupDialog();
                    if (DialogResult.OK == startupDialog.ShowDialog())
                    {
                        // Configure the InitializerSettings in one of two ways
                        InitializerSettings initializerSettings = null;
                        switch (startupDialog.DialogResultValue)
                        {
                            case 1   // Load from configuration file
                            :
                                initializerSettings = new InitializerSettings(startupDialog.SettingsFilename, startupDialog.IsConfidential);
                                break;
                            case 2    // Configure manually
                            :
                                initializerSettings = new InitializerSettings(false, startupDialog.IsConfidential);
                                break;
                            case 3    // Initialize Only
                            :
                                initializerSettings = new InitializerSettings(true, startupDialog.IsConfidential);
                                break;
                        }
                        // Are we ready to provision and initialize
                        if ((null != initializerSettings) && (true == initializerSettings.IsConfigured))
                        {
                            VirtualMachineProvisionerDialog virtualMachineProvisionerDialog = new VirtualMachineProvisionerDialog(ref initializerSettings);
                            virtualMachineProvisionerDialog.ShowDialog();
                        }
                    }
                }
            }
            
        }
    }
}