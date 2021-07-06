
namespace StringTo64BitHashGenerator
{
    partial class MainDialog
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainDialog));
            this.m_TextToConvertTextBox = new System.Windows.Forms.TextBox();
            this.m_64BitHashTextBox = new System.Windows.Forms.TextBox();
            this.m_TextToCovertLabel = new System.Windows.Forms.Label();
            this.m_64BitHashLabel = new System.Windows.Forms.Label();
            this.m_Copy64BitHashLinkLabel = new System.Windows.Forms.LinkLabel();
            this.m_LogoPictureBox = new System.Windows.Forms.PictureBox();
            this.m_StatusStrip = new System.Windows.Forms.StatusStrip();
            this.m_CopyrightToolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_TimeStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_TimeRefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.m_CaseSensitiveCheckBox = new System.Windows.Forms.CheckBox();
            this.m_TitleLabel = new System.Windows.Forms.Label();
            this.m_PastLinkLabel = new System.Windows.Forms.LinkLabel();
            ((System.ComponentModel.ISupportInitialize)(this.m_LogoPictureBox)).BeginInit();
            this.m_StatusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_TextToConvertTextBox
            // 
            this.m_TextToConvertTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_TextToConvertTextBox.Location = new System.Drawing.Point(171, 28);
            this.m_TextToConvertTextBox.Margin = new System.Windows.Forms.Padding(4);
            this.m_TextToConvertTextBox.Multiline = true;
            this.m_TextToConvertTextBox.Name = "m_TextToConvertTextBox";
            this.m_TextToConvertTextBox.Size = new System.Drawing.Size(588, 194);
            this.m_TextToConvertTextBox.TabIndex = 0;
            this.m_TextToConvertTextBox.TextChanged += new System.EventHandler(this.m_TextToConvertTextBox_TextChanged);
            // 
            // m_64BitHashTextBox
            // 
            this.m_64BitHashTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_64BitHashTextBox.Location = new System.Drawing.Point(171, 272);
            this.m_64BitHashTextBox.Margin = new System.Windows.Forms.Padding(4);
            this.m_64BitHashTextBox.Name = "m_64BitHashTextBox";
            this.m_64BitHashTextBox.ReadOnly = true;
            this.m_64BitHashTextBox.Size = new System.Drawing.Size(588, 23);
            this.m_64BitHashTextBox.TabIndex = 1;
            // 
            // m_TextToCovertLabel
            // 
            this.m_TextToCovertLabel.AutoSize = true;
            this.m_TextToCovertLabel.Location = new System.Drawing.Point(168, 9);
            this.m_TextToCovertLabel.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.m_TextToCovertLabel.Name = "m_TextToCovertLabel";
            this.m_TextToCovertLabel.Size = new System.Drawing.Size(147, 15);
            this.m_TextToCovertLabel.TabIndex = 2;
            this.m_TextToCovertLabel.Text = "Input String Here...";
            // 
            // m_64BitHashLabel
            // 
            this.m_64BitHashLabel.AutoSize = true;
            this.m_64BitHashLabel.Location = new System.Drawing.Point(168, 253);
            this.m_64BitHashLabel.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.m_64BitHashLabel.Name = "m_64BitHashLabel";
            this.m_64BitHashLabel.Size = new System.Drawing.Size(175, 15);
            this.m_64BitHashLabel.TabIndex = 3;
            this.m_64BitHashLabel.Text = "64 Bit String Hash (HEX)";
            // 
            // m_Copy64BitHashLinkLabel
            // 
            this.m_Copy64BitHashLinkLabel.AutoSize = true;
            this.m_Copy64BitHashLinkLabel.Location = new System.Drawing.Point(767, 275);
            this.m_Copy64BitHashLinkLabel.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.m_Copy64BitHashLinkLabel.Name = "m_Copy64BitHashLinkLabel";
            this.m_Copy64BitHashLinkLabel.Size = new System.Drawing.Size(35, 15);
            this.m_Copy64BitHashLinkLabel.TabIndex = 4;
            this.m_Copy64BitHashLinkLabel.TabStop = true;
            this.m_Copy64BitHashLinkLabel.Text = "Copy";
            this.m_Copy64BitHashLinkLabel.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.m_Copy64BitHashLinkLabel_LinkClicked);
            // 
            // m_LogoPictureBox
            // 
            this.m_LogoPictureBox.Image = global::StringHashGenerator.Properties.Resources.SAIL_Icon1;
            this.m_LogoPictureBox.Location = new System.Drawing.Point(18, 32);
            this.m_LogoPictureBox.Margin = new System.Windows.Forms.Padding(4);
            this.m_LogoPictureBox.Name = "m_LogoPictureBox";
            this.m_LogoPictureBox.Size = new System.Drawing.Size(140, 140);
            this.m_LogoPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.m_LogoPictureBox.TabIndex = 5;
            this.m_LogoPictureBox.TabStop = false;
            // 
            // m_StatusStrip
            // 
            this.m_StatusStrip.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_StatusStrip.GripMargin = new System.Windows.Forms.Padding(0);
            this.m_StatusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_CopyrightToolStripStatusLabel,
            this.m_TimeStripStatusLabel});
            this.m_StatusStrip.Location = new System.Drawing.Point(0, 311);
            this.m_StatusStrip.Name = "m_StatusStrip";
            this.m_StatusStrip.Padding = new System.Windows.Forms.Padding(2, 0, 21, 0);
            this.m_StatusStrip.Size = new System.Drawing.Size(816, 29);
            this.m_StatusStrip.TabIndex = 6;
            this.m_StatusStrip.Text = "statusStrip1";
            // 
            // m_CopyrightToolStripStatusLabel
            // 
            this.m_CopyrightToolStripStatusLabel.AutoSize = false;
            this.m_CopyrightToolStripStatusLabel.Name = "m_CopyrightToolStripStatusLabel";
            this.m_CopyrightToolStripStatusLabel.Size = new System.Drawing.Size(650, 24);
            this.m_CopyrightToolStripStatusLabel.Text = "Copyright (C) 2021 Secure AI Labs, Inc.";
            this.m_CopyrightToolStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_TimeStripStatusLabel
            // 
            this.m_TimeStripStatusLabel.ForeColor = System.Drawing.Color.Green;
            this.m_TimeStripStatusLabel.Name = "m_TimeStripStatusLabel";
            this.m_TimeStripStatusLabel.Size = new System.Drawing.Size(224, 24);
            this.m_TimeStripStatusLabel.Text = "September 31st, 2021 @ 22:22:22";
            this.m_TimeStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_TimeRefreshTimer
            // 
            this.m_TimeRefreshTimer.Enabled = true;
            this.m_TimeRefreshTimer.Tick += new System.EventHandler(this.m_TimeRefreshTimer_Tick);
            // 
            // m_CaseSensitiveCheckBox
            // 
            this.m_CaseSensitiveCheckBox.AutoSize = true;
            this.m_CaseSensitiveCheckBox.Location = new System.Drawing.Point(171, 230);
            this.m_CaseSensitiveCheckBox.Margin = new System.Windows.Forms.Padding(4);
            this.m_CaseSensitiveCheckBox.Name = "m_CaseSensitiveCheckBox";
            this.m_CaseSensitiveCheckBox.Size = new System.Drawing.Size(320, 19);
            this.m_CaseSensitiveCheckBox.TabIndex = 7;
            this.m_CaseSensitiveCheckBox.Text = "Generate Case Sensitive 64 Bit String Hash";
            this.m_CaseSensitiveCheckBox.UseVisualStyleBackColor = true;
            this.m_CaseSensitiveCheckBox.CheckedChanged += new System.EventHandler(this.m_CaseSensitiveCheckBox_CheckedChanged);
            // 
            // m_TitleLabel
            // 
            this.m_TitleLabel.AutoSize = true;
            this.m_TitleLabel.Font = new System.Drawing.Font("Segoe UI", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_TitleLabel.Location = new System.Drawing.Point(13, 176);
            this.m_TitleLabel.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.m_TitleLabel.Name = "m_TitleLabel";
            this.m_TitleLabel.Size = new System.Drawing.Size(153, 39);
            this.m_TitleLabel.TabIndex = 8;
            this.m_TitleLabel.Text = "Gimlet\r\n64 Bit String Hash Generator\r\nVersion 1.0.0\r\n";
            this.m_TitleLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // m_PastLinkLabel
            // 
            this.m_PastLinkLabel.AutoSize = true;
            this.m_PastLinkLabel.Location = new System.Drawing.Point(766, 118);
            this.m_PastLinkLabel.Name = "m_PastLinkLabel";
            this.m_PastLinkLabel.Size = new System.Drawing.Size(42, 15);
            this.m_PastLinkLabel.TabIndex = 9;
            this.m_PastLinkLabel.TabStop = true;
            this.m_PastLinkLabel.Text = "Paste";
            this.m_PastLinkLabel.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.m_PastLinkLabel_LinkClicked);
            // 
            // MainDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(816, 340);
            this.Controls.Add(this.m_PastLinkLabel);
            this.Controls.Add(this.m_TitleLabel);
            this.Controls.Add(this.m_CaseSensitiveCheckBox);
            this.Controls.Add(this.m_StatusStrip);
            this.Controls.Add(this.m_LogoPictureBox);
            this.Controls.Add(this.m_Copy64BitHashLinkLabel);
            this.Controls.Add(this.m_64BitHashLabel);
            this.Controls.Add(this.m_TextToCovertLabel);
            this.Controls.Add(this.m_64BitHashTextBox);
            this.Controls.Add(this.m_TextToConvertTextBox);
            this.Font = new System.Drawing.Font("Consolas", 9.75F);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "MainDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Gimlet!!! A 64 Bit String Hash Generator";
            ((System.ComponentModel.ISupportInitialize)(this.m_LogoPictureBox)).EndInit();
            this.m_StatusStrip.ResumeLayout(false);
            this.m_StatusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox m_TextToConvertTextBox;
        private System.Windows.Forms.TextBox m_64BitHashTextBox;
        private System.Windows.Forms.Label m_TextToCovertLabel;
        private System.Windows.Forms.Label m_64BitHashLabel;
        private System.Windows.Forms.LinkLabel m_Copy64BitHashLinkLabel;
        private System.Windows.Forms.PictureBox m_LogoPictureBox;
        private System.Windows.Forms.StatusStrip m_StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel m_CopyrightToolStripStatusLabel;
        private System.Windows.Forms.ToolStripStatusLabel m_TimeStripStatusLabel;
        private System.Windows.Forms.Timer m_TimeRefreshTimer;
        private System.Windows.Forms.CheckBox m_CaseSensitiveCheckBox;
        private System.Windows.Forms.Label m_TitleLabel;
        private System.Windows.Forms.LinkLabel m_PastLinkLabel;
    }
}

