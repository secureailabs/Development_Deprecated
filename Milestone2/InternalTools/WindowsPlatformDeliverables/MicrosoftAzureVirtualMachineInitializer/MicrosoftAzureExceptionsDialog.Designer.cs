
namespace MicrosoftAzureVirtualMachineInitializer
{
    partial class MicrosoftAzureExceptionsDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MicrosoftAzureExceptionsDialog));
            this.m_CloseExceptionsDialogButton = new System.Windows.Forms.Button();
            this.m_ClearExceptionsButton = new System.Windows.Forms.Button();
            this.m_ExceptionsListBox = new System.Windows.Forms.ListBox();
            this.SuspendLayout();
            // 
            // m_CloseExceptionsDialogButton
            // 
            this.m_CloseExceptionsDialogButton.Location = new System.Drawing.Point(792, 444);
            this.m_CloseExceptionsDialogButton.Name = "m_CloseExceptionsDialogButton";
            this.m_CloseExceptionsDialogButton.Size = new System.Drawing.Size(129, 32);
            this.m_CloseExceptionsDialogButton.TabIndex = 1;
            this.m_CloseExceptionsDialogButton.Text = "Close";
            this.m_CloseExceptionsDialogButton.UseVisualStyleBackColor = true;
            this.m_CloseExceptionsDialogButton.Click += new System.EventHandler(this.m_CloseExceptionsDialogButton_Click);
            // 
            // m_ClearExceptionsButton
            // 
            this.m_ClearExceptionsButton.Location = new System.Drawing.Point(13, 444);
            this.m_ClearExceptionsButton.Name = "m_ClearExceptionsButton";
            this.m_ClearExceptionsButton.Size = new System.Drawing.Size(166, 32);
            this.m_ClearExceptionsButton.TabIndex = 2;
            this.m_ClearExceptionsButton.Text = "Clear Exceptions";
            this.m_ClearExceptionsButton.UseVisualStyleBackColor = true;
            this.m_ClearExceptionsButton.Click += new System.EventHandler(this.m_ClearExceptionsButton_Click);
            // 
            // m_ExceptionsListBox
            // 
            this.m_ExceptionsListBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
            | System.Windows.Forms.AnchorStyles.Left)
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_ExceptionsListBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_ExceptionsListBox.FormattingEnabled = true;
            this.m_ExceptionsListBox.HorizontalScrollbar = true;
            this.m_ExceptionsListBox.IntegralHeight = false;
            this.m_ExceptionsListBox.ItemHeight = 14;
            this.m_ExceptionsListBox.Location = new System.Drawing.Point(12, 12);
            this.m_ExceptionsListBox.Name = "m_ExceptionsListBox";
            this.m_ExceptionsListBox.Size = new System.Drawing.Size(909, 424);
            this.m_ExceptionsListBox.TabIndex = 3;
            // 
            // ExceptionsDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.ClientSize = new System.Drawing.Size(933, 485);
            this.ControlBox = false;
            this.Controls.Add(this.m_ExceptionsListBox);
            this.Controls.Add(this.m_ClearExceptionsButton);
            this.Controls.Add(this.m_CloseExceptionsDialogButton);
            this.DoubleBuffered = true;
            this.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "ExceptionsDialog";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "ExceptionsDialog";
            this.TopMost = true;
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button m_CloseExceptionsDialogButton;
        private System.Windows.Forms.Button m_ClearExceptionsButton;
        private System.Windows.Forms.ListBox m_ExceptionsListBox;
    }
}