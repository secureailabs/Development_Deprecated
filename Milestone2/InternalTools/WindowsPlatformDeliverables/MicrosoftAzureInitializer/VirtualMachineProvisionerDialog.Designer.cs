
namespace MicrosoftAzureVirtualMachineInitializer
{
    partial class VirtualMachineProvisionerDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(VirtualMachineProvisionerDialog));
            this.m_InstructionsLabel = new System.Windows.Forms.Label();
            this.m_GoButton = new System.Windows.Forms.Button();
            this.m_ExitButton = new System.Windows.Forms.Button();
            this.m_StatusStrip = new System.Windows.Forms.StatusStrip();
            this.m_CopyrightToolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_ExceptionsStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_RefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.m_ListBox = new System.Windows.Forms.ListBox();
            this.m_StatusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_InstructionsLabel
            // 
            this.m_InstructionsLabel.AutoSize = true;
            this.m_InstructionsLabel.Location = new System.Drawing.Point(9, 7);
            this.m_InstructionsLabel.Margin = new System.Windows.Forms.Padding(1, 0, 1, 0);
            this.m_InstructionsLabel.Name = "m_InstructionsLabel";
            this.m_InstructionsLabel.Size = new System.Drawing.Size(534, 13);
            this.m_InstructionsLabel.TabIndex = 1;
            this.m_InstructionsLabel.Text = "Press \"Go!\" in order to trigger the provisioning and initialization process of al" +
    "l of the virtual machines listed below.";
            // 
            // m_GoButton
            // 
            this.m_GoButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.m_GoButton.Location = new System.Drawing.Point(263, 273);
            this.m_GoButton.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.m_GoButton.Name = "m_GoButton";
            this.m_GoButton.Size = new System.Drawing.Size(101, 25);
            this.m_GoButton.TabIndex = 2;
            this.m_GoButton.Text = "Go!";
            this.m_GoButton.UseVisualStyleBackColor = true;
            this.m_GoButton.Click += new System.EventHandler(this.m_GoButton_Click);
            // 
            // m_ExitButton
            // 
            this.m_ExitButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.m_ExitButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_ExitButton.Location = new System.Drawing.Point(366, 273);
            this.m_ExitButton.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.m_ExitButton.Name = "m_ExitButton";
            this.m_ExitButton.Size = new System.Drawing.Size(101, 25);
            this.m_ExitButton.TabIndex = 4;
            this.m_ExitButton.Text = "Exit";
            this.m_ExitButton.UseVisualStyleBackColor = true;
            this.m_ExitButton.Click += new System.EventHandler(this.m_ExitButton_Click);
            // 
            // m_StatusStrip
            // 
            this.m_StatusStrip.AutoSize = false;
            this.m_StatusStrip.Font = new System.Drawing.Font("Consolas", 9F);
            this.m_StatusStrip.GripStyle = System.Windows.Forms.ToolStripGripStyle.Visible;
            this.m_StatusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_CopyrightToolStripStatusLabel,
            this.m_ExceptionsStripStatusLabel});
            this.m_StatusStrip.LayoutStyle = System.Windows.Forms.ToolStripLayoutStyle.HorizontalStackWithOverflow;
            this.m_StatusStrip.Location = new System.Drawing.Point(0, 306);
            this.m_StatusStrip.Name = "m_StatusStrip";
            this.m_StatusStrip.Padding = new System.Windows.Forms.Padding(1, 0, 9, 0);
            this.m_StatusStrip.Size = new System.Drawing.Size(730, 28);
            this.m_StatusStrip.TabIndex = 5;
            this.m_StatusStrip.Text = "Text";
            // 
            // m_CopyrightToolStripStatusLabel
            // 
            this.m_CopyrightToolStripStatusLabel.AutoSize = false;
            this.m_CopyrightToolStripStatusLabel.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.m_CopyrightToolStripStatusLabel.Name = "m_CopyrightToolStripStatusLabel";
            this.m_CopyrightToolStripStatusLabel.Size = new System.Drawing.Size(350, 28);
            this.m_CopyrightToolStripStatusLabel.Text = "Copyright (C) 2021 Secure AI Labs, Inc.";
            this.m_CopyrightToolStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_ExceptionsStripStatusLabel
            // 
            this.m_ExceptionsStripStatusLabel.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.m_ExceptionsStripStatusLabel.AutoSize = false;
            this.m_ExceptionsStripStatusLabel.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.m_ExceptionsStripStatusLabel.IsLink = true;
            this.m_ExceptionsStripStatusLabel.LinkColor = System.Drawing.Color.Red;
            this.m_ExceptionsStripStatusLabel.Name = "m_ExceptionsStripStatusLabel";
            this.m_ExceptionsStripStatusLabel.Size = new System.Drawing.Size(250, 28);
            this.m_ExceptionsStripStatusLabel.Spring = true;
            this.m_ExceptionsStripStatusLabel.Text = "Click here to view exceptions";
            this.m_ExceptionsStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.m_ExceptionsStripStatusLabel.Click += new System.EventHandler(this.m_ExceptionsStripStatusLabel_Click);
            // 
            // m_RefreshTimer
            // 
            this.m_RefreshTimer.Enabled = true;
            this.m_RefreshTimer.Interval = 250;
            this.m_RefreshTimer.Tick += new System.EventHandler(this.m_RefreshTimer_Tick);
            // 
            // m_ListBox
            // 
            this.m_ListBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_ListBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_ListBox.Font = new System.Drawing.Font("Consolas", 10F);
            this.m_ListBox.FormattingEnabled = true;
            this.m_ListBox.ItemHeight = 15;
            this.m_ListBox.Location = new System.Drawing.Point(10, 22);
            this.m_ListBox.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.m_ListBox.Name = "m_ListBox";
            this.m_ListBox.Size = new System.Drawing.Size(709, 244);
            this.m_ListBox.TabIndex = 6;
            // 
            // VirtualMachineProvisionerDialog
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(730, 334);
            this.ControlBox = false;
            this.Controls.Add(this.m_ListBox);
            this.Controls.Add(this.m_StatusStrip);
            this.Controls.Add(this.m_ExitButton);
            this.Controls.Add(this.m_GoButton);
            this.Controls.Add(this.m_InstructionsLabel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(1, 2, 1, 2);
            this.MinimumSize = new System.Drawing.Size(648, 308);
            this.Name = "VirtualMachineProvisionerDialog";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Ignite! A Microsoft Azure Initializer.";
            this.Load += new System.EventHandler(this.VirtualMachineProvisionerDialog_Load);
            this.m_StatusStrip.ResumeLayout(false);
            this.m_StatusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label m_InstructionsLabel;
        private System.Windows.Forms.Button m_GoButton;
        private System.Windows.Forms.Button m_ExitButton;
        private System.Windows.Forms.StatusStrip m_StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel m_CopyrightToolStripStatusLabel;
        private System.Windows.Forms.Timer m_RefreshTimer;
        private System.Windows.Forms.ListBox m_ListBox;
        private System.Windows.Forms.ToolStripStatusLabel m_ExceptionsStripStatusLabel;
    }
}