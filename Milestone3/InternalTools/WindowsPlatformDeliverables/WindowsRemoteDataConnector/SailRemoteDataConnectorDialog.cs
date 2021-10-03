using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Win32;

namespace WindowsRemoteDataConnector
{
    public partial class SailRemoteDataConnectorDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public SailRemoteDataConnectorDialog()
        {
            InitializeComponent();

            m_Mutex = new System.Threading.Mutex();
            m_ListOfRegisteredDatasets = new HashSet<string>();
            m_StopButton.Enabled = false;

            // Load default settings from the registry of they exist
            RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
            string[] registryKeyValues = registryKey.GetValueNames();
            if (true == registryKeyValues.Contains("RemoteDataConnectorSourceFolder"))
            {
                m_SourceFolderTextBox.Text = registryKey.GetValue("RemoteDataConnectorSourceFolder").ToString();
                // Put a notification
                this.AddNotification(DateTime.UtcNow.ToString("G") + " (UTC) : Source folder selected (" + m_SourceFolderTextBox.Text + ")");
            }
            else
            {
                // The default is the SailDatasets folder within the MyDocuments folder
                m_SourceFolderTextBox.Text = System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "SailDatasets");
                if (false == System.IO.Directory.Exists(m_SourceFolderTextBox.Text))
                {
                    // If the default folder doesn't exist, we create it.
                    System.IO.Directory.CreateDirectory(m_SourceFolderTextBox.Text);
                    // Persist some of the settings to the registry to make it easier to restart the
                    // application later.
                    registryKey.SetValue("RemoteDataConnectorSourceFolder", m_SourceFolderTextBox.Text);
                    // Put a notification
                    this.AddNotification(DateTime.UtcNow.ToString("G") + " (UTC) : Default source folder created (" + m_SourceFolderTextBox.Text + ")");
                }
            }
            registryKey.Close();

            // Setup default values
            m_NumberOfHeartbeats = 0;
            m_NumberOfFailedHeartbeats = 0;
            m_NumberOfHeartbeatsTextBox.Text = "0";
            m_NumberOfFailedHeartbeatsTextBox.Text = "0";
            m_LastHeartbeartTimeTextBox.Text = "Never";
            m_CurrentTimeTextBox.Text = DateTime.UtcNow.ToString("G");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_BrowseForSourceFolder_Click(
            object sender,
            EventArgs e
            )
        {
            if (DialogResult.OK == m_FolderBrowserDialog.ShowDialog())
            {
                // Register the new source folder
                m_SourceFolderTextBox.Text = m_FolderBrowserDialog.SelectedPath;
                // Persist some of the settings to the registry to make it easier to restart the
                // application later.
                RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
                registryKey.SetValue("RemoteDataConnectorSourceFolder", m_SourceFolderTextBox.Text);
                registryKey.Close();
                // Put a notification
                this.AddNotification(DateTime.UtcNow.ToString("G") + " (UTC) : New source folder selected (" + m_SourceFolderTextBox.Text + ")");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_StartButton_Click(
            object sender,
            EventArgs e
            )
        {
            // Put a notification
            this.AddNotification(DateTime.UtcNow.ToString("G") + " (UTC) : Remote Data Connector started");
            // First we need to list all of the files within the source folder. For each dataset, this
            // function will call to register the dataset
            this.InitialScanForDatasets();
            // Now we set some properties to get things rolling
            m_BrowseForSourceFolderButton.Enabled = false;
            m_StartButton.Enabled = false;
            m_StopButton.Enabled = true;
            m_HeartbeatTimer.Enabled = true;
            m_UpdateDatasetsTimer.Enabled = true;
            this.m_HeartbeatTimer_Tick(sender, e);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_StopButton_Click(
            object sender,
            EventArgs e
            )
        {
            m_BrowseForSourceFolderButton.Enabled = true;
            m_StartButton.Enabled = true;
            m_StopButton.Enabled = false;
            m_HeartbeatTimer.Enabled = false;
            m_UpdateDatasetsTimer.Enabled = false;
            // Put a notification
            this.AddNotification(DateTime.UtcNow.ToString("G") + " (UTC) : Remote Data Connector stopped");
        }

        /// <summary>
        /// 
        /// </summary>
        private void InitialScanForDatasets()
        {
            string[] listOfDatasets = System.IO.Directory.GetFiles(m_SourceFolderTextBox.Text, "*.csvp");
            foreach (string datasetFilePath in listOfDatasets)
            {
                string filename = datasetFilePath.ToLower();
                m_Mutex.WaitOne();
                SailWebApiPortalInterop.RemoteDataConnectorAddDataset(filename);
                m_ListOfRegisteredDatasets.Add(filename);
                m_Mutex.ReleaseMutex();
            }
            m_Mutex.WaitOne();
            int numberOfDatasetsRegistered = SailWebApiPortalInterop.RemoteDataConnectorUpdateDatasets();
            m_Mutex.ReleaseMutex();
            if (0 < numberOfDatasetsRegistered)
            {
                // Put a notification
                this.AddNotification(DateTime.UtcNow.ToString("G") + " (UTC) : " + numberOfDatasetsRegistered + " datasets registered.");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ExitButton_Click(
            object sender,
            EventArgs e
            )
        {
            this.Close();
        }

        /// <summary>
        /// This function fires 10 times a second and is used to update the time
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_RefreshTimer_Tick(
            object sender,
            EventArgs e
            )
        {
            m_CurrentTimeTextBox.Text = DateTime.UtcNow.ToString("G");
        }

        /// <summary>
        /// This function should fire each 30 seconds to send a heartbeat signal to the
        /// API Portal.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_HeartbeatTimer_Tick(
            object sender,
            EventArgs e
            )
        {
            m_Mutex.WaitOne();
            int nReturnCode = SailWebApiPortalInterop.RemoteDataConnectorHeartbeat();
            m_Mutex.ReleaseMutex();

            if (1 == nReturnCode)
            {
                m_NumberOfHeartbeats++;
                m_NumberOfHeartbeatsTextBox.Text = m_NumberOfHeartbeats.ToString();
                m_LastHeartbeartTimeTextBox.Text = DateTime.UtcNow.ToString("G");
            }
            else if (0 != nReturnCode)
            {
                m_NumberOfFailedHeartbeats++;
                m_NumberOfFailedHeartbeatsTextBox.Text = m_NumberOfFailedHeartbeats.ToString();
                m_NumberOfFailedHeartbeatsTextBox.ClearUndo();
            }
        }

        /// <summary>
        /// This function should fire each 5 seconds and basically cause the RemoteDataConnector
        /// to connect to the API Portal if datasets need to be updated.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_UpdateDatasetsTimer_Tick(
            object sender,
            EventArgs e
            )
        {
            int numberOfDeletedDatasets = 0;
            // First we figure out if any files have been deleted
            HashSet<string> listOfDeletedDatasets = new HashSet<string>();
            m_Mutex.WaitOne();
            foreach (string registeredFilename in m_ListOfRegisteredDatasets)
            {
                if (false == System.IO.File.Exists(registeredFilename))
                {
                    SailWebApiPortalInterop.RemoteDataConnectorRemoveDataset(registeredFilename);
                    listOfDeletedDatasets.Add(registeredFilename);
                    numberOfDeletedDatasets++;
                }
            }
            m_Mutex.ReleaseMutex();
            if (0 < numberOfDeletedDatasets)
            {
                // Delete the entries
                foreach (string registeredFilename in listOfDeletedDatasets)
                {
                    m_ListOfRegisteredDatasets.Remove(registeredFilename);
                }
                // Put a notification
                this.AddNotification(DateTime.UtcNow.ToString("G") + " (UTC) : " + numberOfDeletedDatasets + " datasets removed (unregistered).");
            }
            // Now we need to figure out if any new files were added. We list all files and then
            // compare that list with m_ListOfRegisteredDatasets
            string[] listOfDatasets = System.IO.Directory.GetFiles(m_SourceFolderTextBox.Text, "*.csvp");
            foreach (string datasetFilePath in listOfDatasets)
            {
                string filename = datasetFilePath.ToLower();
                m_Mutex.WaitOne();
                bool isRegistered = m_ListOfRegisteredDatasets.Contains(filename);
                m_Mutex.ReleaseMutex();
                if (false == isRegistered)
                {
                    m_Mutex.WaitOne();
                    SailWebApiPortalInterop.RemoteDataConnectorAddDataset(filename);
                    m_ListOfRegisteredDatasets.Add(filename);
                    m_Mutex.ReleaseMutex();
                }
            }
            m_Mutex.WaitOne();
            int numberOfDatasetsRegistered = SailWebApiPortalInterop.RemoteDataConnectorUpdateDatasets();
            m_Mutex.ReleaseMutex();
            if (0 < numberOfDatasetsRegistered)
            {
                // Put a notification
                this.AddNotification(DateTime.UtcNow.ToString("G") + " (UTC) : " + numberOfDatasetsRegistered + " datasets registered.");
            }
        }

        /// <summary>
        /// This function is used to add a notification in the notification area.
        /// </summary>
        /// <param name="notification"></param>
        private void AddNotification(
            string notification
            )
        {
            m_NotificationsTextBox.BeginUpdate();
            m_NotificationsTextBox.Items.Add(notification);
            m_NotificationsTextBox.EndUpdate();
        }

        private System.Threading.Mutex m_Mutex;
        private int m_NumberOfHeartbeats;
        private int m_NumberOfFailedHeartbeats;
        private HashSet<string> m_ListOfRegisteredDatasets;
    }
}
