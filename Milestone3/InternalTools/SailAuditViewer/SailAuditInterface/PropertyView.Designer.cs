
namespace SailAuditInterface
{
    partial class PropertyView
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(PropertyView));
            this.m_PropertyTextBox = new System.Windows.Forms.TextBox();
            this.m_CloseButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // m_PropertyTextBox
            // 
            this.m_PropertyTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_PropertyTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_PropertyTextBox.Font = new System.Drawing.Font("Consolas", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_PropertyTextBox.Location = new System.Drawing.Point(8, 8);
            this.m_PropertyTextBox.Margin = new System.Windows.Forms.Padding(2);
            this.m_PropertyTextBox.Multiline = true;
            this.m_PropertyTextBox.Name = "m_PropertyTextBox";
            this.m_PropertyTextBox.ReadOnly = true;
            this.m_PropertyTextBox.Size = new System.Drawing.Size(961, 329);
            this.m_PropertyTextBox.TabIndex = 1;
            // 
            // m_CloseButton
            // 
            this.m_CloseButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.m_CloseButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_CloseButton.Location = new System.Drawing.Point(442, 342);
            this.m_CloseButton.Margin = new System.Windows.Forms.Padding(2);
            this.m_CloseButton.Name = "m_CloseButton";
            this.m_CloseButton.Size = new System.Drawing.Size(98, 24);
            this.m_CloseButton.TabIndex = 0;
            this.m_CloseButton.Text = "Close";
            this.m_CloseButton.UseVisualStyleBackColor = true;
            // 
            // PropertyView
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(981, 371);
            this.ControlBox = false;
            this.Controls.Add(this.m_CloseButton);
            this.Controls.Add(this.m_PropertyTextBox);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "PropertyView";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Audit Event Property";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox m_PropertyTextBox;
        private System.Windows.Forms.Button m_CloseButton;
    }
}