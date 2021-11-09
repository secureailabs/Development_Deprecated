
namespace WindowsRemoteDataConnector
{
    partial class ConnectToApiWebServices
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
            this.m_WaitingProgressBar = new System.Windows.Forms.ProgressBar();
            this.label1 = new System.Windows.Forms.Label();
            this.m_QuitTryingAndExitButton = new System.Windows.Forms.Button();
            this.m_RetryTimer = new System.Windows.Forms.Timer(this.components);
            this.m_RefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // m_WaitingProgressBar
            // 
            this.m_WaitingProgressBar.Location = new System.Drawing.Point(13, 31);
            this.m_WaitingProgressBar.Name = "m_WaitingProgressBar";
            this.m_WaitingProgressBar.Size = new System.Drawing.Size(413, 19);
            this.m_WaitingProgressBar.Style = System.Windows.Forms.ProgressBarStyle.Marquee;
            this.m_WaitingProgressBar.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(11, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(417, 19);
            this.label1.TabIndex = 1;
            this.label1.Text = "Lost connection with SAIL Web Services. Attempting to reconnect...";
            // 
            // m_QuitTryingAndExitButton
            // 
            this.m_QuitTryingAndExitButton.Location = new System.Drawing.Point(133, 56);
            this.m_QuitTryingAndExitButton.Name = "m_QuitTryingAndExitButton";
            this.m_QuitTryingAndExitButton.Size = new System.Drawing.Size(172, 29);
            this.m_QuitTryingAndExitButton.TabIndex = 2;
            this.m_QuitTryingAndExitButton.Text = "Quit Trying and Exit";
            this.m_QuitTryingAndExitButton.UseVisualStyleBackColor = true;
            this.m_QuitTryingAndExitButton.Click += new System.EventHandler(this.m_QuitTryingAndExitButton_Click);
            // 
            // m_RetryTimer
            // 
            this.m_RetryTimer.Enabled = true;
            this.m_RetryTimer.Interval = 10000;
            this.m_RetryTimer.Tick += new System.EventHandler(this.m_RetryTimer_Tick);
            // 
            // m_RefreshTimer
            // 
            this.m_RefreshTimer.Enabled = true;
            this.m_RefreshTimer.Tick += new System.EventHandler(this.m_RefreshTimer_Tick);
            // 
            // ConnectToApiWebServices
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 17F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(439, 97);
            this.Controls.Add(this.m_QuitTryingAndExitButton);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.m_WaitingProgressBar);
            this.Font = new System.Drawing.Font("Segoe UI", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "ConnectToApiWebServices";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ProgressBar m_WaitingProgressBar;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button m_QuitTryingAndExitButton;
        private System.Windows.Forms.Timer m_RetryTimer;
        private System.Windows.Forms.Timer m_RefreshTimer;
    }
}