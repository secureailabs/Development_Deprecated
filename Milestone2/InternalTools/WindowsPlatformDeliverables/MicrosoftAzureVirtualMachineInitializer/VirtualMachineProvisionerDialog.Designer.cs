
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
            this.m_CancelButton = new System.Windows.Forms.Button();
            this.m_ExitButton = new System.Windows.Forms.Button();
            this.m_StatusStrip = new System.Windows.Forms.StatusStrip();
            this.m_CopyrightToolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_RefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.m_ListBox = new System.Windows.Forms.ListBox();
            this.m_StatusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_InstructionsLabel
            // 
            this.m_InstructionsLabel.AutoSize = true;
            this.m_InstructionsLabel.Location = new System.Drawing.Point(6, 4);
            this.m_InstructionsLabel.Margin = new System.Windows.Forms.Padding(1, 0, 1, 0);
            this.m_InstructionsLabel.Name = "m_InstructionsLabel";
            this.m_InstructionsLabel.Size = new System.Drawing.Size(605, 15);
            this.m_InstructionsLabel.TabIndex = 1;
            this.m_InstructionsLabel.Text = "Press \"Go!\" in order to trigger the provisioning and initialization process of al" +
    "l of the virtual machines listed below.";
            // 
            // m_GoButton
            // 
            this.m_GoButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.m_GoButton.Location = new System.Drawing.Point(214, 221);
            this.m_GoButton.Margin = new System.Windows.Forms.Padding(1, 1, 1, 1);
            this.m_GoButton.Name = "m_GoButton";
            this.m_GoButton.Size = new System.Drawing.Size(71, 22);
            this.m_GoButton.TabIndex = 2;
            this.m_GoButton.Text = "Go!";
            this.m_GoButton.UseVisualStyleBackColor = true;
            this.m_GoButton.Click += new System.EventHandler(this.m_GoButton_Click);
            // 
            // m_CancelButton
            // 
            this.m_CancelButton.DialogResult = System.Windows.Forms.DialogResult.Abort;
            this.m_CancelButton.Location = new System.Drawing.Point(288, 221);
            this.m_CancelButton.Margin = new System.Windows.Forms.Padding(1, 1, 1, 1);
            this.m_CancelButton.Name = "m_CancelButton";
            this.m_CancelButton.Size = new System.Drawing.Size(71, 22);
            this.m_CancelButton.TabIndex = 3;
            this.m_CancelButton.Text = "Cancel";
            this.m_CancelButton.UseVisualStyleBackColor = true;
            this.m_CancelButton.Click += new System.EventHandler(this.m_CancelButton_Click);
            // 
            // m_ExitButton
            // 
            this.m_ExitButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_ExitButton.Location = new System.Drawing.Point(363, 221);
            this.m_ExitButton.Margin = new System.Windows.Forms.Padding(1, 1, 1, 1);
            this.m_ExitButton.Name = "m_ExitButton";
            this.m_ExitButton.Size = new System.Drawing.Size(71, 22);
            this.m_ExitButton.TabIndex = 4;
            this.m_ExitButton.Text = "Exit";
            this.m_ExitButton.UseVisualStyleBackColor = true;
            this.m_ExitButton.Click += new System.EventHandler(this.m_ExitButton_Click);
            // 
            // m_StatusStrip
            // 
            this.m_StatusStrip.AutoSize = false;
            this.m_StatusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_CopyrightToolStripStatusLabel});
            this.m_StatusStrip.Location = new System.Drawing.Point(0, 250);
            this.m_StatusStrip.Name = "m_StatusStrip";
            this.m_StatusStrip.Padding = new System.Windows.Forms.Padding(1, 0, 6, 0);
            this.m_StatusStrip.Size = new System.Drawing.Size(647, 13);
            this.m_StatusStrip.TabIndex = 5;
            this.m_StatusStrip.Text = "statusStrip1";
            // 
            // m_CopyrightToolStripStatusLabel
            // 
            this.m_CopyrightToolStripStatusLabel.Name = "m_CopyrightToolStripStatusLabel";
            this.m_CopyrightToolStripStatusLabel.Size = new System.Drawing.Size(210, 8);
            this.m_CopyrightToolStripStatusLabel.Text = "Copyright (C) 2021 Secure AI Labs, Inc.";
            this.m_CopyrightToolStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_RefreshTimer
            // 
            this.m_RefreshTimer.Enabled = true;
            this.m_RefreshTimer.Interval = 250;
            this.m_RefreshTimer.Tick += new System.EventHandler(this.m_RefreshTimer_Tick);
            // 
            // m_ListBox
            // 
            this.m_ListBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_ListBox.Font = new System.Drawing.Font("Consolas", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.m_ListBox.FormattingEnabled = true;
            this.m_ListBox.ItemHeight = 15;
            this.m_ListBox.Location = new System.Drawing.Point(8, 20);
            this.m_ListBox.Margin = new System.Windows.Forms.Padding(1, 1, 1, 1);
            this.m_ListBox.Name = "m_ListBox";
            this.m_ListBox.Size = new System.Drawing.Size(633, 184);
            this.m_ListBox.TabIndex = 6;
            // 
            // VirtualMachineProvisionerDialog
            // 
            this.AcceptButton = this.m_GoButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.m_ExitButton;
            this.ClientSize = new System.Drawing.Size(647, 263);
            this.ControlBox = false;
            this.Controls.Add(this.m_ListBox);
            this.Controls.Add(this.m_StatusStrip);
            this.Controls.Add(this.m_ExitButton);
            this.Controls.Add(this.m_CancelButton);
            this.Controls.Add(this.m_GoButton);
            this.Controls.Add(this.m_InstructionsLabel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(1, 1, 1, 1);
            this.Name = "VirtualMachineProvisionerDialog";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Ignite! a Microsoft Azure Initializer";
            this.Load += new System.EventHandler(this.VirtualMachineProvisionerDialog_Load);
            this.m_StatusStrip.ResumeLayout(false);
            this.m_StatusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label m_InstructionsLabel;
        private System.Windows.Forms.Button m_GoButton;
        private System.Windows.Forms.Button m_CancelButton;
        private System.Windows.Forms.Button m_ExitButton;
        private System.Windows.Forms.StatusStrip m_StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel m_CopyrightToolStripStatusLabel;
        private System.Windows.Forms.Timer m_RefreshTimer;
        private System.Windows.Forms.ListBox m_ListBox;
    }
}