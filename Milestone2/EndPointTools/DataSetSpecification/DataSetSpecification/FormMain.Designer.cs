using System.Runtime.InteropServices;

namespace DataSetSpecification
{
    partial class FormMain
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormMain));
            this.ButtonAddNewDataSet = new System.Windows.Forms.Button();
            this.FlowLayoutPanelTables = new System.Windows.Forms.FlowLayoutPanel();
            this.PanelDataSetInfo = new System.Windows.Forms.Panel();
            this.SuspendLayout();
            // 
            // ButtonAddNewDataSet
            // 
            this.ButtonAddNewDataSet.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.ButtonAddNewDataSet.Location = new System.Drawing.Point(17, 13);
            this.ButtonAddNewDataSet.Name = "ButtonAddNewDataSet";
            this.ButtonAddNewDataSet.Size = new System.Drawing.Size(152, 27);
            this.ButtonAddNewDataSet.TabIndex = 21;
            this.ButtonAddNewDataSet.Text = "Add New Table";
            this.ButtonAddNewDataSet.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
            this.ButtonAddNewDataSet.UseVisualStyleBackColor = true;
            this.ButtonAddNewDataSet.Click += new System.EventHandler(this.ButtonAddNewDataSet_Click);
            // 
            // FlowLayoutPanelTables
            // 
            this.FlowLayoutPanelTables.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.FlowLayoutPanelTables.AutoScroll = true;
            this.FlowLayoutPanelTables.AutoSize = true;
            this.FlowLayoutPanelTables.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.FlowLayoutPanelTables.Location = new System.Drawing.Point(12, 49);
            this.FlowLayoutPanelTables.MaximumSize = new System.Drawing.Size(835, 700);
            this.FlowLayoutPanelTables.Name = "FlowLayoutPanelTables";
            this.FlowLayoutPanelTables.Size = new System.Drawing.Size(835, 16);
            this.FlowLayoutPanelTables.TabIndex = 22;
            this.FlowLayoutPanelTables.WrapContents = false;
            // 
            // PanelDataSetInfo
            // 
            this.PanelDataSetInfo.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.PanelDataSetInfo.Location = new System.Drawing.Point(12, 71);
            this.PanelDataSetInfo.Name = "PanelDataSetInfo";
            this.PanelDataSetInfo.Size = new System.Drawing.Size(835, 248);
            this.PanelDataSetInfo.TabIndex = 23;
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(859, 331);
            this.Controls.Add(this.PanelDataSetInfo);
            this.Controls.Add(this.FlowLayoutPanelTables);
            this.Controls.Add(this.ButtonAddNewDataSet);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.MaximumSize = new System.Drawing.Size(900, 1000);
            this.Name = "FormMain";
            this.Text = "SAIL Dataset Specification Tool";
            this.Load += new System.EventHandler(this.FormMain_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button ButtonAddNewDataSet;
        private System.Windows.Forms.FlowLayoutPanel FlowLayoutPanelTables;
        private System.Windows.Forms.Panel PanelDataSetInfo;
    }
}

