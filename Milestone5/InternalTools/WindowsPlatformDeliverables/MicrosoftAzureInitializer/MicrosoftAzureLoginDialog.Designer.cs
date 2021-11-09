
namespace MicrosoftAzureVirtualMachineInitializer
{
    partial class MicrosoftAzureLoginDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MicrosoftAzureLoginDialog));
            this.m_SecretLabel = new System.Windows.Forms.Label();
            this.m_TenantIdentifierLabel = new System.Windows.Forms.Label();
            this.m_SecretEditBox = new System.Windows.Forms.TextBox();
            this.m_TenantIdentifierEditBox = new System.Windows.Forms.TextBox();
            this.m_InstructionsLabel = new System.Windows.Forms.Label();
            this.m_LoginButton = new System.Windows.Forms.Button();
            this.m_CancelButton = new System.Windows.Forms.Button();
            this.m_ApplicationIdentifierTextBox = new System.Windows.Forms.TextBox();
            this.m_IpAddressLabel = new System.Windows.Forms.Label();
            this.m_MicrosoftLogoPictureBox = new System.Windows.Forms.PictureBox();
            this.m_StatusStrip = new System.Windows.Forms.StatusStrip();
            this.m_CopyrightStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_ExceptionStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_RefreshTimer = new System.Windows.Forms.Timer(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.m_MicrosoftLogoPictureBox)).BeginInit();
            this.m_StatusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_SecretLabel
            // 
            this.m_SecretLabel.AutoSize = true;
            this.m_SecretLabel.Location = new System.Drawing.Point(224, 94);
            this.m_SecretLabel.Name = "m_SecretLabel";
            this.m_SecretLabel.Size = new System.Drawing.Size(39, 15);
            this.m_SecretLabel.TabIndex = 0;
            this.m_SecretLabel.Text = "Secret";
            this.m_SecretLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // m_TenantIdentifierLabel
            // 
            this.m_TenantIdentifierLabel.AutoSize = true;
            this.m_TenantIdentifierLabel.Location = new System.Drawing.Point(171, 64);
            this.m_TenantIdentifierLabel.Name = "m_TenantIdentifierLabel";
            this.m_TenantIdentifierLabel.Size = new System.Drawing.Size(92, 15);
            this.m_TenantIdentifierLabel.TabIndex = 1;
            this.m_TenantIdentifierLabel.Text = "Tenant Identifier";
            // 
            // m_SecretEditBox
            // 
            this.m_SecretEditBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_SecretEditBox.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_SecretEditBox.Location = new System.Drawing.Point(269, 92);
            this.m_SecretEditBox.Name = "m_SecretEditBox";
            this.m_SecretEditBox.PasswordChar = '*';
            this.m_SecretEditBox.Size = new System.Drawing.Size(396, 22);
            this.m_SecretEditBox.TabIndex = 3;
            // 
            // m_TenantIdentifierEditBox
            // 
            this.m_TenantIdentifierEditBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_TenantIdentifierEditBox.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.m_TenantIdentifierEditBox.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_TenantIdentifierEditBox.Location = new System.Drawing.Point(269, 62);
            this.m_TenantIdentifierEditBox.Name = "m_TenantIdentifierEditBox";
            this.m_TenantIdentifierEditBox.Size = new System.Drawing.Size(396, 22);
            this.m_TenantIdentifierEditBox.TabIndex = 2;
            // 
            // m_InstructionsLabel
            // 
            this.m_InstructionsLabel.AutoSize = true;
            this.m_InstructionsLabel.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold);
            this.m_InstructionsLabel.Location = new System.Drawing.Point(266, 7);
            this.m_InstructionsLabel.Name = "m_InstructionsLabel";
            this.m_InstructionsLabel.Size = new System.Drawing.Size(245, 15);
            this.m_InstructionsLabel.TabIndex = 0;
            this.m_InstructionsLabel.Text = "Please enter your Microsoft Azure centials.";
            // 
            // m_LoginButton
            // 
            this.m_LoginButton.Location = new System.Drawing.Point(269, 124);
            this.m_LoginButton.Name = "m_LoginButton";
            this.m_LoginButton.Size = new System.Drawing.Size(128, 27);
            this.m_LoginButton.TabIndex = 4;
            this.m_LoginButton.Text = "&Login";
            this.m_LoginButton.UseVisualStyleBackColor = true;
            this.m_LoginButton.Click += new System.EventHandler(this.m_LoginButton_Click);
            // 
            // m_CancelButton
            // 
            this.m_CancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_CancelButton.Location = new System.Drawing.Point(403, 124);
            this.m_CancelButton.Name = "m_CancelButton";
            this.m_CancelButton.Size = new System.Drawing.Size(128, 27);
            this.m_CancelButton.TabIndex = 5;
            this.m_CancelButton.Text = "&Cancel";
            this.m_CancelButton.UseVisualStyleBackColor = true;
            this.m_CancelButton.Click += new System.EventHandler(this.m_CancelButton_Click);
            // 
            // m_ApplicationIdentifierTextBox
            // 
            this.m_ApplicationIdentifierTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_ApplicationIdentifierTextBox.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.m_ApplicationIdentifierTextBox.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_ApplicationIdentifierTextBox.Location = new System.Drawing.Point(269, 32);
            this.m_ApplicationIdentifierTextBox.Name = "m_ApplicationIdentifierTextBox";
            this.m_ApplicationIdentifierTextBox.Size = new System.Drawing.Size(396, 22);
            this.m_ApplicationIdentifierTextBox.TabIndex = 1;
            // 
            // m_IpAddressLabel
            // 
            this.m_IpAddressLabel.AutoSize = true;
            this.m_IpAddressLabel.Location = new System.Drawing.Point(145, 34);
            this.m_IpAddressLabel.Name = "m_IpAddressLabel";
            this.m_IpAddressLabel.Size = new System.Drawing.Size(118, 15);
            this.m_IpAddressLabel.TabIndex = 8;
            this.m_IpAddressLabel.Text = "Application Identifier";
            this.m_IpAddressLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // m_MicrosoftLogoPictureBox
            // 
            this.m_MicrosoftLogoPictureBox.Image = ((System.Drawing.Image)(resources.GetObject("m_MicrosoftLogoPictureBox.Image")));
            this.m_MicrosoftLogoPictureBox.Location = new System.Drawing.Point(12, 22);
            this.m_MicrosoftLogoPictureBox.Name = "m_MicrosoftLogoPictureBox";
            this.m_MicrosoftLogoPictureBox.Size = new System.Drawing.Size(120, 120);
            this.m_MicrosoftLogoPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.m_MicrosoftLogoPictureBox.TabIndex = 9;
            this.m_MicrosoftLogoPictureBox.TabStop = false;
            // 
            // m_StatusStrip
            // 
            this.m_StatusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_CopyrightStripStatusLabel,
            this.m_ExceptionStripStatusLabel});
            this.m_StatusStrip.LayoutStyle = System.Windows.Forms.ToolStripLayoutStyle.HorizontalStackWithOverflow;
            this.m_StatusStrip.Location = new System.Drawing.Point(0, 159);
            this.m_StatusStrip.Name = "m_StatusStrip";
            this.m_StatusStrip.Padding = new System.Windows.Forms.Padding(2, 0, 14, 0);
            this.m_StatusStrip.Size = new System.Drawing.Size(681, 27);
            this.m_StatusStrip.TabIndex = 10;
            this.m_StatusStrip.Text = "statusStrip1";
            // 
            // m_CopyrightStripStatusLabel
            // 
            this.m_CopyrightStripStatusLabel.AutoSize = false;
            this.m_CopyrightStripStatusLabel.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_CopyrightStripStatusLabel.Name = "m_CopyrightStripStatusLabel";
            this.m_CopyrightStripStatusLabel.Size = new System.Drawing.Size(350, 22);
            this.m_CopyrightStripStatusLabel.Text = "Copyright (C) 2021 Secure AI Labs, Inc.";
            this.m_CopyrightStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_ExceptionStripStatusLabel
            // 
            this.m_ExceptionStripStatusLabel.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.m_ExceptionStripStatusLabel.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_ExceptionStripStatusLabel.ForeColor = System.Drawing.Color.Red;
            this.m_ExceptionStripStatusLabel.IsLink = true;
            this.m_ExceptionStripStatusLabel.LinkColor = System.Drawing.Color.Red;
            this.m_ExceptionStripStatusLabel.Name = "m_ExceptionStripStatusLabel";
            this.m_ExceptionStripStatusLabel.Size = new System.Drawing.Size(210, 22);
            this.m_ExceptionStripStatusLabel.Text = "Click here to view exceptions";
            this.m_ExceptionStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.m_ExceptionStripStatusLabel.Click += new System.EventHandler(this.m_ExceptionStripStatusLabel_Click);
            // 
            // m_RefreshTimer
            // 
            this.m_RefreshTimer.Enabled = true;
            this.m_RefreshTimer.Tick += new System.EventHandler(this.m_RefreshTimer_Tick);
            // 
            // MicrosoftAzureLoginDialog
            // 
            this.AcceptButton = this.m_LoginButton;
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.CancelButton = this.m_CancelButton;
            this.ClientSize = new System.Drawing.Size(681, 186);
            this.ControlBox = false;
            this.Controls.Add(this.m_StatusStrip);
            this.Controls.Add(this.m_MicrosoftLogoPictureBox);
            this.Controls.Add(this.m_IpAddressLabel);
            this.Controls.Add(this.m_ApplicationIdentifierTextBox);
            this.Controls.Add(this.m_CancelButton);
            this.Controls.Add(this.m_LoginButton);
            this.Controls.Add(this.m_InstructionsLabel);
            this.Controls.Add(this.m_TenantIdentifierEditBox);
            this.Controls.Add(this.m_SecretEditBox);
            this.Controls.Add(this.m_TenantIdentifierLabel);
            this.Controls.Add(this.m_SecretLabel);
            this.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.Name = "MicrosoftAzureLoginDialog";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Ignite! A Microsoft Azure Initializer.";
            ((System.ComponentModel.ISupportInitialize)(this.m_MicrosoftLogoPictureBox)).EndInit();
            this.m_StatusStrip.ResumeLayout(false);
            this.m_StatusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label m_SecretLabel;
        private System.Windows.Forms.Label m_TenantIdentifierLabel;
        private System.Windows.Forms.TextBox m_SecretEditBox;
        private System.Windows.Forms.TextBox m_TenantIdentifierEditBox;
        private System.Windows.Forms.Label m_InstructionsLabel;
        private System.Windows.Forms.Button m_LoginButton;
        private System.Windows.Forms.Button m_CancelButton;
        private System.Windows.Forms.TextBox m_ApplicationIdentifierTextBox;
        private System.Windows.Forms.Label m_IpAddressLabel;
        private System.Windows.Forms.PictureBox m_MicrosoftLogoPictureBox;
        private System.Windows.Forms.StatusStrip m_StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel m_CopyrightStripStatusLabel;
        private System.Windows.Forms.ToolStripStatusLabel m_ExceptionStripStatusLabel;
        private System.Windows.Forms.Timer m_RefreshTimer;
    }
}

