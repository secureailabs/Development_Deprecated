
namespace MicrosoftAzureVirtualMachineInitializer
{
    partial class InitializerOnlySettingsDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(InitializerOnlySettingsDialog));
            this.m_DigitalContractsComboBox = new System.Windows.Forms.ComboBox();
            this.m_DatasetFilenameTextBox = new System.Windows.Forms.TextBox();
            this.m_BrowseLink = new System.Windows.Forms.LinkLabel();
            this.m_DigitalContractLabel = new System.Windows.Forms.Label();
            this.m_DatasetFilenameLabel = new System.Windows.Forms.Label();
            this.m_IpAddressLabel = new System.Windows.Forms.Label();
            this.m_SailLogoPictureBox = new System.Windows.Forms.PictureBox();
            this.m_PreviousButton = new System.Windows.Forms.Button();
            this.m_NextButton = new System.Windows.Forms.Button();
            this.m_StatusStrip = new System.Windows.Forms.StatusStrip();
            this.m_CopyrightStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_ExceptionsToolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_OpenFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.m_RefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.m_IpAddressTextBox = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.m_SailLogoPictureBox)).BeginInit();
            this.m_StatusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_DigitalContractsComboBox
            // 
            this.m_DigitalContractsComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.m_DigitalContractsComboBox.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_DigitalContractsComboBox.FormattingEnabled = true;
            this.m_DigitalContractsComboBox.Location = new System.Drawing.Point(271, 10);
            this.m_DigitalContractsComboBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.m_DigitalContractsComboBox.Name = "m_DigitalContractsComboBox";
            this.m_DigitalContractsComboBox.Size = new System.Drawing.Size(497, 22);
            this.m_DigitalContractsComboBox.TabIndex = 0;
            // 
            // m_DatasetFilenameTextBox
            // 
            this.m_DatasetFilenameTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_DatasetFilenameTextBox.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_DatasetFilenameTextBox.Location = new System.Drawing.Point(271, 39);
            this.m_DatasetFilenameTextBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.m_DatasetFilenameTextBox.Name = "m_DatasetFilenameTextBox";
            this.m_DatasetFilenameTextBox.ReadOnly = true;
            this.m_DatasetFilenameTextBox.Size = new System.Drawing.Size(438, 22);
            this.m_DatasetFilenameTextBox.TabIndex = 1;
            // 
            // m_BrowseLink
            // 
            this.m_BrowseLink.AutoSize = true;
            this.m_BrowseLink.Location = new System.Drawing.Point(715, 42);
            this.m_BrowseLink.Name = "m_BrowseLink";
            this.m_BrowseLink.Size = new System.Drawing.Size(54, 15);
            this.m_BrowseLink.TabIndex = 3;
            this.m_BrowseLink.TabStop = true;
            this.m_BrowseLink.Text = "Browse...";
            this.m_BrowseLink.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.m_BrowseLink_LinkClicked);
            // 
            // m_DigitalContractLabel
            // 
            this.m_DigitalContractLabel.AutoSize = true;
            this.m_DigitalContractLabel.Location = new System.Drawing.Point(135, 13);
            this.m_DigitalContractLabel.Name = "m_DigitalContractLabel";
            this.m_DigitalContractLabel.Size = new System.Drawing.Size(133, 15);
            this.m_DigitalContractLabel.TabIndex = 4;
            this.m_DigitalContractLabel.Text = "Select a Digital Contract\r\n";
            // 
            // m_DatasetFilenameLabel
            // 
            this.m_DatasetFilenameLabel.AutoSize = true;
            this.m_DatasetFilenameLabel.Location = new System.Drawing.Point(156, 42);
            this.m_DatasetFilenameLabel.Name = "m_DatasetFilenameLabel";
            this.m_DatasetFilenameLabel.Size = new System.Drawing.Size(110, 15);
            this.m_DatasetFilenameLabel.TabIndex = 5;
            this.m_DatasetFilenameLabel.Text = "Select a Dataset File";
            // 
            // m_IpAddressLabel
            // 
            this.m_IpAddressLabel.AutoSize = true;
            this.m_IpAddressLabel.Location = new System.Drawing.Point(205, 74);
            this.m_IpAddressLabel.Name = "m_IpAddressLabel";
            this.m_IpAddressLabel.Size = new System.Drawing.Size(62, 15);
            this.m_IpAddressLabel.TabIndex = 6;
            this.m_IpAddressLabel.Text = "IP Address";
            // 
            // m_SailLogoPictureBox
            // 
            this.m_SailLogoPictureBox.Image = global::MicrosoftAzureInitializer.Properties.Resources.SAIL_Icon1;
            this.m_SailLogoPictureBox.Location = new System.Drawing.Point(11, 10);
            this.m_SailLogoPictureBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.m_SailLogoPictureBox.Name = "m_SailLogoPictureBox";
            this.m_SailLogoPictureBox.Size = new System.Drawing.Size(120, 120);
            this.m_SailLogoPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.m_SailLogoPictureBox.TabIndex = 9;
            this.m_SailLogoPictureBox.TabStop = false;
            // 
            // m_PreviousButton
            // 
            this.m_PreviousButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_PreviousButton.Location = new System.Drawing.Point(412, 104);
            this.m_PreviousButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.m_PreviousButton.Name = "m_PreviousButton";
            this.m_PreviousButton.Size = new System.Drawing.Size(135, 26);
            this.m_PreviousButton.TabIndex = 11;
            this.m_PreviousButton.Text = "Cancel";
            this.m_PreviousButton.UseVisualStyleBackColor = true;
            this.m_PreviousButton.Click += new System.EventHandler(this.m_PreviousButton_Click);
            // 
            // m_NextButton
            // 
            this.m_NextButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.m_NextButton.Enabled = false;
            this.m_NextButton.Location = new System.Drawing.Point(271, 104);
            this.m_NextButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.m_NextButton.Name = "m_NextButton";
            this.m_NextButton.Size = new System.Drawing.Size(135, 26);
            this.m_NextButton.TabIndex = 10;
            this.m_NextButton.Text = "Next";
            this.m_NextButton.UseVisualStyleBackColor = true;
            this.m_NextButton.Click += new System.EventHandler(this.m_NextButton_Click);
            // 
            // m_StatusStrip
            // 
            this.m_StatusStrip.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_StatusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_CopyrightStripStatusLabel,
            this.m_ExceptionsToolStripStatusLabel});
            this.m_StatusStrip.LayoutStyle = System.Windows.Forms.ToolStripLayoutStyle.HorizontalStackWithOverflow;
            this.m_StatusStrip.Location = new System.Drawing.Point(0, 141);
            this.m_StatusStrip.Name = "m_StatusStrip";
            this.m_StatusStrip.Padding = new System.Windows.Forms.Padding(1, 0, 13, 0);
            this.m_StatusStrip.Size = new System.Drawing.Size(784, 22);
            this.m_StatusStrip.TabIndex = 12;
            this.m_StatusStrip.Text = "statusStrip1";
            // 
            // m_CopyrightStripStatusLabel
            // 
            this.m_CopyrightStripStatusLabel.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_CopyrightStripStatusLabel.Name = "m_CopyrightStripStatusLabel";
            this.m_CopyrightStripStatusLabel.Size = new System.Drawing.Size(280, 17);
            this.m_CopyrightStripStatusLabel.Text = "Copyright (C) 2021 Secure AI Labs, Inc.";
            this.m_CopyrightStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_ExceptionsToolStripStatusLabel
            // 
            this.m_ExceptionsToolStripStatusLabel.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.m_ExceptionsToolStripStatusLabel.IsLink = true;
            this.m_ExceptionsToolStripStatusLabel.LinkColor = System.Drawing.Color.Red;
            this.m_ExceptionsToolStripStatusLabel.Name = "m_ExceptionsToolStripStatusLabel";
            this.m_ExceptionsToolStripStatusLabel.Size = new System.Drawing.Size(210, 17);
            this.m_ExceptionsToolStripStatusLabel.Text = "Click here to view exceptions";
            this.m_ExceptionsToolStripStatusLabel.Click += new System.EventHandler(this.m_ExceptionsToolStripStatusLabel_Click);
            // 
            // m_OpenFileDialog
            // 
            this.m_OpenFileDialog.DefaultExt = "csvp";
            this.m_OpenFileDialog.Filter = "Dataset File|*.csvp";
            this.m_OpenFileDialog.Title = "Select Dataset File";
            // 
            // m_RefreshTimer
            // 
            this.m_RefreshTimer.Enabled = true;
            this.m_RefreshTimer.Tick += new System.EventHandler(this.m_RefreshTimer_Tick);
            // 
            // m_IpAddressTextBox
            // 
            this.m_IpAddressTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_IpAddressTextBox.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_IpAddressTextBox.Location = new System.Drawing.Point(271, 71);
            this.m_IpAddressTextBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.m_IpAddressTextBox.Name = "m_IpAddressTextBox";
            this.m_IpAddressTextBox.Size = new System.Drawing.Size(162, 22);
            this.m_IpAddressTextBox.TabIndex = 13;
            // 
            // InitializerOnlySettingsDialog
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(784, 163);
            this.ControlBox = false;
            this.Controls.Add(this.m_IpAddressTextBox);
            this.Controls.Add(this.m_StatusStrip);
            this.Controls.Add(this.m_PreviousButton);
            this.Controls.Add(this.m_NextButton);
            this.Controls.Add(this.m_SailLogoPictureBox);
            this.Controls.Add(this.m_IpAddressLabel);
            this.Controls.Add(this.m_DatasetFilenameLabel);
            this.Controls.Add(this.m_DigitalContractLabel);
            this.Controls.Add(this.m_BrowseLink);
            this.Controls.Add(this.m_DatasetFilenameTextBox);
            this.Controls.Add(this.m_DigitalContractsComboBox);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.Name = "InitializerOnlySettingsDialog";
            this.Text = "Ignite! A Microsoft Azure Initializer.";
            ((System.ComponentModel.ISupportInitialize)(this.m_SailLogoPictureBox)).EndInit();
            this.m_StatusStrip.ResumeLayout(false);
            this.m_StatusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox m_DigitalContractsComboBox;
        private System.Windows.Forms.TextBox m_DatasetFilenameTextBox;
        private System.Windows.Forms.LinkLabel m_BrowseLink;
        private System.Windows.Forms.Label m_DigitalContractLabel;
        private System.Windows.Forms.Label m_DatasetFilenameLabel;
        private System.Windows.Forms.Label m_IpAddressLabel;
        private System.Windows.Forms.PictureBox m_SailLogoPictureBox;
        private System.Windows.Forms.Button m_PreviousButton;
        private System.Windows.Forms.Button m_NextButton;
        private System.Windows.Forms.StatusStrip m_StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel m_CopyrightStripStatusLabel;
        private System.Windows.Forms.ToolStripStatusLabel m_ExceptionsToolStripStatusLabel;
        private System.Windows.Forms.OpenFileDialog m_OpenFileDialog;
        private System.Windows.Forms.Timer m_RefreshTimer;
        private System.Windows.Forms.TextBox m_IpAddressTextBox;
    }
}