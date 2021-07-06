
namespace MicrosoftAzureVirtualMachineInitializer
{
    partial class StartupDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(StartupDialog));
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.m_ProductLabel = new System.Windows.Forms.Label();
            this.m_StatusStrip = new System.Windows.Forms.StatusStrip();
            this.m_CopyrightStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_LoadFromConfigurationButton = new System.Windows.Forms.Button();
            this.m_ManuallyConfigureButton = new System.Windows.Forms.Button();
            this.m_CloseButton = new System.Windows.Forms.Button();
            this.m_InstructionsLabel = new System.Windows.Forms.Label();
            this.m_OpenSettingsFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.m_SecureVirtualMachineGroupBox = new System.Windows.Forms.GroupBox();
            this.m_WebPortalVirtualMachineConfigurationGroupBox = new System.Windows.Forms.GroupBox();
            this.m_WebPortalGroupBox = new System.Windows.Forms.Label();
            this.m_StartWebPortalVirtualMachineButton = new System.Windows.Forms.Button();
            this.m_InitializerOnlyGroupBox = new System.Windows.Forms.GroupBox();
            this.m_InitializerButton = new System.Windows.Forms.Button();
            this.m_InitializerOnlyLabel = new System.Windows.Forms.Label();
            this.m_ConfidentialVirtualMachine = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.m_StatusStrip.SuspendLayout();
            this.m_SecureVirtualMachineGroupBox.SuspendLayout();
            this.m_WebPortalVirtualMachineConfigurationGroupBox.SuspendLayout();
            this.m_InitializerOnlyGroupBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(23, 174);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(135, 139);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // m_ProductLabel
            // 
            this.m_ProductLabel.AutoSize = true;
            this.m_ProductLabel.Font = new System.Drawing.Font("Segoe UI", 8.25F, System.Drawing.FontStyle.Italic, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_ProductLabel.Location = new System.Drawing.Point(26, 329);
            this.m_ProductLabel.Name = "m_ProductLabel";
            this.m_ProductLabel.Size = new System.Drawing.Size(132, 39);
            this.m_ProductLabel.TabIndex = 1;
            this.m_ProductLabel.Text = "Ignition\r\na Microsoft Azure Initiazer\r\nVersion 1.0.0";
            this.m_ProductLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // m_StatusStrip
            // 
            this.m_StatusStrip.AutoSize = false;
            this.m_StatusStrip.Font = new System.Drawing.Font("Segoe UI", 8.25F);
            this.m_StatusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_CopyrightStripStatusLabel});
            this.m_StatusStrip.Location = new System.Drawing.Point(0, 474);
            this.m_StatusStrip.Name = "m_StatusStrip";
            this.m_StatusStrip.Size = new System.Drawing.Size(555, 28);
            this.m_StatusStrip.TabIndex = 2;
            this.m_StatusStrip.Text = "statusStrip1";
            // 
            // m_CopyrightStripStatusLabel
            // 
            this.m_CopyrightStripStatusLabel.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_CopyrightStripStatusLabel.Name = "m_CopyrightStripStatusLabel";
            this.m_CopyrightStripStatusLabel.Size = new System.Drawing.Size(280, 23);
            this.m_CopyrightStripStatusLabel.Text = "Copyright (C) 2021 Secure AI Labs, Inc.";
            // 
            // m_LoadFromConfigurationButton
            // 
            this.m_LoadFromConfigurationButton.AutoSize = true;
            this.m_LoadFromConfigurationButton.Location = new System.Drawing.Point(33, 147);
            this.m_LoadFromConfigurationButton.Name = "m_LoadFromConfigurationButton";
            this.m_LoadFromConfigurationButton.Size = new System.Drawing.Size(294, 26);
            this.m_LoadFromConfigurationButton.TabIndex = 3;
            this.m_LoadFromConfigurationButton.Text = "Load settings from File";
            this.m_LoadFromConfigurationButton.UseVisualStyleBackColor = true;
            this.m_LoadFromConfigurationButton.Click += new System.EventHandler(this.m_LoadFromConfigurationButton_Click);
            // 
            // m_ManuallyConfigureButton
            // 
            this.m_ManuallyConfigureButton.AutoSize = true;
            this.m_ManuallyConfigureButton.Location = new System.Drawing.Point(33, 179);
            this.m_ManuallyConfigureButton.Name = "m_ManuallyConfigureButton";
            this.m_ManuallyConfigureButton.Size = new System.Drawing.Size(294, 26);
            this.m_ManuallyConfigureButton.TabIndex = 4;
            this.m_ManuallyConfigureButton.Text = "Manually Configure Settings";
            this.m_ManuallyConfigureButton.UseVisualStyleBackColor = true;
            this.m_ManuallyConfigureButton.Click += new System.EventHandler(this.m_ManuallyConfigureButton_Click);
            // 
            // m_CloseButton
            // 
            this.m_CloseButton.AutoSize = true;
            this.m_CloseButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_CloseButton.Location = new System.Drawing.Point(213, 437);
            this.m_CloseButton.Name = "m_CloseButton";
            this.m_CloseButton.Size = new System.Drawing.Size(294, 26);
            this.m_CloseButton.TabIndex = 5;
            this.m_CloseButton.Text = "Exit";
            this.m_CloseButton.UseVisualStyleBackColor = true;
            this.m_CloseButton.Click += new System.EventHandler(this.m_CloseButton_Click);
            // 
            // m_InstructionsLabel
            // 
            this.m_InstructionsLabel.AutoSize = true;
            this.m_InstructionsLabel.Font = new System.Drawing.Font("Segoe UI Semibold", 9F, System.Drawing.FontStyle.Bold);
            this.m_InstructionsLabel.Location = new System.Drawing.Point(11, 22);
            this.m_InstructionsLabel.Name = "m_InstructionsLabel";
            this.m_InstructionsLabel.Size = new System.Drawing.Size(338, 90);
            this.m_InstructionsLabel.TabIndex = 6;
            this.m_InstructionsLabel.Text = resources.GetString("m_InstructionsLabel.Text");
            this.m_InstructionsLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.m_InstructionsLabel.Click += new System.EventHandler(this.m_InstructionsLabel_Click);
            // 
            // m_OpenSettingsFileDialog
            // 
            this.m_OpenSettingsFileDialog.DefaultExt = "cfg";
            this.m_OpenSettingsFileDialog.Filter = "Ignition Settings File|*.cfg";
            // 
            // m_SecureVirtualMachineGroupBox
            // 
            this.m_SecureVirtualMachineGroupBox.Controls.Add(this.m_ConfidentialVirtualMachine);
            this.m_SecureVirtualMachineGroupBox.Controls.Add(this.m_InstructionsLabel);
            this.m_SecureVirtualMachineGroupBox.Controls.Add(this.m_LoadFromConfigurationButton);
            this.m_SecureVirtualMachineGroupBox.Controls.Add(this.m_ManuallyConfigureButton);
            this.m_SecureVirtualMachineGroupBox.Location = new System.Drawing.Point(179, 1);
            this.m_SecureVirtualMachineGroupBox.Name = "m_SecureVirtualMachineGroupBox";
            this.m_SecureVirtualMachineGroupBox.Size = new System.Drawing.Size(363, 214);
            this.m_SecureVirtualMachineGroupBox.TabIndex = 7;
            this.m_SecureVirtualMachineGroupBox.TabStop = false;
            this.m_SecureVirtualMachineGroupBox.Text = "Secure Computational Virtual Machine Configuration";
            // 
            // m_WebPortalVirtualMachineConfigurationGroupBox
            // 
            this.m_WebPortalVirtualMachineConfigurationGroupBox.Controls.Add(this.m_WebPortalGroupBox);
            this.m_WebPortalVirtualMachineConfigurationGroupBox.Controls.Add(this.m_StartWebPortalVirtualMachineButton);
            this.m_WebPortalVirtualMachineConfigurationGroupBox.Location = new System.Drawing.Point(180, 221);
            this.m_WebPortalVirtualMachineConfigurationGroupBox.Name = "m_WebPortalVirtualMachineConfigurationGroupBox";
            this.m_WebPortalVirtualMachineConfigurationGroupBox.Size = new System.Drawing.Size(363, 101);
            this.m_WebPortalVirtualMachineConfigurationGroupBox.TabIndex = 8;
            this.m_WebPortalVirtualMachineConfigurationGroupBox.TabStop = false;
            this.m_WebPortalVirtualMachineConfigurationGroupBox.Text = "Web Portal Virtual Machine Instantiation";
            // 
            // m_WebPortalGroupBox
            // 
            this.m_WebPortalGroupBox.AutoSize = true;
            this.m_WebPortalGroupBox.Font = new System.Drawing.Font("Segoe UI Semibold", 9F, System.Drawing.FontStyle.Bold);
            this.m_WebPortalGroupBox.Location = new System.Drawing.Point(16, 20);
            this.m_WebPortalGroupBox.Name = "m_WebPortalGroupBox";
            this.m_WebPortalGroupBox.Size = new System.Drawing.Size(329, 30);
            this.m_WebPortalGroupBox.TabIndex = 8;
            this.m_WebPortalGroupBox.Text = "Web portal instances do not need to be configured with any\r\nspecific settings. Al" +
    "l you need to do is instantiate an instance.\r\n";
            this.m_WebPortalGroupBox.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.m_WebPortalGroupBox.Click += new System.EventHandler(this.m_WebPortalGroupBox_Click);
            // 
            // m_StartWebPortalVirtualMachineButton
            // 
            this.m_StartWebPortalVirtualMachineButton.AutoSize = true;
            this.m_StartWebPortalVirtualMachineButton.Location = new System.Drawing.Point(33, 60);
            this.m_StartWebPortalVirtualMachineButton.Name = "m_StartWebPortalVirtualMachineButton";
            this.m_StartWebPortalVirtualMachineButton.Size = new System.Drawing.Size(294, 26);
            this.m_StartWebPortalVirtualMachineButton.TabIndex = 7;
            this.m_StartWebPortalVirtualMachineButton.Text = "Instantiate";
            this.m_StartWebPortalVirtualMachineButton.UseVisualStyleBackColor = true;
            this.m_StartWebPortalVirtualMachineButton.Click += new System.EventHandler(this.m_StartWebPortalVirtualMachineButton_Click);
            // 
            // m_InitializerOnlyGroupBox
            // 
            this.m_InitializerOnlyGroupBox.Controls.Add(this.m_InitializerButton);
            this.m_InitializerOnlyGroupBox.Controls.Add(this.m_InitializerOnlyLabel);
            this.m_InitializerOnlyGroupBox.Location = new System.Drawing.Point(180, 328);
            this.m_InitializerOnlyGroupBox.Name = "m_InitializerOnlyGroupBox";
            this.m_InitializerOnlyGroupBox.Size = new System.Drawing.Size(363, 98);
            this.m_InitializerOnlyGroupBox.TabIndex = 9;
            this.m_InitializerOnlyGroupBox.TabStop = false;
            this.m_InitializerOnlyGroupBox.Text = "Initialize Existing Virtual Machine";
            // 
            // m_InitializerButton
            // 
            this.m_InitializerButton.Location = new System.Drawing.Point(33, 60);
            this.m_InitializerButton.Name = "m_InitializerButton";
            this.m_InitializerButton.Size = new System.Drawing.Size(294, 26);
            this.m_InitializerButton.TabIndex = 1;
            this.m_InitializerButton.Text = "Initialize!";
            this.m_InitializerButton.UseVisualStyleBackColor = true;
            this.m_InitializerButton.Click += new System.EventHandler(this.m_InitializerButton_Click);
            // 
            // m_InitializerOnlyLabel
            // 
            this.m_InitializerOnlyLabel.AutoSize = true;
            this.m_InitializerOnlyLabel.Font = new System.Drawing.Font("Segoe UI Semibold", 9F, System.Drawing.FontStyle.Bold);
            this.m_InitializerOnlyLabel.Location = new System.Drawing.Point(30, 22);
            this.m_InitializerOnlyLabel.Name = "m_InitializerOnlyLabel";
            this.m_InitializerOnlyLabel.Size = new System.Drawing.Size(303, 30);
            this.m_InitializerOnlyLabel.TabIndex = 0;
            this.m_InitializerOnlyLabel.Text = "Initialize an existing virtual machine already running the\r\nRootOfTrust process. " +
    "This is doe debug purposes.";
            this.m_InitializerOnlyLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.m_InitializerOnlyLabel.Click += new System.EventHandler(this.m_InitializerOnlyLabel_Click);
            // 
            // m_ConfidentialVirtualMachine
            // 
            this.m_ConfidentialVirtualMachine.AutoSize = true;
            this.m_ConfidentialVirtualMachine.Location = new System.Drawing.Point(96, 122);
            this.m_ConfidentialVirtualMachine.Name = "m_ConfidentialVirtualMachine";
            this.m_ConfidentialVirtualMachine.Size = new System.Drawing.Size(177, 19);
            this.m_ConfidentialVirtualMachine.TabIndex = 7;
            this.m_ConfidentialVirtualMachine.Text = "Confidential Virtual Machine";
            this.m_ConfidentialVirtualMachine.UseVisualStyleBackColor = true;
            // 
            // StartupDialog
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.CancelButton = this.m_CloseButton;
            this.ClientSize = new System.Drawing.Size(555, 502);
            this.ControlBox = false;
            this.Controls.Add(this.m_InitializerOnlyGroupBox);
            this.Controls.Add(this.m_WebPortalVirtualMachineConfigurationGroupBox);
            this.Controls.Add(this.m_SecureVirtualMachineGroupBox);
            this.Controls.Add(this.m_StatusStrip);
            this.Controls.Add(this.m_CloseButton);
            this.Controls.Add(this.m_ProductLabel);
            this.Controls.Add(this.pictureBox1);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "StartupDialog";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Ignite! A Microsoft Azure Initializer.";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.m_StatusStrip.ResumeLayout(false);
            this.m_StatusStrip.PerformLayout();
            this.m_SecureVirtualMachineGroupBox.ResumeLayout(false);
            this.m_SecureVirtualMachineGroupBox.PerformLayout();
            this.m_WebPortalVirtualMachineConfigurationGroupBox.ResumeLayout(false);
            this.m_WebPortalVirtualMachineConfigurationGroupBox.PerformLayout();
            this.m_InitializerOnlyGroupBox.ResumeLayout(false);
            this.m_InitializerOnlyGroupBox.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label m_ProductLabel;
        private System.Windows.Forms.StatusStrip m_StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel m_CopyrightStripStatusLabel;
        private System.Windows.Forms.Button m_LoadFromConfigurationButton;
        private System.Windows.Forms.Button m_ManuallyConfigureButton;
        private System.Windows.Forms.Button m_CloseButton;
        private System.Windows.Forms.Label m_InstructionsLabel;
        private System.Windows.Forms.OpenFileDialog m_OpenSettingsFileDialog;
        private System.Windows.Forms.GroupBox m_SecureVirtualMachineGroupBox;
        private System.Windows.Forms.GroupBox m_WebPortalVirtualMachineConfigurationGroupBox;
        private System.Windows.Forms.Label m_WebPortalGroupBox;
        private System.Windows.Forms.Button m_StartWebPortalVirtualMachineButton;
        private System.Windows.Forms.GroupBox m_InitializerOnlyGroupBox;
        private System.Windows.Forms.Button m_InitializerButton;
        private System.Windows.Forms.Label m_InitializerOnlyLabel;
        private System.Windows.Forms.CheckBox m_ConfidentialVirtualMachine;
    }
}