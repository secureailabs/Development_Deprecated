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
        static int Main(
            string[] arguments
            )
        {
            int returnValue = 0;

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            // If the application is called without any parameters, then it will act as the parent instance.
            if (0 == arguments.Length)
            {
                // First we get the Sail credentials and make sure we can log on. This is done as a test from the
                // instance of this parent process. There will be TWO instances running, one designed to restart the other
                // if it closes unexpectedly
                SailWebApiPortalLoginDialog sailWebApiPortalLoginDialog = new SailWebApiPortalLoginDialog();
                if (DialogResult.OK == sailWebApiPortalLoginDialog.ShowDialog())
                {
                    string applicationFullPath = System.Windows.Forms.Application.ExecutablePath;
                    string ipAddress = sailWebApiPortalLoginDialog.IpAddress;
                    string username = sailWebApiPortalLoginDialog.Username;
                    string password = sailWebApiPortalLoginDialog.Password;
                    
                    string ipAddressBase64 = System.Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes(ipAddress));
                    string usernameBase64 = System.Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes(username));
                    string passwordBase64 = System.Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes(password));
                    string commandLineArguments = ipAddressBase64 + " " + usernameBase64 + " " + passwordBase64;
                    int exitCode = 0;
                    do
                    {
                        // Start the child instance and wait for it to exit. Make sure to capture the
                        // exit code
                        System.Diagnostics.ProcessStartInfo processStartInformation = new System.Diagnostics.ProcessStartInfo(applicationFullPath, commandLineArguments);
                        using (System.Diagnostics.Process childProcess = System.Diagnostics.Process.Start(processStartInformation))
                        {
                            childProcess.WaitForExit();
                            exitCode = childProcess.ExitCode;
                        }

                        // Only in one instance, where the child application returns 0x13131313 does the parent
                        // instance try to restart it.
                        if (0x13131313 == exitCode)
                        {
                            ConnectToApiWebServices connectToApiWebServicesDialog = new ConnectToApiWebServices(username, password);
                            if (DialogResult.Abort == connectToApiWebServicesDialog.ShowDialog())
                            {
                                // Normal exit since the user chose to manually abort retrying to connect
                                returnValue = 0x76541234;
                            }
                        }
                    }
                    while (0x76541234 != exitCode);
                }
                else
                {
                    // Normal exit since the user manually aborted the login dialog
                    returnValue = 0x76541234;
                }
            }
            else
            {
                if (3 != arguments.Length)
                {
                    // Invalid command line parameters
                    returnValue = 0x09127834;
                }
                else
                {
                    // Extract the username and password which should be in Base64 encoding on the command line
                    byte[] decodedIpAddress = System.Convert.FromBase64String(arguments[0]);
                    string ipAddress= System.Text.Encoding.Default.GetString(decodedIpAddress);
                    byte[] decodedUsername = System.Convert.FromBase64String(arguments[1]);
                    string username = System.Text.Encoding.Default.GetString(decodedUsername);
                    byte[] decodedPassword = System.Convert.FromBase64String(arguments[2]);
                    string password = System.Text.Encoding.Default.GetString(decodedPassword);
                    // Make sure to register the IP Address
                    SailWebApiPortalInterop.SetIpAddress(ipAddress);
                    // Now, try to login. If it doesn't work, send the "no connection" error
                    if (false == SailWebApiPortalInterop.Login(username, password))
                    {
                        // Return an error which denotes that the RemoteDataConnector was either
                        // 1. Not able to login
                        // 2. Lost connection
                        // In both cases, this will tell the parent application to try restarting the
                        // child instance once it is able to re-establish contact with the API portal
                        // When this happens, the child process ends and the parent process takes over
                        // trying to reconnect to the API portal until it succeeds.
                        returnValue = 0x13131313;
                    }
                    else
                    {
                        SailRemoteDataConnectorDialog sailRemoteDataConnectorDialog = new SailRemoteDataConnectorDialog();
                        sailRemoteDataConnectorDialog.ShowDialog();
                        returnValue = sailRemoteDataConnectorDialog.ExitCode;
                    }
                }
            }

            return returnValue;
        }
    }
}
