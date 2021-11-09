
namespace WindowsRemoteDataConnector
{
    partial class SailRemoteDataConnectorDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SailRemoteDataConnectorDialog));
            this.m_SourceFolderGroupBox = new System.Windows.Forms.GroupBox();
            this.m_BrowseForSourceFolderButton = new System.Windows.Forms.Button();
            this.m_SourceFolderTextBox = new System.Windows.Forms.TextBox();
            this.m_SourceFolderInformationLabel = new System.Windows.Forms.Label();
            this.m_FolderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            this.m_RuntimeGroupBox = new System.Windows.Forms.GroupBox();
            this.m_ClearNotificationsLinkLabel = new System.Windows.Forms.LinkLabel();
            this.m_CopyLinkLabel = new System.Windows.Forms.LinkLabel();
            this.m_NotificationsTextBox = new System.Windows.Forms.ListBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.m_NumberOfFailedHeartbeatsTextBox = new System.Windows.Forms.TextBox();
            this.m_NumberOfHeartbeatsTextBox = new System.Windows.Forms.TextBox();
            this.m_LastHeartbeartTimeTextBox = new System.Windows.Forms.TextBox();
            this.m_CurrentTimeTextBox = new System.Windows.Forms.TextBox();
            this.m_StopButton = new System.Windows.Forms.Button();
            this.m_StartButton = new System.Windows.Forms.Button();
            this.m_ExitButton = new System.Windows.Forms.Button();
            this.m_RefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.m_HeartbeatTimer = new System.Windows.Forms.Timer(this.components);
            this.m_UpdateDatasetsTimer = new System.Windows.Forms.Timer(this.components);
            this.m_SourceFolderGroupBox.SuspendLayout();
            this.m_RuntimeGroupBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_SourceFolderGroupBox
            // 
            this.m_SourceFolderGroupBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_SourceFolderGroupBox.Controls.Add(this.m_BrowseForSourceFolderButton);
            this.m_SourceFolderGroupBox.Controls.Add(this.m_SourceFolderTextBox);
            this.m_SourceFolderGroupBox.Controls.Add(this.m_SourceFolderInformationLabel);
            this.m_SourceFolderGroupBox.Location = new System.Drawing.Point(12, 12);
            this.m_SourceFolderGroupBox.Name = "m_SourceFolderGroupBox";
            this.m_SourceFolderGroupBox.Size = new System.Drawing.Size(809, 149);
            this.m_SourceFolderGroupBox.TabIndex = 0;
            this.m_SourceFolderGroupBox.TabStop = false;
            this.m_SourceFolderGroupBox.Text = "Source Folder for Datasets";
            // 
            // m_BrowseForSourceFolderButton
            // 
            this.m_BrowseForSourceFolderButton.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_BrowseForSourceFolderButton.ImageAlign = System.Drawing.ContentAlignment.TopCenter;
            this.m_BrowseForSourceFolderButton.Location = new System.Drawing.Point(667, 107);
            this.m_BrowseForSourceFolderButton.Name = "m_BrowseForSourceFolderButton";
            this.m_BrowseForSourceFolderButton.Size = new System.Drawing.Size(121, 25);
            this.m_BrowseForSourceFolderButton.TabIndex = 2;
            this.m_BrowseForSourceFolderButton.Text = "B&rowse";
            this.m_BrowseForSourceFolderButton.UseVisualStyleBackColor = true;
            this.m_BrowseForSourceFolderButton.Click += new System.EventHandler(this.m_BrowseForSourceFolder_Click);
            // 
            // m_SourceFolderTextBox
            // 
            this.m_SourceFolderTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_SourceFolderTextBox.Location = new System.Drawing.Point(20, 107);
            this.m_SourceFolderTextBox.Name = "m_SourceFolderTextBox";
            this.m_SourceFolderTextBox.ReadOnly = true;
            this.m_SourceFolderTextBox.Size = new System.Drawing.Size(641, 25);
            this.m_SourceFolderTextBox.TabIndex = 1;
            this.m_SourceFolderTextBox.TabStop = false;
            // 
            // m_SourceFolderInformationLabel
            // 
            this.m_SourceFolderInformationLabel.AutoSize = true;
            this.m_SourceFolderInformationLabel.Font = new System.Drawing.Font("Segoe UI", 10F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_SourceFolderInformationLabel.Location = new System.Drawing.Point(13, 38);
            this.m_SourceFolderInformationLabel.Name = "m_SourceFolderInformationLabel";
            this.m_SourceFolderInformationLabel.Size = new System.Drawing.Size(782, 57);
            this.m_SourceFolderInformationLabel.TabIndex = 0;
            this.m_SourceFolderInformationLabel.Text = resources.GetString("m_SourceFolderInformationLabel.Text");
            this.m_SourceFolderInformationLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // m_FolderBrowserDialog
            // 
            this.m_FolderBrowserDialog.Description = "Select the source folder where all of the SAIL Datasets will be stored";
            this.m_FolderBrowserDialog.RootFolder = System.Environment.SpecialFolder.MyComputer;
            // 
            // m_RuntimeGroupBox
            // 
            this.m_RuntimeGroupBox.Controls.Add(this.m_ClearNotificationsLinkLabel);
            this.m_RuntimeGroupBox.Controls.Add(this.m_CopyLinkLabel);
            this.m_RuntimeGroupBox.Controls.Add(this.m_NotificationsTextBox);
            this.m_RuntimeGroupBox.Controls.Add(this.label5);
            this.m_RuntimeGroupBox.Controls.Add(this.label4);
            this.m_RuntimeGroupBox.Controls.Add(this.label3);
            this.m_RuntimeGroupBox.Controls.Add(this.label2);
            this.m_RuntimeGroupBox.Controls.Add(this.label1);
            this.m_RuntimeGroupBox.Controls.Add(this.m_NumberOfFailedHeartbeatsTextBox);
            this.m_RuntimeGroupBox.Controls.Add(this.m_NumberOfHeartbeatsTextBox);
            this.m_RuntimeGroupBox.Controls.Add(this.m_LastHeartbeartTimeTextBox);
            this.m_RuntimeGroupBox.Controls.Add(this.m_CurrentTimeTextBox);
            this.m_RuntimeGroupBox.Controls.Add(this.m_StopButton);
            this.m_RuntimeGroupBox.Controls.Add(this.m_StartButton);
            this.m_RuntimeGroupBox.Location = new System.Drawing.Point(12, 167);
            this.m_RuntimeGroupBox.Name = "m_RuntimeGroupBox";
            this.m_RuntimeGroupBox.Size = new System.Drawing.Size(808, 447);
            this.m_RuntimeGroupBox.TabIndex = 1;
            this.m_RuntimeGroupBox.TabStop = false;
            this.m_RuntimeGroupBox.Text = "Status";
            // 
            // m_ClearNotificationsLinkLabel
            // 
            this.m_ClearNotificationsLinkLabel.AutoSize = true;
            this.m_ClearNotificationsLinkLabel.Location = new System.Drawing.Point(9, 393);
            this.m_ClearNotificationsLinkLabel.Name = "m_ClearNotificationsLinkLabel";
            this.m_ClearNotificationsLinkLabel.Size = new System.Drawing.Size(120, 19);
            this.m_ClearNotificationsLinkLabel.TabIndex = 15;
            this.m_ClearNotificationsLinkLabel.TabStop = true;
            this.m_ClearNotificationsLinkLabel.Text = "Clear Notifications";
            this.m_ClearNotificationsLinkLabel.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.m_ClearNotificationsLinkLabel_LinkClicked);
            // 
            // m_CopyLinkLabel
            // 
            this.m_CopyLinkLabel.AutoSize = true;
            this.m_CopyLinkLabel.Location = new System.Drawing.Point(747, 393);
            this.m_CopyLinkLabel.Name = "m_CopyLinkLabel";
            this.m_CopyLinkLabel.Size = new System.Drawing.Size(41, 19);
            this.m_CopyLinkLabel.TabIndex = 14;
            this.m_CopyLinkLabel.TabStop = true;
            this.m_CopyLinkLabel.Text = "Copy";
            this.m_CopyLinkLabel.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.m_CopyLabel_LinkClicked);
            // 
            // m_NotificationsTextBox
            // 
            this.m_NotificationsTextBox.BackColor = System.Drawing.SystemColors.InactiveCaption;
            this.m_NotificationsTextBox.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_NotificationsTextBox.FormattingEnabled = true;
            this.m_NotificationsTextBox.HorizontalScrollbar = true;
            this.m_NotificationsTextBox.Location = new System.Drawing.Point(9, 191);
            this.m_NotificationsTextBox.Name = "m_NotificationsTextBox";
            this.m_NotificationsTextBox.Size = new System.Drawing.Size(778, 199);
            this.m_NotificationsTextBox.TabIndex = 13;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Segoe UI", 10F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(45, 36);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(719, 38);
            this.label5.TabIndex = 3;
            this.label5.Text = "Once a source folder (see above) is selected, the Remote Data Connector must be s" +
    "tarted. The RDC will keep on\r\nrunning continuously until it is stopped or until " +
    "the application is exited.\r\n";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(379, 161);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(185, 19);
            this.label4.TabIndex = 12;
            this.label4.Text = "Number of Failed Heartbeats";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(20, 161);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(146, 19);
            this.label3.TabIndex = 11;
            this.label3.Text = "Number of Heartbeats";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(395, 133);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(169, 19);
            this.label2.TabIndex = 10;
            this.label2.Text = "Last Heartbeat Time (UTC)";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(40, 133);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(126, 19);
            this.label1.TabIndex = 9;
            this.label1.Text = "Current Time (UTC)";
            // 
            // m_NumberOfFailedHeartbeatsTextBox
            // 
            this.m_NumberOfFailedHeartbeatsTextBox.BackColor = System.Drawing.SystemColors.InactiveCaption;
            this.m_NumberOfFailedHeartbeatsTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_NumberOfFailedHeartbeatsTextBox.Location = new System.Drawing.Point(570, 161);
            this.m_NumberOfFailedHeartbeatsTextBox.Name = "m_NumberOfFailedHeartbeatsTextBox";
            this.m_NumberOfFailedHeartbeatsTextBox.ReadOnly = true;
            this.m_NumberOfFailedHeartbeatsTextBox.Size = new System.Drawing.Size(109, 22);
            this.m_NumberOfFailedHeartbeatsTextBox.TabIndex = 8;
            this.m_NumberOfFailedHeartbeatsTextBox.TabStop = false;
            this.m_NumberOfFailedHeartbeatsTextBox.Text = "0";
            // 
            // m_NumberOfHeartbeatsTextBox
            // 
            this.m_NumberOfHeartbeatsTextBox.BackColor = System.Drawing.SystemColors.InactiveCaption;
            this.m_NumberOfHeartbeatsTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_NumberOfHeartbeatsTextBox.Location = new System.Drawing.Point(172, 161);
            this.m_NumberOfHeartbeatsTextBox.Name = "m_NumberOfHeartbeatsTextBox";
            this.m_NumberOfHeartbeatsTextBox.ReadOnly = true;
            this.m_NumberOfHeartbeatsTextBox.Size = new System.Drawing.Size(109, 22);
            this.m_NumberOfHeartbeatsTextBox.TabIndex = 7;
            this.m_NumberOfHeartbeatsTextBox.TabStop = false;
            this.m_NumberOfHeartbeatsTextBox.Text = "0";
            // 
            // m_LastHeartbeartTimeTextBox
            // 
            this.m_LastHeartbeartTimeTextBox.BackColor = System.Drawing.SystemColors.InactiveCaption;
            this.m_LastHeartbeartTimeTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_LastHeartbeartTimeTextBox.Location = new System.Drawing.Point(570, 133);
            this.m_LastHeartbeartTimeTextBox.Name = "m_LastHeartbeartTimeTextBox";
            this.m_LastHeartbeartTimeTextBox.ReadOnly = true;
            this.m_LastHeartbeartTimeTextBox.Size = new System.Drawing.Size(217, 22);
            this.m_LastHeartbeartTimeTextBox.TabIndex = 6;
            this.m_LastHeartbeartTimeTextBox.TabStop = false;
            // 
            // m_CurrentTimeTextBox
            // 
            this.m_CurrentTimeTextBox.BackColor = System.Drawing.SystemColors.InactiveCaption;
            this.m_CurrentTimeTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_CurrentTimeTextBox.Location = new System.Drawing.Point(172, 133);
            this.m_CurrentTimeTextBox.Name = "m_CurrentTimeTextBox";
            this.m_CurrentTimeTextBox.ReadOnly = true;
            this.m_CurrentTimeTextBox.Size = new System.Drawing.Size(217, 22);
            this.m_CurrentTimeTextBox.TabIndex = 5;
            this.m_CurrentTimeTextBox.TabStop = false;
            // 
            // m_StopButton
            // 
            this.m_StopButton.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_StopButton.ImageAlign = System.Drawing.ContentAlignment.TopCenter;
            this.m_StopButton.Location = new System.Drawing.Point(407, 89);
            this.m_StopButton.Name = "m_StopButton";
            this.m_StopButton.Size = new System.Drawing.Size(131, 25);
            this.m_StopButton.TabIndex = 4;
            this.m_StopButton.Text = "S&top";
            this.m_StopButton.UseVisualStyleBackColor = true;
            this.m_StopButton.Click += new System.EventHandler(this.m_StopButton_Click);
            // 
            // m_StartButton
            // 
            this.m_StartButton.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_StartButton.ImageAlign = System.Drawing.ContentAlignment.TopCenter;
            this.m_StartButton.Location = new System.Drawing.Point(270, 89);
            this.m_StartButton.Name = "m_StartButton";
            this.m_StartButton.Size = new System.Drawing.Size(131, 25);
            this.m_StartButton.TabIndex = 3;
            this.m_StartButton.Text = "&Start";
            this.m_StartButton.UseVisualStyleBackColor = true;
            this.m_StartButton.Click += new System.EventHandler(this.m_StartButton_Click);
            // 
            // m_ExitButton
            // 
            this.m_ExitButton.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_ExitButton.ImageAlign = System.Drawing.ContentAlignment.TopCenter;
            this.m_ExitButton.Location = new System.Drawing.Point(689, 623);
            this.m_ExitButton.Name = "m_ExitButton";
            this.m_ExitButton.Size = new System.Drawing.Size(131, 25);
            this.m_ExitButton.TabIndex = 5;
            this.m_ExitButton.Text = "E&xit";
            this.m_ExitButton.UseVisualStyleBackColor = true;
            this.m_ExitButton.Click += new System.EventHandler(this.m_ExitButton_Click);
            // 
            // m_RefreshTimer
            // 
            this.m_RefreshTimer.Enabled = true;
            this.m_RefreshTimer.Tick += new System.EventHandler(this.m_RefreshTimer_Tick);
            // 
            // m_HeartbeatTimer
            // 
            this.m_HeartbeatTimer.Interval = 30000;
            this.m_HeartbeatTimer.Tick += new System.EventHandler(this.m_HeartbeatTimer_Tick);
            // 
            // m_UpdateDatasetsTimer
            // 
            this.m_UpdateDatasetsTimer.Interval = 5000;
            this.m_UpdateDatasetsTimer.Tick += new System.EventHandler(this.m_UpdateDatasetsTimer_Tick);
            // 
            // SailRemoteDataConnectorDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 17F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(835, 654);
            this.Controls.Add(this.m_ExitButton);
            this.Controls.Add(this.m_RuntimeGroupBox);
            this.Controls.Add(this.m_SourceFolderGroupBox);
            this.Font = new System.Drawing.Font("Segoe UI", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "SailRemoteDataConnectorDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "SAIL Remote Data Connector";
            this.m_SourceFolderGroupBox.ResumeLayout(false);
            this.m_SourceFolderGroupBox.PerformLayout();
            this.m_RuntimeGroupBox.ResumeLayout(false);
            this.m_RuntimeGroupBox.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox m_SourceFolderGroupBox;
        private System.Windows.Forms.Button m_BrowseForSourceFolderButton;
        private System.Windows.Forms.TextBox m_SourceFolderTextBox;
        private System.Windows.Forms.Label m_SourceFolderInformationLabel;
        private System.Windows.Forms.FolderBrowserDialog m_FolderBrowserDialog;
        private System.Windows.Forms.GroupBox m_RuntimeGroupBox;
        private System.Windows.Forms.Button m_StopButton;
        private System.Windows.Forms.Button m_StartButton;
        private System.Windows.Forms.Button m_ExitButton;
        private System.Windows.Forms.ListBox m_NotificationsTextBox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox m_NumberOfFailedHeartbeatsTextBox;
        private System.Windows.Forms.TextBox m_NumberOfHeartbeatsTextBox;
        private System.Windows.Forms.TextBox m_LastHeartbeartTimeTextBox;
        private System.Windows.Forms.TextBox m_CurrentTimeTextBox;
        private System.Windows.Forms.Timer m_RefreshTimer;
        private System.Windows.Forms.Timer m_HeartbeatTimer;
        private System.Windows.Forms.Timer m_UpdateDatasetsTimer;
        private System.Windows.Forms.LinkLabel m_CopyLinkLabel;
        private System.Windows.Forms.LinkLabel m_ClearNotificationsLinkLabel;
    }
}