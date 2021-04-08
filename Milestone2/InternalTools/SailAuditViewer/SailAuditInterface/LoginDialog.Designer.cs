
namespace SailAuditInterface
{
    partial class LoginDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(LoginDialog));
            this.m_UsernameLabel = new System.Windows.Forms.Label();
            this.m_PasswordLabel = new System.Windows.Forms.Label();
            this.m_UsernameEditBox = new System.Windows.Forms.TextBox();
            this.m_PasswordEditBox = new System.Windows.Forms.TextBox();
            this.m_InstructionsLabel = new System.Windows.Forms.Label();
            this.m_LoginButton = new System.Windows.Forms.Button();
            this.m_CancelButton = new System.Windows.Forms.Button();
            this.m_IpAddressTextBox = new System.Windows.Forms.TextBox();
            this.m_IpAddressLabel = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // m_UsernameLabel
            // 
            this.m_UsernameLabel.AutoSize = true;
            this.m_UsernameLabel.Location = new System.Drawing.Point(169, 84);
            this.m_UsernameLabel.Name = "m_UsernameLabel";
            this.m_UsernameLabel.Size = new System.Drawing.Size(60, 15);
            this.m_UsernameLabel.TabIndex = 0;
            this.m_UsernameLabel.Text = "Username";
            // 
            // m_PasswordLabel
            // 
            this.m_PasswordLabel.AutoSize = true;
            this.m_PasswordLabel.Location = new System.Drawing.Point(172, 119);
            this.m_PasswordLabel.Name = "m_PasswordLabel";
            this.m_PasswordLabel.Size = new System.Drawing.Size(57, 15);
            this.m_PasswordLabel.TabIndex = 1;
            this.m_PasswordLabel.Text = "Password";
            // 
            // m_UsernameEditBox
            // 
            this.m_UsernameEditBox.Location = new System.Drawing.Point(235, 81);
            this.m_UsernameEditBox.Name = "m_UsernameEditBox";
            this.m_UsernameEditBox.Size = new System.Drawing.Size(323, 23);
            this.m_UsernameEditBox.TabIndex = 2;
            // 
            // m_PasswordEditBox
            // 
            this.m_PasswordEditBox.Location = new System.Drawing.Point(235, 116);
            this.m_PasswordEditBox.Name = "m_PasswordEditBox";
            this.m_PasswordEditBox.PasswordChar = '*';
            this.m_PasswordEditBox.Size = new System.Drawing.Size(323, 23);
            this.m_PasswordEditBox.TabIndex = 3;
            // 
            // m_InstructionsLabel
            // 
            this.m_InstructionsLabel.AutoSize = true;
            this.m_InstructionsLabel.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_InstructionsLabel.Location = new System.Drawing.Point(167, 18);
            this.m_InstructionsLabel.Name = "m_InstructionsLabel";
            this.m_InstructionsLabel.Size = new System.Drawing.Size(391, 15);
            this.m_InstructionsLabel.TabIndex = 0;
            this.m_InstructionsLabel.Text = "Please enter your SAIL Credentials. The username your email address.";
            // 
            // m_LoginButton
            // 
            this.m_LoginButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.m_LoginButton.Location = new System.Drawing.Point(235, 155);
            this.m_LoginButton.Name = "m_LoginButton";
            this.m_LoginButton.Size = new System.Drawing.Size(128, 28);
            this.m_LoginButton.TabIndex = 4;
            this.m_LoginButton.Text = "&Login";
            this.m_LoginButton.UseVisualStyleBackColor = true;
            this.m_LoginButton.Click += new System.EventHandler(this.m_LoginButton_Click);
            // 
            // m_CancelButton
            // 
            this.m_CancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_CancelButton.Location = new System.Drawing.Point(369, 155);
            this.m_CancelButton.Name = "m_CancelButton";
            this.m_CancelButton.Size = new System.Drawing.Size(128, 28);
            this.m_CancelButton.TabIndex = 5;
            this.m_CancelButton.Text = "&Cancel";
            this.m_CancelButton.UseVisualStyleBackColor = true;
            this.m_CancelButton.Click += new System.EventHandler(this.m_CancelButton_Click);
            // 
            // m_IpAddressTextBox
            // 
            this.m_IpAddressTextBox.Location = new System.Drawing.Point(235, 46);
            this.m_IpAddressTextBox.Name = "m_IpAddressTextBox";
            this.m_IpAddressTextBox.Size = new System.Drawing.Size(323, 23);
            this.m_IpAddressTextBox.TabIndex = 1;
            // 
            // m_IpAddressLabel
            // 
            this.m_IpAddressLabel.AutoSize = true;
            this.m_IpAddressLabel.Location = new System.Drawing.Point(167, 49);
            this.m_IpAddressLabel.Name = "m_IpAddressLabel";
            this.m_IpAddressLabel.Size = new System.Drawing.Size(62, 15);
            this.m_IpAddressLabel.TabIndex = 8;
            this.m_IpAddressLabel.Text = "IP Address";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::SailAuditInterface.Properties.Resources.SAIL_Icon;
            this.pictureBox1.InitialImage = ((System.Drawing.Image)(resources.GetObject("pictureBox1.InitialImage")));
            this.pictureBox1.Location = new System.Drawing.Point(18, 33);
            this.pictureBox1.Margin = new System.Windows.Forms.Padding(4);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(135, 133);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 9;
            this.pictureBox1.TabStop = false;
            // 
            // LoginDialog
            // 
            this.AcceptButton = this.m_LoginButton;
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.CancelButton = this.m_CancelButton;
            this.ClientSize = new System.Drawing.Size(574, 198);
            this.ControlBox = false;
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.m_IpAddressLabel);
            this.Controls.Add(this.m_IpAddressTextBox);
            this.Controls.Add(this.m_CancelButton);
            this.Controls.Add(this.m_LoginButton);
            this.Controls.Add(this.m_InstructionsLabel);
            this.Controls.Add(this.m_PasswordEditBox);
            this.Controls.Add(this.m_UsernameEditBox);
            this.Controls.Add(this.m_PasswordLabel);
            this.Controls.Add(this.m_UsernameLabel);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.Name = "LoginDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "SAIL Audit Viewer";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
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
        private System.Windows.Forms.PictureBox pictureBox1;
    }
}

