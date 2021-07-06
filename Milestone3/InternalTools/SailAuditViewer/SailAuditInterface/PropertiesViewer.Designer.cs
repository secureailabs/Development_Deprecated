
namespace SailAuditInterface
{
    partial class PropertiesViewer
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(PropertiesViewer));
            this.m_PropertiesTextBox = new System.Windows.Forms.TextBox();
            this.m_CloseButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // m_PropertiesTextBox
            // 
            this.m_PropertiesTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_PropertiesTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_PropertiesTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_PropertiesTextBox.Location = new System.Drawing.Point(8, 8);
            this.m_PropertiesTextBox.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.m_PropertiesTextBox.Multiline = true;
            this.m_PropertiesTextBox.Name = "m_PropertiesTextBox";
            this.m_PropertiesTextBox.ReadOnly = true;
            this.m_PropertiesTextBox.Size = new System.Drawing.Size(506, 273);
            this.m_PropertiesTextBox.TabIndex = 0;
            // 
            // m_CloseButton
            // 
            this.m_CloseButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.m_CloseButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_CloseButton.Location = new System.Drawing.Point(208, 285);
            this.m_CloseButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.m_CloseButton.Name = "m_CloseButton";
            this.m_CloseButton.Size = new System.Drawing.Size(104, 24);
            this.m_CloseButton.TabIndex = 1;
            this.m_CloseButton.Text = "Close";
            this.m_CloseButton.UseVisualStyleBackColor = true;
            // 
            // PropertiesViewer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(521, 315);
            this.ControlBox = false;
            this.Controls.Add(this.m_CloseButton);
            this.Controls.Add(this.m_PropertiesTextBox);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.Name = "PropertiesViewer";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Audit Event Properties";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox m_PropertiesTextBox;
        private System.Windows.Forms.Button m_CloseButton;
    }
}