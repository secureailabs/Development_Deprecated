
namespace WindowsRemoteDataConnector
{
    partial class SailWebApiPortalLoginDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SailWebApiPortalLoginDialog));
            this.m_UsernameLabel = new System.Windows.Forms.Label();
            this.m_PasswordLabel = new System.Windows.Forms.Label();
            this.m_UsernameEditBox = new System.Windows.Forms.TextBox();
            this.m_PasswordEditBox = new System.Windows.Forms.TextBox();
            this.m_InstructionsLabel = new System.Windows.Forms.Label();
            this.m_LoginButton = new System.Windows.Forms.Button();
            this.m_CancelButton = new System.Windows.Forms.Button();
            this.m_IpAddressTextBox = new System.Windows.Forms.TextBox();
            this.m_IpAddressLabel = new System.Windows.Forms.Label();
            this.m_SailLogoPictureBox = new System.Windows.Forms.PictureBox();
            this.m_StatusStrip = new System.Windows.Forms.StatusStrip();
            this.m_CopyrightStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_ExceptionsStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_RefreshTimer = new System.Windows.Forms.Timer(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.m_SailLogoPictureBox)).BeginInit();
            this.m_StatusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_UsernameLabel
            // 
            this.m_UsernameLabel.AutoSize = true;
            this.m_UsernameLabel.Location = new System.Drawing.Point(151, 59);
            this.m_UsernameLabel.Margin = new System.Windows.Forms.Padding(1, 0, 1, 0);
            this.m_UsernameLabel.Name = "m_UsernameLabel";
            this.m_UsernameLabel.Size = new System.Drawing.Size(60, 15);
            this.m_UsernameLabel.TabIndex = 0;
            this.m_UsernameLabel.Text = "Username";
            // 
            // m_PasswordLabel
            // 
            this.m_PasswordLabel.AutoSize = true;
            this.m_PasswordLabel.Location = new System.Drawing.Point(154, 87);
            this.m_PasswordLabel.Margin = new System.Windows.Forms.Padding(1, 0, 1, 0);
            this.m_PasswordLabel.Name = "m_PasswordLabel";
            this.m_PasswordLabel.Size = new System.Drawing.Size(57, 15);
            this.m_PasswordLabel.TabIndex = 1;
            this.m_PasswordLabel.Text = "Password";
            // 
            // m_UsernameEditBox
            // 
            this.m_UsernameEditBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_UsernameEditBox.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_UsernameEditBox.Location = new System.Drawing.Point(213, 57);
            this.m_UsernameEditBox.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.m_UsernameEditBox.Name = "m_UsernameEditBox";
            this.m_UsernameEditBox.Size = new System.Drawing.Size(375, 22);
            this.m_UsernameEditBox.TabIndex = 2;
            // 
            // m_PasswordEditBox
            // 
            this.m_PasswordEditBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_PasswordEditBox.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_PasswordEditBox.Location = new System.Drawing.Point(213, 85);
            this.m_PasswordEditBox.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.m_PasswordEditBox.Name = "m_PasswordEditBox";
            this.m_PasswordEditBox.PasswordChar = '*';
            this.m_PasswordEditBox.Size = new System.Drawing.Size(375, 22);
            this.m_PasswordEditBox.TabIndex = 3;
            // 
            // m_InstructionsLabel
            // 
            this.m_InstructionsLabel.AutoSize = true;
            this.m_InstructionsLabel.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold);
            this.m_InstructionsLabel.Location = new System.Drawing.Point(209, 7);
            this.m_InstructionsLabel.Margin = new System.Windows.Forms.Padding(1, 0, 1, 0);
            this.m_InstructionsLabel.Name = "m_InstructionsLabel";
            this.m_InstructionsLabel.Size = new System.Drawing.Size(250, 15);
            this.m_InstructionsLabel.TabIndex = 0;
            this.m_InstructionsLabel.Text = "Please enter your SAIL Web Api Credentials.";
            // 
            // m_LoginButton
            // 
            this.m_LoginButton.Location = new System.Drawing.Point(213, 117);
            this.m_LoginButton.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.m_LoginButton.Name = "m_LoginButton";
            this.m_LoginButton.Size = new System.Drawing.Size(99, 27);
            this.m_LoginButton.TabIndex = 4;
            this.m_LoginButton.Text = "&Login";
            this.m_LoginButton.UseVisualStyleBackColor = true;
            this.m_LoginButton.Click += new System.EventHandler(this.m_LoginButton_Click);
            // 
            // m_CancelButton
            // 
            this.m_CancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_CancelButton.Location = new System.Drawing.Point(314, 117);
            this.m_CancelButton.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.m_CancelButton.Name = "m_CancelButton";
            this.m_CancelButton.Size = new System.Drawing.Size(99, 27);
            this.m_CancelButton.TabIndex = 5;
            this.m_CancelButton.Text = "&Cancel";
            this.m_CancelButton.UseVisualStyleBackColor = true;
            this.m_CancelButton.Click += new System.EventHandler(this.m_CancelButton_Click);
            // 
            // m_IpAddressTextBox
            // 
            this.m_IpAddressTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_IpAddressTextBox.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_IpAddressTextBox.Location = new System.Drawing.Point(213, 29);
            this.m_IpAddressTextBox.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.m_IpAddressTextBox.Name = "m_IpAddressTextBox";
            this.m_IpAddressTextBox.Size = new System.Drawing.Size(375, 22);
            this.m_IpAddressTextBox.TabIndex = 1;
            // 
            // m_IpAddressLabel
            // 
            this.m_IpAddressLabel.AutoSize = true;
            this.m_IpAddressLabel.Location = new System.Drawing.Point(149, 31);
            this.m_IpAddressLabel.Margin = new System.Windows.Forms.Padding(1, 0, 1, 0);
            this.m_IpAddressLabel.Name = "m_IpAddressLabel";
            this.m_IpAddressLabel.Size = new System.Drawing.Size(62, 15);
            this.m_IpAddressLabel.TabIndex = 8;
            this.m_IpAddressLabel.Text = "IP Address";
            // 
            // m_SailLogoPictureBox
            // 
            this.m_SailLogoPictureBox.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.m_SailLogoPictureBox.Image = ((System.Drawing.Image)(resources.GetObject("m_SailLogoPictureBox.Image")));
            this.m_SailLogoPictureBox.Location = new System.Drawing.Point(10, 18);
            this.m_SailLogoPictureBox.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.m_SailLogoPictureBox.Name = "m_SailLogoPictureBox";
            this.m_SailLogoPictureBox.Size = new System.Drawing.Size(120, 120);
            this.m_SailLogoPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.m_SailLogoPictureBox.TabIndex = 9;
            this.m_SailLogoPictureBox.TabStop = false;
            // 
            // m_StatusStrip
            // 
            this.m_StatusStrip.AutoSize = false;
            this.m_StatusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_CopyrightStripStatusLabel,
            this.m_ExceptionsStripStatusLabel});
            this.m_StatusStrip.LayoutStyle = System.Windows.Forms.ToolStripLayoutStyle.HorizontalStackWithOverflow;
            this.m_StatusStrip.Location = new System.Drawing.Point(0, 156);
            this.m_StatusStrip.Name = "m_StatusStrip";
            this.m_StatusStrip.Padding = new System.Windows.Forms.Padding(1, 0, 9, 0);
            this.m_StatusStrip.Size = new System.Drawing.Size(605, 28);
            this.m_StatusStrip.TabIndex = 10;
            this.m_StatusStrip.Text = "statusStrip1";
            // 
            // m_CopyrightStripStatusLabel
            // 
            this.m_CopyrightStripStatusLabel.AutoSize = false;
            this.m_CopyrightStripStatusLabel.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_CopyrightStripStatusLabel.Name = "m_CopyrightStripStatusLabel";
            this.m_CopyrightStripStatusLabel.Size = new System.Drawing.Size(330, 21);
            this.m_CopyrightStripStatusLabel.Text = "Copyright (C) 2021 Secure AI Labs, Inc.";
            this.m_CopyrightStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_ExceptionsStripStatusLabel
            // 
            this.m_ExceptionsStripStatusLabel.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.m_ExceptionsStripStatusLabel.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_ExceptionsStripStatusLabel.ForeColor = System.Drawing.Color.Red;
            this.m_ExceptionsStripStatusLabel.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.m_ExceptionsStripStatusLabel.IsLink = true;
            this.m_ExceptionsStripStatusLabel.LinkColor = System.Drawing.Color.Red;
            this.m_ExceptionsStripStatusLabel.Name = "m_ExceptionsStripStatusLabel";
            this.m_ExceptionsStripStatusLabel.Size = new System.Drawing.Size(0, 23);
            this.m_ExceptionsStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.m_ExceptionsStripStatusLabel.Click += new System.EventHandler(this.m_ExceptionsStripStatusLabel_Click);
            // 
            // m_RefreshTimer
            // 
            this.m_RefreshTimer.Enabled = true;
            this.m_RefreshTimer.Tick += new System.EventHandler(this.m_RefreshTimer_Tick);
            // 
            // SailWebApiPortalLoginDialog
            // 
            this.AcceptButton = this.m_LoginButton;
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.CancelButton = this.m_CancelButton;
            this.ClientSize = new System.Drawing.Size(605, 184);
            this.ControlBox = false;
            this.Controls.Add(this.m_StatusStrip);
            this.Controls.Add(this.m_SailLogoPictureBox);
            this.Controls.Add(this.m_IpAddressLabel);
            this.Controls.Add(this.m_IpAddressTextBox);
            this.Controls.Add(this.m_CancelButton);
            this.Controls.Add(this.m_LoginButton);
            this.Controls.Add(this.m_InstructionsLabel);
            this.Controls.Add(this.m_PasswordEditBox);
            this.Controls.Add(this.m_UsernameEditBox);
            this.Controls.Add(this.m_PasswordLabel);
            this.Controls.Add(this.m_UsernameLabel);
            this.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(1, 3, 1, 3);
            this.Name = "SailWebApiPortalLoginDialog";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "SAIL Remote Data Connector";
            this.Load += new System.EventHandler(this.SailWebApiPortalLoginDialog_Load);
            ((System.ComponentModel.ISupportInitialize)(this.m_SailLogoPictureBox)).EndInit();
            this.m_StatusStrip.ResumeLayout(false);
            this.m_StatusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label m_UsernameLabel;
        private System.Windows.Forms.Label m_PasswordLabel;
        private System.Windows.Forms.TextBox m_UsernameEditBox;
        private System.Windows.Forms.TextBox m_PasswordEditBox;
        private System.Windows.Forms.Label m_InstructionsLabel;
        private System.Windows.Forms.Button m_LoginButton;
        private System.Windows.Forms.Button m_CancelButton;
        private System.Windows.Forms.TextBox m_IpAddressTextBox;
        private System.Windows.Forms.Label m_IpAddressLabel;
        private System.Windows.Forms.PictureBox m_SailLogoPictureBox;
        private System.Windows.Forms.StatusStrip m_StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel m_CopyrightStripStatusLabel;
        private System.Windows.Forms.ToolStripStatusLabel m_ExceptionsStripStatusLabel;
        private System.Windows.Forms.Timer m_RefreshTimer;
    }
}

