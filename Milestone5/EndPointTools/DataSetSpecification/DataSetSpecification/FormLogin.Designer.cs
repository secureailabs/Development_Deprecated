namespace DataSetSpecification
{
    partial class FormLogin
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormLogin));
            this.userID = new System.Windows.Forms.Label();
            this.password = new System.Windows.Forms.Label();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.loginStatus = new System.Windows.Forms.Label();
            this.textBoxWebPortalUrl = new System.Windows.Forms.TextBox();
            this.labelPortalUrl = new System.Windows.Forms.Label();
            this.checkBoxDatasetRegister = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // userID
            // 
            this.userID.AutoSize = true;
            this.userID.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.userID.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.userID.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.userID.Location = new System.Drawing.Point(254, 71);
            this.userID.Name = "userID";
            this.userID.Size = new System.Drawing.Size(62, 22);
            this.userID.TabIndex = 14;
            this.userID.Text = "UserID";
            // 
            // password
            // 
            this.password.AutoSize = true;
            this.password.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.password.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.password.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.password.Location = new System.Drawing.Point(236, 114);
            this.password.Name = "password";
            this.password.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.password.Size = new System.Drawing.Size(80, 22);
            this.password.TabIndex = 15;
            this.password.Text = "Password";
            // 
            // textBox1
            // 
            this.textBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBox1.Location = new System.Drawing.Point(325, 67);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(184, 26);
            this.textBox1.TabIndex = 16;
            this.textBox1.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // textBox2
            // 
            this.textBox2.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBox2.Location = new System.Drawing.Point(324, 110);
            this.textBox2.Name = "textBox2";
            this.textBox2.PasswordChar = '*';
            this.textBox2.Size = new System.Drawing.Size(184, 26);
            this.textBox2.TabIndex = 17;
            this.textBox2.TextChanged += new System.EventHandler(this.textBox2_TextChanged);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(340, 174);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(109, 31);
            this.button1.TabIndex = 18;
            this.button1.Text = "Login";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(12, 66);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(172, 75);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 19;
            this.pictureBox1.TabStop = false;
            // 
            // loginStatus
            // 
            this.loginStatus.AutoSize = true;
            this.loginStatus.ForeColor = System.Drawing.Color.Red;
            this.loginStatus.Location = new System.Drawing.Point(344, 212);
            this.loginStatus.Name = "loginStatus";
            this.loginStatus.Size = new System.Drawing.Size(0, 13);
            this.loginStatus.TabIndex = 20;
            // 
            // textBoxWebPortalUrl
            // 
            this.textBoxWebPortalUrl.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBoxWebPortalUrl.Location = new System.Drawing.Point(325, 28);
            this.textBoxWebPortalUrl.Name = "textBoxWebPortalUrl";
            this.textBoxWebPortalUrl.Size = new System.Drawing.Size(184, 26);
            this.textBoxWebPortalUrl.TabIndex = 22;
            // 
            // labelPortalUrl
            // 
            this.labelPortalUrl.AutoSize = true;
            this.labelPortalUrl.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.labelPortalUrl.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.labelPortalUrl.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelPortalUrl.Location = new System.Drawing.Point(207, 32);
            this.labelPortalUrl.Name = "labelPortalUrl";
            this.labelPortalUrl.Size = new System.Drawing.Size(109, 22);
            this.labelPortalUrl.TabIndex = 21;
            this.labelPortalUrl.Text = "WebPortal Url";
            // 
            // checkBoxDatasetRegister
            // 
            this.checkBoxDatasetRegister.AutoSize = true;
            this.checkBoxDatasetRegister.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.checkBoxDatasetRegister.Location = new System.Drawing.Point(285, 146);
            this.checkBoxDatasetRegister.Name = "checkBoxDatasetRegister";
            this.checkBoxDatasetRegister.Size = new System.Drawing.Size(201, 22);
            this.checkBoxDatasetRegister.TabIndex = 23;
            this.checkBoxDatasetRegister.Text = "Register Dataset on Portal";
            this.checkBoxDatasetRegister.UseVisualStyleBackColor = true;
            // 
            // FormLogin
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(529, 228);
            this.Controls.Add(this.checkBoxDatasetRegister);
            this.Controls.Add(this.textBoxWebPortalUrl);
            this.Controls.Add(this.labelPortalUrl);
            this.Controls.Add(this.loginStatus);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.textBox2);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.password);
            this.Controls.Add(this.userID);
            this.Name = "FormLogin";
            this.Text = "Login";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label userID;
        private System.Windows.Forms.Label password;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.TextBox textBox2;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label loginStatus;
        private System.Windows.Forms.TextBox textBoxWebPortalUrl;
        private System.Windows.Forms.Label labelPortalUrl;
        private System.Windows.Forms.CheckBox checkBoxDatasetRegister;
    }
}