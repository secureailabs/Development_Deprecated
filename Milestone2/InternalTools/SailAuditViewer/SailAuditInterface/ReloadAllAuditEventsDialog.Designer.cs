namespace SailAuditInterface
{
    partial class ReloadAllAuditEventsDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ReloadAllAuditEventsDialog));
            this.m_WaitForAllAuditsToLoadTimer = new System.Windows.Forms.Timer(this.components);
            this.m_ProgressSpinner = new ProgressControls.ProgressIndicator();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // m_WaitForAllAuditsToLoadTimer
            // 
            this.m_WaitForAllAuditsToLoadTimer.Enabled = true;
            this.m_WaitForAllAuditsToLoadTimer.Interval = 50;
            this.m_WaitForAllAuditsToLoadTimer.Tick += new System.EventHandler(this.m_WaitForAllAuditsToLoadTimer_Tick);
            // 
            // m_ProgressSpinner
            // 
            this.m_ProgressSpinner.AutoStart = true;
            this.m_ProgressSpinner.BackColor = System.Drawing.Color.Transparent;
            this.m_ProgressSpinner.CircleColor = System.Drawing.SystemColors.Desktop;
            this.m_ProgressSpinner.CircleSize = 0.6F;
            this.m_ProgressSpinner.Font = new System.Drawing.Font("Segoe UI", 18F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_ProgressSpinner.Location = new System.Drawing.Point(12, 14);
            this.m_ProgressSpinner.Name = "m_ProgressSpinner";
            this.m_ProgressSpinner.NumberOfCircles = 16;
            this.m_ProgressSpinner.NumberOfVisibleCircles = 16;
            this.m_ProgressSpinner.Percentage = 0F;
            this.m_ProgressSpinner.Size = new System.Drawing.Size(62, 62);
            this.m_ProgressSpinner.TabIndex = 0;
            this.m_ProgressSpinner.Text = "Loading...";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(81, 35);
            this.label1.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(148, 19);
            this.label1.TabIndex = 1;
            this.label1.Text = "Loading Audit Events...";
            // 
            // ReloadAllAuditEventsDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.BackColor = System.Drawing.SystemColors.ControlLight;
            this.ClientSize = new System.Drawing.Size(241, 90);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.m_ProgressSpinner);
            this.Font = new System.Drawing.Font("Segoe UI", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.Name = "ReloadAllAuditEventsDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "ReloadAllAuditEventsDialog";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Timer m_WaitForAllAuditsToLoadTimer;
        private ProgressControls.ProgressIndicator m_ProgressSpinner;
        private System.Windows.Forms.Label label1;
    }
}