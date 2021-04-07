
namespace MicrosoftAzureVirtualMachineInitializer
{
    partial class ManualSettingsDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ManualSettingsDialog));
            this.m_DigitalContractsComboBox = new System.Windows.Forms.ComboBox();
            this.m_DigitalContractLabel = new System.Windows.Forms.Label();
            this.m_DatasetFilenameTextBox = new System.Windows.Forms.TextBox();
            this.m_BrowseForDatasetFilenameLinkLabel = new System.Windows.Forms.LinkLabel();
            this.m_DatasetFilenameLabel = new System.Windows.Forms.Label();
            this.m_StatusStrip = new System.Windows.Forms.StatusStrip();
            this.m_CopyrightStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_HelpStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_ExceptionsToolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_VirtualMachineCountLabel = new System.Windows.Forms.Label();
            this.m_SailLogoPictureBox = new System.Windows.Forms.PictureBox();
            this.m_NextButton = new System.Windows.Forms.Button();
            this.m_PreviousButton = new System.Windows.Forms.Button();
            this.m_VirtualMachineCountNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.m_OpenFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.m_SubscriptionIdentifierTextBox = new System.Windows.Forms.TextBox();
            this.m_ResourceGroupTextBox = new System.Windows.Forms.TextBox();
            this.m_LocationTextBox = new System.Windows.Forms.TextBox();
            this.m_VirtualNetworkTextBox = new System.Windows.Forms.TextBox();
            this.m_NetworkSecurityGroupTextBox = new System.Windows.Forms.TextBox();
            this.m_SubscriptionIdentifierLabel = new System.Windows.Forms.Label();
            this.m_ResourceGroupLabel = new System.Windows.Forms.Label();
            this.m_LocationLabel = new System.Windows.Forms.Label();
            this.m_VirtualNetworkLabel = new System.Windows.Forms.Label();
            this.m_NetworkSecurityGroupLabel = new System.Windows.Forms.Label();
            this.m_RefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.m_BaseMachineNameTextBox = new System.Windows.Forms.TextBox();
            this.m_VirtualMachineSizeTextBox = new System.Windows.Forms.TextBox();
            this.m_BaseMachineNameLabel = new System.Windows.Forms.Label();
            this.m_VirtualMachineSizeLabel = new System.Windows.Forms.Label();
            this.m_StatusStrip.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.m_SailLogoPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.m_VirtualMachineCountNumericUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // m_DigitalContractsComboBox
            // 
            this.m_DigitalContractsComboBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_DigitalContractsComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.m_DigitalContractsComboBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_DigitalContractsComboBox.FormattingEnabled = true;
            this.m_DigitalContractsComboBox.Location = new System.Drawing.Point(443, 12);
            this.m_DigitalContractsComboBox.Name = "m_DigitalContractsComboBox";
            this.m_DigitalContractsComboBox.Size = new System.Drawing.Size(552, 30);
            this.m_DigitalContractsComboBox.Sorted = true;
            this.m_DigitalContractsComboBox.TabIndex = 0;
            // 
            // m_DigitalContractLabel
            // 
            this.m_DigitalContractLabel.AutoSize = true;
            this.m_DigitalContractLabel.Location = new System.Drawing.Point(237, 13);
            this.m_DigitalContractLabel.Name = "m_DigitalContractLabel";
            this.m_DigitalContractLabel.Size = new System.Drawing.Size(200, 25);
            this.m_DigitalContractLabel.TabIndex = 1;
            this.m_DigitalContractLabel.Text = "Select a Digital Contract\r\n";
            // 
            // m_DatasetFilenameTextBox
            // 
            this.m_DatasetFilenameTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_DatasetFilenameTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_DatasetFilenameTextBox.Location = new System.Drawing.Point(443, 51);
            this.m_DatasetFilenameTextBox.Name = "m_DatasetFilenameTextBox";
            this.m_DatasetFilenameTextBox.ReadOnly = true;
            this.m_DatasetFilenameTextBox.Size = new System.Drawing.Size(465, 29);
            this.m_DatasetFilenameTextBox.TabIndex = 1;
            this.m_DatasetFilenameTextBox.TextChanged += new System.EventHandler(this.m_DatasetFilenameTextBox_TextChanged);
            // 
            // m_BrowseForDatasetFilenameLinkLabel
            // 
            this.m_BrowseForDatasetFilenameLinkLabel.AutoSize = true;
            this.m_BrowseForDatasetFilenameLinkLabel.Location = new System.Drawing.Point(914, 53);
            this.m_BrowseForDatasetFilenameLinkLabel.Name = "m_BrowseForDatasetFilenameLinkLabel";
            this.m_BrowseForDatasetFilenameLinkLabel.Size = new System.Drawing.Size(81, 25);
            this.m_BrowseForDatasetFilenameLinkLabel.TabIndex = 3;
            this.m_BrowseForDatasetFilenameLinkLabel.TabStop = true;
            this.m_BrowseForDatasetFilenameLinkLabel.Text = "Browse...";
            this.m_BrowseForDatasetFilenameLinkLabel.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.m_BrowseForDatasetFilenameLinkLabel_LinkClicked);
            // 
            // m_DatasetFilenameLabel
            // 
            this.m_DatasetFilenameLabel.AutoSize = true;
            this.m_DatasetFilenameLabel.Location = new System.Drawing.Point(269, 51);
            this.m_DatasetFilenameLabel.Name = "m_DatasetFilenameLabel";
            this.m_DatasetFilenameLabel.Size = new System.Drawing.Size(168, 25);
            this.m_DatasetFilenameLabel.TabIndex = 4;
            this.m_DatasetFilenameLabel.Text = "Select a Dataset File";
            // 
            // m_StatusStrip
            // 
            this.m_StatusStrip.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_StatusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_CopyrightStripStatusLabel,
            this.m_HelpStripStatusLabel,
            this.m_ExceptionsToolStripStatusLabel});
            this.m_StatusStrip.LayoutStyle = System.Windows.Forms.ToolStripLayoutStyle.HorizontalStackWithOverflow;
            this.m_StatusStrip.Location = new System.Drawing.Point(0, 482);
            this.m_StatusStrip.Name = "m_StatusStrip";
            this.m_StatusStrip.Size = new System.Drawing.Size(1014, 29);
            this.m_StatusStrip.TabIndex = 6;
            this.m_StatusStrip.Text = "statusStrip1";
            // 
            // m_CopyrightStripStatusLabel
            // 
            this.m_CopyrightStripStatusLabel.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_CopyrightStripStatusLabel.Name = "m_CopyrightStripStatusLabel";
            this.m_CopyrightStripStatusLabel.Size = new System.Drawing.Size(400, 22);
            this.m_CopyrightStripStatusLabel.Text = "Copyright (C) 2021 Secure AI Labs, Inc.";
            this.m_CopyrightStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_HelpStripStatusLabel
            // 
            this.m_HelpStripStatusLabel.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_HelpStripStatusLabel.IsLink = true;
            this.m_HelpStripStatusLabel.Name = "m_HelpStripStatusLabel";
            this.m_HelpStripStatusLabel.Size = new System.Drawing.Size(50, 22);
            this.m_HelpStripStatusLabel.Text = "Help";
            this.m_HelpStripStatusLabel.Click += new System.EventHandler(this.m_HelpStripStatusLabel_Click);
            // 
            // m_ExceptionsToolStripStatusLabel
            // 
            this.m_ExceptionsToolStripStatusLabel.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.m_ExceptionsToolStripStatusLabel.IsLink = true;
            this.m_ExceptionsToolStripStatusLabel.LinkColor = System.Drawing.Color.Red;
            this.m_ExceptionsToolStripStatusLabel.Name = "m_ExceptionsToolStripStatusLabel";
            this.m_ExceptionsToolStripStatusLabel.Size = new System.Drawing.Size(300, 22);
            this.m_ExceptionsToolStripStatusLabel.Text = "Click here to view exceptions";
            this.m_ExceptionsToolStripStatusLabel.Click += new System.EventHandler(this.m_ExceptionsToolStripStatusLabel_Click);

            // 
            // m_VirtualMachineCountLabel
            // 
            this.m_VirtualMachineCountLabel.AutoSize = true;
            this.m_VirtualMachineCountLabel.Location = new System.Drawing.Point(251, 90);
            this.m_VirtualMachineCountLabel.Name = "m_VirtualMachineCountLabel";
            this.m_VirtualMachineCountLabel.Size = new System.Drawing.Size(186, 25);
            this.m_VirtualMachineCountLabel.TabIndex = 7;
            this.m_VirtualMachineCountLabel.Text = "Virtual Machine Count";
            // 
            // m_SailLogoPictureBox
            // 
            this.m_SailLogoPictureBox.Image = global::MicrosoftAzureVirtualMachineInitializer.Properties.Resources.SAIL_Icon;
            this.m_SailLogoPictureBox.Location = new System.Drawing.Point(27, 157);
            this.m_SailLogoPictureBox.Name = "m_SailLogoPictureBox";
            this.m_SailLogoPictureBox.Size = new System.Drawing.Size(178, 170);
            this.m_SailLogoPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.m_SailLogoPictureBox.TabIndex = 8;
            this.m_SailLogoPictureBox.TabStop = false;
            // 
            // m_NextButton
            // 
            this.m_NextButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.m_NextButton.Enabled = false;
            this.m_NextButton.Location = new System.Drawing.Point(344, 411);
            this.m_NextButton.Name = "m_NextButton";
            this.m_NextButton.Size = new System.Drawing.Size(150, 44);
            this.m_NextButton.TabIndex = 3;
            this.m_NextButton.Text = "Next";
            this.m_NextButton.UseVisualStyleBackColor = true;
            this.m_NextButton.Click += new System.EventHandler(this.m_NextButton_Click);
            // 
            // m_PreviousButton
            // 
            this.m_PreviousButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_PreviousButton.Location = new System.Drawing.Point(520, 411);
            this.m_PreviousButton.Name = "m_PreviousButton";
            this.m_PreviousButton.Size = new System.Drawing.Size(150, 44);
            this.m_PreviousButton.TabIndex = 4;
            this.m_PreviousButton.Text = "Cancel";
            this.m_PreviousButton.UseVisualStyleBackColor = true;
            this.m_PreviousButton.Click += new System.EventHandler(this.m_PreviousButton_Click);
            // 
            // m_VirtualMachineCountNumericUpDown
            // 
            this.m_VirtualMachineCountNumericUpDown.BackColor = System.Drawing.SystemColors.Info;
            this.m_VirtualMachineCountNumericUpDown.Location = new System.Drawing.Point(443, 89);
            this.m_VirtualMachineCountNumericUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.m_VirtualMachineCountNumericUpDown.Name = "m_VirtualMachineCountNumericUpDown";
            this.m_VirtualMachineCountNumericUpDown.Size = new System.Drawing.Size(180, 31);
            this.m_VirtualMachineCountNumericUpDown.TabIndex = 2;
            this.m_VirtualMachineCountNumericUpDown.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.m_VirtualMachineCountNumericUpDown.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // m_OpenFileDialog
            // 
            this.m_OpenFileDialog.DefaultExt = "csvp";
            this.m_OpenFileDialog.Filter = "Dataset File|*.csvp";
            this.m_OpenFileDialog.Title = "Select Dataset File";
            // 
            // m_SubscriptionIdentifierTextBox
            // 
            this.m_SubscriptionIdentifierTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_SubscriptionIdentifierTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_SubscriptionIdentifierTextBox.Location = new System.Drawing.Point(443, 129);
            this.m_SubscriptionIdentifierTextBox.Name = "m_SubscriptionIdentifierTextBox";
            this.m_SubscriptionIdentifierTextBox.Size = new System.Drawing.Size(552, 29);
            this.m_SubscriptionIdentifierTextBox.TabIndex = 9;
            // 
            // m_ResourceGroupTextBox
            // 
            this.m_ResourceGroupTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_ResourceGroupTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_ResourceGroupTextBox.Location = new System.Drawing.Point(443, 167);
            this.m_ResourceGroupTextBox.Name = "m_ResourceGroupTextBox";
            this.m_ResourceGroupTextBox.Size = new System.Drawing.Size(552, 29);
            this.m_ResourceGroupTextBox.TabIndex = 10;
            // 
            // m_LocationTextBox
            // 
            this.m_LocationTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_LocationTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_LocationTextBox.Location = new System.Drawing.Point(443, 205);
            this.m_LocationTextBox.Name = "m_LocationTextBox";
            this.m_LocationTextBox.Size = new System.Drawing.Size(552, 29);
            this.m_LocationTextBox.TabIndex = 11;
            // 
            // m_VirtualNetworkTextBox
            // 
            this.m_VirtualNetworkTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_VirtualNetworkTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_VirtualNetworkTextBox.Location = new System.Drawing.Point(443, 243);
            this.m_VirtualNetworkTextBox.Name = "m_VirtualNetworkTextBox";
            this.m_VirtualNetworkTextBox.Size = new System.Drawing.Size(552, 29);
            this.m_VirtualNetworkTextBox.TabIndex = 12;
            // 
            // m_NetworkSecurityGroupTextBox
            // 
            this.m_NetworkSecurityGroupTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_NetworkSecurityGroupTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_NetworkSecurityGroupTextBox.Location = new System.Drawing.Point(443, 281);
            this.m_NetworkSecurityGroupTextBox.Name = "m_NetworkSecurityGroupTextBox";
            this.m_NetworkSecurityGroupTextBox.Size = new System.Drawing.Size(552, 29);
            this.m_NetworkSecurityGroupTextBox.TabIndex = 13;
            // 
            // m_SubscriptionIdentifierLabel
            // 
            this.m_SubscriptionIdentifierLabel.AutoSize = true;
            this.m_SubscriptionIdentifierLabel.Location = new System.Drawing.Point(251, 129);
            this.m_SubscriptionIdentifierLabel.Name = "m_SubscriptionIdentifierLabel";
            this.m_SubscriptionIdentifierLabel.Size = new System.Drawing.Size(186, 25);
            this.m_SubscriptionIdentifierLabel.TabIndex = 14;
            this.m_SubscriptionIdentifierLabel.Text = "Subscription Identifier";
            // 
            // m_ResourceGroupLabel
            // 
            this.m_ResourceGroupLabel.AutoSize = true;
            this.m_ResourceGroupLabel.Location = new System.Drawing.Point(299, 167);
            this.m_ResourceGroupLabel.Name = "m_ResourceGroupLabel";
            this.m_ResourceGroupLabel.Size = new System.Drawing.Size(138, 25);
            this.m_ResourceGroupLabel.TabIndex = 15;
            this.m_ResourceGroupLabel.Text = "Resource Group";
            // 
            // m_LocationLabel
            // 
            this.m_LocationLabel.AutoSize = true;
            this.m_LocationLabel.Location = new System.Drawing.Point(358, 205);
            this.m_LocationLabel.Name = "m_LocationLabel";
            this.m_LocationLabel.Size = new System.Drawing.Size(79, 25);
            this.m_LocationLabel.TabIndex = 16;
            this.m_LocationLabel.Text = "Location";
            // 
            // m_VirtualNetworkLabel
            // 
            this.m_VirtualNetworkLabel.AutoSize = true;
            this.m_VirtualNetworkLabel.Location = new System.Drawing.Point(303, 243);
            this.m_VirtualNetworkLabel.Name = "m_VirtualNetworkLabel";
            this.m_VirtualNetworkLabel.Size = new System.Drawing.Size(134, 25);
            this.m_VirtualNetworkLabel.TabIndex = 17;
            this.m_VirtualNetworkLabel.Text = "Virtual Network";
            // 
            // m_NetworkSecurityGroupLabel
            // 
            this.m_NetworkSecurityGroupLabel.AutoSize = true;
            this.m_NetworkSecurityGroupLabel.Location = new System.Drawing.Point(236, 281);
            this.m_NetworkSecurityGroupLabel.Name = "m_NetworkSecurityGroupLabel";
            this.m_NetworkSecurityGroupLabel.Size = new System.Drawing.Size(201, 25);
            this.m_NetworkSecurityGroupLabel.TabIndex = 18;
            this.m_NetworkSecurityGroupLabel.Text = "Network Security Group";
            // 
            // m_RefreshTimer
            // 
            this.m_RefreshTimer.Enabled = true;
            this.m_RefreshTimer.Tick += new System.EventHandler(this.m_RefreshTimer_Tick);
            // 
            // m_BaseMachineNameTextBox
            // 
            this.m_BaseMachineNameTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_BaseMachineNameTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_BaseMachineNameTextBox.Location = new System.Drawing.Point(443, 319);
            this.m_BaseMachineNameTextBox.Name = "m_BaseMachineNameTextBox";
            this.m_BaseMachineNameTextBox.Size = new System.Drawing.Size(552, 29);
            this.m_BaseMachineNameTextBox.TabIndex = 19;
            // 
            // m_VirtualMachineSizeTextBox
            // 
            this.m_VirtualMachineSizeTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_VirtualMachineSizeTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_VirtualMachineSizeTextBox.Location = new System.Drawing.Point(443, 357);
            this.m_VirtualMachineSizeTextBox.Name = "m_VirtualMachineSizeTextBox";
            this.m_VirtualMachineSizeTextBox.Size = new System.Drawing.Size(552, 29);
            this.m_VirtualMachineSizeTextBox.TabIndex = 20;
            // 
            // m_BaseMachineNameLabel
            // 
            this.m_BaseMachineNameLabel.AutoSize = true;
            this.m_BaseMachineNameLabel.Location = new System.Drawing.Point(266, 319);
            this.m_BaseMachineNameLabel.Name = "m_BaseMachineNameLabel";
            this.m_BaseMachineNameLabel.Size = new System.Drawing.Size(171, 25);
            this.m_BaseMachineNameLabel.TabIndex = 21;
            this.m_BaseMachineNameLabel.Text = "Base Machine Name";
            // 
            // m_VirtualMachineSizeLabel
            // 
            this.m_VirtualMachineSizeLabel.AutoSize = true;
            this.m_VirtualMachineSizeLabel.Location = new System.Drawing.Point(268, 357);
            this.m_VirtualMachineSizeLabel.Name = "m_VirtualMachineSizeLabel";
            this.m_VirtualMachineSizeLabel.Size = new System.Drawing.Size(169, 25);
            this.m_VirtualMachineSizeLabel.TabIndex = 22;
            this.m_VirtualMachineSizeLabel.Text = "Virtual Machine Size";
            // 
            // ManualSettingsDialog
            // 
            this.AcceptButton = this.m_NextButton;
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(1014, 511);
            this.ControlBox = false;
            this.Controls.Add(this.m_VirtualMachineSizeLabel);
            this.Controls.Add(this.m_BaseMachineNameLabel);
            this.Controls.Add(this.m_VirtualMachineSizeTextBox);
            this.Controls.Add(this.m_BaseMachineNameTextBox);
            this.Controls.Add(this.m_NetworkSecurityGroupLabel);
            this.Controls.Add(this.m_VirtualNetworkLabel);
            this.Controls.Add(this.m_LocationLabel);
            this.Controls.Add(this.m_ResourceGroupLabel);
            this.Controls.Add(this.m_SubscriptionIdentifierLabel);
            this.Controls.Add(this.m_NetworkSecurityGroupTextBox);
            this.Controls.Add(this.m_VirtualNetworkTextBox);
            this.Controls.Add(this.m_LocationTextBox);
            this.Controls.Add(this.m_ResourceGroupTextBox);
            this.Controls.Add(this.m_SubscriptionIdentifierTextBox);
            this.Controls.Add(this.m_VirtualMachineCountNumericUpDown);
            this.Controls.Add(this.m_PreviousButton);
            this.Controls.Add(this.m_NextButton);
            this.Controls.Add(this.m_SailLogoPictureBox);
            this.Controls.Add(this.m_VirtualMachineCountLabel);
            this.Controls.Add(this.m_StatusStrip);
            this.Controls.Add(this.m_DatasetFilenameLabel);
            this.Controls.Add(this.m_BrowseForDatasetFilenameLinkLabel);
            this.Controls.Add(this.m_DatasetFilenameTextBox);
            this.Controls.Add(this.m_DigitalContractLabel);
            this.Controls.Add(this.m_DigitalContractsComboBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "ManualSettingsDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Manual Virtual Machine And Microsoft Azure Configuration";
            this.m_StatusStrip.ResumeLayout(false);
            this.m_StatusStrip.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.m_SailLogoPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.m_VirtualMachineCountNumericUpDown)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox m_DigitalContractsComboBox;
        private System.Windows.Forms.Label m_DigitalContractLabel;
        private System.Windows.Forms.TextBox m_DatasetFilenameTextBox;
        private System.Windows.Forms.LinkLabel m_BrowseForDatasetFilenameLinkLabel;
        private System.Windows.Forms.Label m_DatasetFilenameLabel;
        private System.Windows.Forms.StatusStrip m_StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel m_CopyrightStripStatusLabel;
        private System.Windows.Forms.Label m_VirtualMachineCountLabel;
        private System.Windows.Forms.PictureBox m_SailLogoPictureBox;
        private System.Windows.Forms.Button m_NextButton;
        private System.Windows.Forms.Button m_PreviousButton;
        private System.Windows.Forms.ToolStripStatusLabel m_HelpStripStatusLabel;
        private System.Windows.Forms.NumericUpDown m_VirtualMachineCountNumericUpDown;
        private System.Windows.Forms.OpenFileDialog m_OpenFileDialog;
        private System.Windows.Forms.TextBox m_SubscriptionIdentifierTextBox;
        private System.Windows.Forms.TextBox m_ResourceGroupTextBox;
        private System.Windows.Forms.TextBox m_LocationTextBox;
        private System.Windows.Forms.TextBox m_VirtualNetworkTextBox;
        private System.Windows.Forms.TextBox m_NetworkSecurityGroupTextBox;
        private System.Windows.Forms.Label m_SubscriptionIdentifierLabel;
        private System.Windows.Forms.Label m_ResourceGroupLabel;
        private System.Windows.Forms.Label m_LocationLabel;
        private System.Windows.Forms.Label m_VirtualNetworkLabel;
        private System.Windows.Forms.Label m_NetworkSecurityGroupLabel;
        private System.Windows.Forms.Timer m_RefreshTimer;
        private System.Windows.Forms.TextBox m_BaseMachineNameTextBox;
        private System.Windows.Forms.TextBox m_VirtualMachineSizeTextBox;
        private System.Windows.Forms.Label m_BaseMachineNameLabel;
        private System.Windows.Forms.Label m_VirtualMachineSizeLabel;
        private System.Windows.Forms.ToolStripStatusLabel m_ExceptionsToolStripStatusLabel;
    }
}