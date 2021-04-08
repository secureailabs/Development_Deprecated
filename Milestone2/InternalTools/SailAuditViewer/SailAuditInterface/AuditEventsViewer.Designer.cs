
namespace SailAuditInterface
{
    partial class AuditEventsViewer
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
            System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem(new string[] {
            "0",
            "September 30th, 2021",
            "22:22:22:2222",
            "PUSH_FN"}, -1);
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AuditEventsViewer));
            System.Windows.Forms.ListViewItem listViewItem2 = new System.Windows.Forms.ListViewItem(new string[] {
            "9999/99/99 @ 99:99:99:9999 GMT",
            "PUSH_DATA",
            ""}, -1);
            this.m_RefreshButton = new System.Windows.Forms.Button();
            this.m_ExitButton = new System.Windows.Forms.Button();
            this.m_EventTypeFiltersGroupBox = new System.Windows.Forms.GroupBox();
            this.m_ViewJobEngineActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewAllActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewPullDataActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewPushDataActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewPushFnActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewExecFnActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewDataConnectorActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewVmActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewDatasetActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewDigitalContractActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewAuditLogAccessesActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewUserOrganizationActivityRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewAccessesRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewInformationalRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewWarningsRadioButton = new System.Windows.Forms.RadioButton();
            this.m_ViewErrorsRadioButton = new System.Windows.Forms.RadioButton();
            this.m_BackButton = new System.Windows.Forms.Button();
            this.m_CurrentBranchNodeTextBox = new System.Windows.Forms.TextBox();
            this.m_AuditEventsListView = new System.Windows.Forms.ListView();
            this.m_SequenceNumberColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.m_EpochDateColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.m_EpochTimeColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.m_EventTypeColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.m_AuditEventPropertiesTextBox = new System.Windows.Forms.TextBox();
            this.m_TreeViewContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.m_CopyCurrentAuditEventPropertiesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.m_ViewAdditionalDataDetailsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.m_DateTimeFiltersGroupBox = new System.Windows.Forms.GroupBox();
            this.m_ResetLatestDateTimePickerButton = new System.Windows.Forms.Button();
            this.m_ResetEarliestDateTimePickerButton = new System.Windows.Forms.Button();
            this.m_LatestDateTimePickerLabel = new System.Windows.Forms.Label();
            this.m_StartTimeLabel = new System.Windows.Forms.Label();
            this.m_LatestDateTimePicker = new System.Windows.Forms.DateTimePicker();
            this.m_EarliestDateTimePicker = new System.Windows.Forms.DateTimePicker();
            this.m_AuditEventViewTabControl = new System.Windows.Forms.TabControl();
            this.m_TreeViewTabPage = new System.Windows.Forms.TabPage();
            this.m_FlatViewTabPage = new System.Windows.Forms.TabPage();
            this.m_FlatViewPropertiesTextBox = new System.Windows.Forms.TextBox();
            this.m_FlatViewListView = new System.Windows.Forms.ListView();
            this.m_DateTimeInFlatListViewColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.m_EventNameInFlatListViewColumnHeader = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.m_StatusStrip = new System.Windows.Forms.StatusStrip();
            this.m_TimeStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_MessageStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_ErrorStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_StatusBarRefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.m_EventTypeFiltersGroupBox.SuspendLayout();
            this.m_TreeViewContextMenu.SuspendLayout();
            this.m_DateTimeFiltersGroupBox.SuspendLayout();
            this.m_AuditEventViewTabControl.SuspendLayout();
            this.m_TreeViewTabPage.SuspendLayout();
            this.m_FlatViewTabPage.SuspendLayout();
            this.m_StatusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_RefreshButton
            // 
            this.m_RefreshButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.m_RefreshButton.DialogResult = System.Windows.Forms.DialogResult.Retry;
            this.m_RefreshButton.Location = new System.Drawing.Point(22, 302);
            this.m_RefreshButton.Name = "m_RefreshButton";
            this.m_RefreshButton.Size = new System.Drawing.Size(125, 27);
            this.m_RefreshButton.TabIndex = 6;
            this.m_RefreshButton.Text = "&Refresh";
            this.m_RefreshButton.UseVisualStyleBackColor = true;
            this.m_RefreshButton.Click += new System.EventHandler(this.m_RefreshButton_Click);
            this.m_RefreshButton.MouseEnter += new System.EventHandler(this.m_RefreshButton_MouseEnter);
            this.m_RefreshButton.MouseLeave += new System.EventHandler(this.Generic_MouseLeave);
            // 
            // m_ExitButton
            // 
            this.m_ExitButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.m_ExitButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_ExitButton.Location = new System.Drawing.Point(834, 302);
            this.m_ExitButton.Name = "m_ExitButton";
            this.m_ExitButton.Size = new System.Drawing.Size(125, 27);
            this.m_ExitButton.TabIndex = 9;
            this.m_ExitButton.Text = "E&xit";
            this.m_ExitButton.UseVisualStyleBackColor = true;
            this.m_ExitButton.Click += new System.EventHandler(this.m_CloseButton_Click);
            // 
            // m_EventTypeFiltersGroupBox
            // 
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewJobEngineActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewAllActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewPullDataActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewPushDataActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewPushFnActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewExecFnActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewDataConnectorActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewVmActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewDatasetActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewDigitalContractActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewAuditLogAccessesActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewUserOrganizationActivityRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewAccessesRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewInformationalRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewWarningsRadioButton);
            this.m_EventTypeFiltersGroupBox.Controls.Add(this.m_ViewErrorsRadioButton);
            this.m_EventTypeFiltersGroupBox.Location = new System.Drawing.Point(6, 6);
            this.m_EventTypeFiltersGroupBox.Name = "m_EventTypeFiltersGroupBox";
            this.m_EventTypeFiltersGroupBox.Size = new System.Drawing.Size(587, 135);
            this.m_EventTypeFiltersGroupBox.TabIndex = 3;
            this.m_EventTypeFiltersGroupBox.TabStop = false;
            this.m_EventTypeFiltersGroupBox.Text = "Audit Event Type Filters";
            // 
            // m_ViewJobEngineActivityRadioButton
            // 
            this.m_ViewJobEngineActivityRadioButton.AutoSize = true;
            this.m_ViewJobEngineActivityRadioButton.Location = new System.Drawing.Point(305, 73);
            this.m_ViewJobEngineActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewJobEngineActivityRadioButton.Name = "m_ViewJobEngineActivityRadioButton";
            this.m_ViewJobEngineActivityRadioButton.Size = new System.Drawing.Size(125, 19);
            this.m_ViewJobEngineActivityRadioButton.TabIndex = 15;
            this.m_ViewJobEngineActivityRadioButton.TabStop = true;
            this.m_ViewJobEngineActivityRadioButton.Text = "Job Engine Activity";
            this.m_ViewJobEngineActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewJobEngineActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewJobEngineActivityRadioButton_CheckedChanged);
            // 
            // m_ViewAllActivityRadioButton
            // 
            this.m_ViewAllActivityRadioButton.AutoSize = true;
            this.m_ViewAllActivityRadioButton.Checked = true;
            this.m_ViewAllActivityRadioButton.Location = new System.Drawing.Point(468, 98);
            this.m_ViewAllActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewAllActivityRadioButton.Name = "m_ViewAllActivityRadioButton";
            this.m_ViewAllActivityRadioButton.Size = new System.Drawing.Size(75, 19);
            this.m_ViewAllActivityRadioButton.TabIndex = 14;
            this.m_ViewAllActivityRadioButton.TabStop = true;
            this.m_ViewAllActivityRadioButton.Text = "VIEW ALL";
            this.m_ViewAllActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewAllActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewDeleteDataActivityRadioButton_CheckedChanged);
            // 
            // m_ViewPullDataActivityRadioButton
            // 
            this.m_ViewPullDataActivityRadioButton.AutoSize = true;
            this.m_ViewPullDataActivityRadioButton.Location = new System.Drawing.Point(468, 73);
            this.m_ViewPullDataActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewPullDataActivityRadioButton.Name = "m_ViewPullDataActivityRadioButton";
            this.m_ViewPullDataActivityRadioButton.Size = new System.Drawing.Size(85, 19);
            this.m_ViewPullDataActivityRadioButton.TabIndex = 13;
            this.m_ViewPullDataActivityRadioButton.TabStop = true;
            this.m_ViewPullDataActivityRadioButton.Text = "PULL_DATA";
            this.m_ViewPullDataActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewPullDataActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewPullDataActivityRadioButton_CheckedChanged);
            // 
            // m_ViewPushDataActivityRadioButton
            // 
            this.m_ViewPushDataActivityRadioButton.AutoSize = true;
            this.m_ViewPushDataActivityRadioButton.Location = new System.Drawing.Point(468, 48);
            this.m_ViewPushDataActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewPushDataActivityRadioButton.Name = "m_ViewPushDataActivityRadioButton";
            this.m_ViewPushDataActivityRadioButton.Size = new System.Drawing.Size(88, 19);
            this.m_ViewPushDataActivityRadioButton.TabIndex = 12;
            this.m_ViewPushDataActivityRadioButton.TabStop = true;
            this.m_ViewPushDataActivityRadioButton.Text = "PUSH_DATA";
            this.m_ViewPushDataActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewPushDataActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewPushDataActivityRadioButton_CheckedChanged);
            // 
            // m_ViewPushFnActivityRadioButton
            // 
            this.m_ViewPushFnActivityRadioButton.AutoSize = true;
            this.m_ViewPushFnActivityRadioButton.Location = new System.Drawing.Point(305, 98);
            this.m_ViewPushFnActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewPushFnActivityRadioButton.Name = "m_ViewPushFnActivityRadioButton";
            this.m_ViewPushFnActivityRadioButton.Size = new System.Drawing.Size(75, 19);
            this.m_ViewPushFnActivityRadioButton.TabIndex = 11;
            this.m_ViewPushFnActivityRadioButton.TabStop = true;
            this.m_ViewPushFnActivityRadioButton.Text = "PUSH_FN";
            this.m_ViewPushFnActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewPushFnActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewPushFnActivityRadioButton_CheckedChanged);
            // 
            // m_ViewExecFnActivityRadioButton
            // 
            this.m_ViewExecFnActivityRadioButton.AutoSize = true;
            this.m_ViewExecFnActivityRadioButton.Location = new System.Drawing.Point(468, 23);
            this.m_ViewExecFnActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewExecFnActivityRadioButton.Name = "m_ViewExecFnActivityRadioButton";
            this.m_ViewExecFnActivityRadioButton.Size = new System.Drawing.Size(72, 19);
            this.m_ViewExecFnActivityRadioButton.TabIndex = 10;
            this.m_ViewExecFnActivityRadioButton.TabStop = true;
            this.m_ViewExecFnActivityRadioButton.Text = "EXEC_FN";
            this.m_ViewExecFnActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewExecFnActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewExecFnActivityRadioButton_CheckedChanged);
            // 
            // m_ViewDataConnectorActivityRadioButton
            // 
            this.m_ViewDataConnectorActivityRadioButton.AutoSize = true;
            this.m_ViewDataConnectorActivityRadioButton.Location = new System.Drawing.Point(305, 48);
            this.m_ViewDataConnectorActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewDataConnectorActivityRadioButton.Name = "m_ViewDataConnectorActivityRadioButton";
            this.m_ViewDataConnectorActivityRadioButton.Size = new System.Drawing.Size(151, 19);
            this.m_ViewDataConnectorActivityRadioButton.TabIndex = 9;
            this.m_ViewDataConnectorActivityRadioButton.TabStop = true;
            this.m_ViewDataConnectorActivityRadioButton.Text = "Data Connector Activity";
            this.m_ViewDataConnectorActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewDataConnectorActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewDataConnectorActivityRadioButton_CheckedChanged);
            // 
            // m_ViewVmActivityRadioButton
            // 
            this.m_ViewVmActivityRadioButton.AutoSize = true;
            this.m_ViewVmActivityRadioButton.Location = new System.Drawing.Point(305, 23);
            this.m_ViewVmActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewVmActivityRadioButton.Name = "m_ViewVmActivityRadioButton";
            this.m_ViewVmActivityRadioButton.Size = new System.Drawing.Size(86, 19);
            this.m_ViewVmActivityRadioButton.TabIndex = 8;
            this.m_ViewVmActivityRadioButton.TabStop = true;
            this.m_ViewVmActivityRadioButton.Text = "VM Activity";
            this.m_ViewVmActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewVmActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewVmActivityRadioButton_CheckedChanged);
            // 
            // m_ViewDatasetActivityRadioButton
            // 
            this.m_ViewDatasetActivityRadioButton.AutoSize = true;
            this.m_ViewDatasetActivityRadioButton.Location = new System.Drawing.Point(126, 98);
            this.m_ViewDatasetActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewDatasetActivityRadioButton.Name = "m_ViewDatasetActivityRadioButton";
            this.m_ViewDatasetActivityRadioButton.Size = new System.Drawing.Size(107, 19);
            this.m_ViewDatasetActivityRadioButton.TabIndex = 7;
            this.m_ViewDatasetActivityRadioButton.TabStop = true;
            this.m_ViewDatasetActivityRadioButton.Text = "Dataset Activity";
            this.m_ViewDatasetActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewDatasetActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewDatasetActivityRadioButton_CheckedChanged);
            // 
            // m_ViewDigitalContractActivityRadioButton
            // 
            this.m_ViewDigitalContractActivityRadioButton.AutoSize = true;
            this.m_ViewDigitalContractActivityRadioButton.Location = new System.Drawing.Point(126, 73);
            this.m_ViewDigitalContractActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewDigitalContractActivityRadioButton.Name = "m_ViewDigitalContractActivityRadioButton";
            this.m_ViewDigitalContractActivityRadioButton.Size = new System.Drawing.Size(151, 19);
            this.m_ViewDigitalContractActivityRadioButton.TabIndex = 6;
            this.m_ViewDigitalContractActivityRadioButton.TabStop = true;
            this.m_ViewDigitalContractActivityRadioButton.Text = "Digital Contract Activity";
            this.m_ViewDigitalContractActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewDigitalContractActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewDigitalContractActivityRadioButton_CheckedChanged);
            // 
            // m_ViewAuditLogAccessesActivityRadioButton
            // 
            this.m_ViewAuditLogAccessesActivityRadioButton.AutoSize = true;
            this.m_ViewAuditLogAccessesActivityRadioButton.Location = new System.Drawing.Point(126, 48);
            this.m_ViewAuditLogAccessesActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewAuditLogAccessesActivityRadioButton.Name = "m_ViewAuditLogAccessesActivityRadioButton";
            this.m_ViewAuditLogAccessesActivityRadioButton.Size = new System.Drawing.Size(127, 19);
            this.m_ViewAuditLogAccessesActivityRadioButton.TabIndex = 5;
            this.m_ViewAuditLogAccessesActivityRadioButton.TabStop = true;
            this.m_ViewAuditLogAccessesActivityRadioButton.Text = "Audit Log Accesses";
            this.m_ViewAuditLogAccessesActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewAuditLogAccessesActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewAuditLogAccessesActivityRadioButton_CheckedChanged);
            // 
            // m_ViewUserOrganizationActivityRadioButton
            // 
            this.m_ViewUserOrganizationActivityRadioButton.AutoSize = true;
            this.m_ViewUserOrganizationActivityRadioButton.Location = new System.Drawing.Point(126, 23);
            this.m_ViewUserOrganizationActivityRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewUserOrganizationActivityRadioButton.Name = "m_ViewUserOrganizationActivityRadioButton";
            this.m_ViewUserOrganizationActivityRadioButton.Size = new System.Drawing.Size(164, 19);
            this.m_ViewUserOrganizationActivityRadioButton.TabIndex = 4;
            this.m_ViewUserOrganizationActivityRadioButton.TabStop = true;
            this.m_ViewUserOrganizationActivityRadioButton.Text = "User/Organization Activity";
            this.m_ViewUserOrganizationActivityRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewUserOrganizationActivityRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewYserOragnizationActivityRadioButton_CheckedChanged);
            // 
            // m_ViewAccessesRadioButton
            // 
            this.m_ViewAccessesRadioButton.AutoSize = true;
            this.m_ViewAccessesRadioButton.Location = new System.Drawing.Point(16, 98);
            this.m_ViewAccessesRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewAccessesRadioButton.Name = "m_ViewAccessesRadioButton";
            this.m_ViewAccessesRadioButton.Size = new System.Drawing.Size(72, 19);
            this.m_ViewAccessesRadioButton.TabIndex = 3;
            this.m_ViewAccessesRadioButton.TabStop = true;
            this.m_ViewAccessesRadioButton.Text = "Accesses";
            this.m_ViewAccessesRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewAccessesRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewAccessesRadioButton_CheckedChanged);
            // 
            // m_ViewInformationalRadioButton
            // 
            this.m_ViewInformationalRadioButton.AutoSize = true;
            this.m_ViewInformationalRadioButton.Location = new System.Drawing.Point(16, 73);
            this.m_ViewInformationalRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewInformationalRadioButton.Name = "m_ViewInformationalRadioButton";
            this.m_ViewInformationalRadioButton.Size = new System.Drawing.Size(97, 19);
            this.m_ViewInformationalRadioButton.TabIndex = 2;
            this.m_ViewInformationalRadioButton.TabStop = true;
            this.m_ViewInformationalRadioButton.Text = "Informational";
            this.m_ViewInformationalRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewInformationalRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewInformationalRadioButton_CheckedChanged);
            // 
            // m_ViewWarningsRadioButton
            // 
            this.m_ViewWarningsRadioButton.AutoSize = true;
            this.m_ViewWarningsRadioButton.Location = new System.Drawing.Point(16, 48);
            this.m_ViewWarningsRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewWarningsRadioButton.Name = "m_ViewWarningsRadioButton";
            this.m_ViewWarningsRadioButton.Size = new System.Drawing.Size(70, 19);
            this.m_ViewWarningsRadioButton.TabIndex = 1;
            this.m_ViewWarningsRadioButton.TabStop = true;
            this.m_ViewWarningsRadioButton.Text = "Warning";
            this.m_ViewWarningsRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewWarningsRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewWarningsRadioButton_CheckedChanged);
            // 
            // m_ViewErrorsRadioButton
            // 
            this.m_ViewErrorsRadioButton.AutoSize = true;
            this.m_ViewErrorsRadioButton.Location = new System.Drawing.Point(16, 23);
            this.m_ViewErrorsRadioButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ViewErrorsRadioButton.Name = "m_ViewErrorsRadioButton";
            this.m_ViewErrorsRadioButton.Size = new System.Drawing.Size(55, 19);
            this.m_ViewErrorsRadioButton.TabIndex = 0;
            this.m_ViewErrorsRadioButton.TabStop = true;
            this.m_ViewErrorsRadioButton.Text = "Errors";
            this.m_ViewErrorsRadioButton.UseVisualStyleBackColor = true;
            this.m_ViewErrorsRadioButton.CheckedChanged += new System.EventHandler(this.m_ViewErrorsRadioButton_CheckedChanged);
            // 
            // m_BackButton
            // 
            this.m_BackButton.Font = new System.Drawing.Font("Calibri", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_BackButton.Location = new System.Drawing.Point(8, 8);
            this.m_BackButton.Name = "m_BackButton";
            this.m_BackButton.Size = new System.Drawing.Size(75, 24);
            this.m_BackButton.TabIndex = 4;
            this.m_BackButton.Text = "<<";
            this.m_BackButton.UseVisualStyleBackColor = true;
            this.m_BackButton.Click += new System.EventHandler(this.m_BackButton_Click);
            this.m_BackButton.MouseEnter += new System.EventHandler(this.m_BackButton_MouseEnter);
            this.m_BackButton.MouseLeave += new System.EventHandler(this.Generic_MouseLeave);
            // 
            // m_CurrentBranchNodeTextBox
            // 
            this.m_CurrentBranchNodeTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_CurrentBranchNodeTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_CurrentBranchNodeTextBox.Cursor = System.Windows.Forms.Cursors.Default;
            this.m_CurrentBranchNodeTextBox.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_CurrentBranchNodeTextBox.Location = new System.Drawing.Point(87, 9);
            this.m_CurrentBranchNodeTextBox.Name = "m_CurrentBranchNodeTextBox";
            this.m_CurrentBranchNodeTextBox.ReadOnly = true;
            this.m_CurrentBranchNodeTextBox.Size = new System.Drawing.Size(846, 23);
            this.m_CurrentBranchNodeTextBox.TabIndex = 5;
            this.m_CurrentBranchNodeTextBox.TabStop = false;
            // 
            // m_AuditEventsListView
            // 
            this.m_AuditEventsListView.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.m_AuditEventsListView.BackColor = System.Drawing.SystemColors.Info;
            this.m_AuditEventsListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.m_SequenceNumberColumnHeader,
            this.m_EpochDateColumnHeader,
            this.m_EpochTimeColumnHeader,
            this.m_EventTypeColumnHeader});
            this.m_AuditEventsListView.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_AuditEventsListView.FullRowSelect = true;
            this.m_AuditEventsListView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.m_AuditEventsListView.HideSelection = false;
            this.m_AuditEventsListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem1});
            this.m_AuditEventsListView.LabelWrap = false;
            this.m_AuditEventsListView.Location = new System.Drawing.Point(6, 38);
            this.m_AuditEventsListView.MultiSelect = false;
            this.m_AuditEventsListView.Name = "m_AuditEventsListView";
            this.m_AuditEventsListView.ShowGroups = false;
            this.m_AuditEventsListView.Size = new System.Drawing.Size(635, 212);
            this.m_AuditEventsListView.TabIndex = 5;
            this.m_AuditEventsListView.UseCompatibleStateImageBehavior = false;
            this.m_AuditEventsListView.View = System.Windows.Forms.View.Details;
            this.m_AuditEventsListView.SelectedIndexChanged += new System.EventHandler(this.m_AuditEventsListView_SelectedIndexChanged);
            this.m_AuditEventsListView.DoubleClick += new System.EventHandler(this.m_AuditEventsListView_DoubleClick);
            this.m_AuditEventsListView.MouseEnter += new System.EventHandler(this.m_AuditEventsListView_MouseEnter);
            this.m_AuditEventsListView.MouseLeave += new System.EventHandler(this.Generic_MouseLeave);
            // 
            // m_SequenceNumberColumnHeader
            // 
            this.m_SequenceNumberColumnHeader.Text = "Sequence #";
            this.m_SequenceNumberColumnHeader.Width = 84;
            // 
            // m_EpochDateColumnHeader
            // 
            this.m_EpochDateColumnHeader.Text = "Date";
            this.m_EpochDateColumnHeader.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.m_EpochDateColumnHeader.Width = 155;
            // 
            // m_EpochTimeColumnHeader
            // 
            this.m_EpochTimeColumnHeader.Text = "Time (UTC)";
            this.m_EpochTimeColumnHeader.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.m_EpochTimeColumnHeader.Width = 105;
            // 
            // m_EventTypeColumnHeader
            // 
            this.m_EventTypeColumnHeader.Text = "Type";
            this.m_EventTypeColumnHeader.Width = 265;
            // 
            // m_AuditEventPropertiesTextBox
            // 
            this.m_AuditEventPropertiesTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_AuditEventPropertiesTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_AuditEventPropertiesTextBox.ContextMenuStrip = this.m_TreeViewContextMenu;
            this.m_AuditEventPropertiesTextBox.Cursor = System.Windows.Forms.Cursors.Default;
            this.m_AuditEventPropertiesTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_AuditEventPropertiesTextBox.Location = new System.Drawing.Point(647, 38);
            this.m_AuditEventPropertiesTextBox.Multiline = true;
            this.m_AuditEventPropertiesTextBox.Name = "m_AuditEventPropertiesTextBox";
            this.m_AuditEventPropertiesTextBox.ReadOnly = true;
            this.m_AuditEventPropertiesTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.m_AuditEventPropertiesTextBox.Size = new System.Drawing.Size(286, 212);
            this.m_AuditEventPropertiesTextBox.TabIndex = 7;
            this.m_AuditEventPropertiesTextBox.TabStop = false;
            this.m_AuditEventPropertiesTextBox.Text = resources.GetString("m_AuditEventPropertiesTextBox.Text");
            this.m_AuditEventPropertiesTextBox.WordWrap = false;
            this.m_AuditEventPropertiesTextBox.MouseEnter += new System.EventHandler(this.m_AuditEventPropertiesTextBox_MouseEnter);
            this.m_AuditEventPropertiesTextBox.MouseLeave += new System.EventHandler(this.Generic_MouseLeave);
            // 
            // m_TreeViewContextMenu
            // 
            this.m_TreeViewContextMenu.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_TreeViewContextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_CopyCurrentAuditEventPropertiesToolStripMenuItem,
            this.m_ViewAdditionalDataDetailsToolStripMenuItem});
            this.m_TreeViewContextMenu.Name = "m_TreeViewContextMenu";
            this.m_TreeViewContextMenu.Size = new System.Drawing.Size(223, 48);
            // 
            // m_CopyCurrentAuditEventPropertiesToolStripMenuItem
            // 
            this.m_CopyCurrentAuditEventPropertiesToolStripMenuItem.Name = "m_CopyCurrentAuditEventPropertiesToolStripMenuItem";
            this.m_CopyCurrentAuditEventPropertiesToolStripMenuItem.Size = new System.Drawing.Size(222, 22);
            this.m_CopyCurrentAuditEventPropertiesToolStripMenuItem.Text = "Copy Properties";
            this.m_CopyCurrentAuditEventPropertiesToolStripMenuItem.Click += new System.EventHandler(this.m_CopyCurrentAuditEventPropertiesToolStripMenuItem_Click);
            // 
            // m_ViewAdditionalDataDetailsToolStripMenuItem
            // 
            this.m_ViewAdditionalDataDetailsToolStripMenuItem.Name = "m_ViewAdditionalDataDetailsToolStripMenuItem";
            this.m_ViewAdditionalDataDetailsToolStripMenuItem.Size = new System.Drawing.Size(222, 22);
            this.m_ViewAdditionalDataDetailsToolStripMenuItem.Text = "View Additional Data Details";
            this.m_ViewAdditionalDataDetailsToolStripMenuItem.Click += new System.EventHandler(this.m_ViewAdditionalDataDetailsToolStripMenuItem_Click);
            // 
            // m_DateTimeFiltersGroupBox
            // 
            this.m_DateTimeFiltersGroupBox.Controls.Add(this.m_ResetLatestDateTimePickerButton);
            this.m_DateTimeFiltersGroupBox.Controls.Add(this.m_ResetEarliestDateTimePickerButton);
            this.m_DateTimeFiltersGroupBox.Controls.Add(this.m_LatestDateTimePickerLabel);
            this.m_DateTimeFiltersGroupBox.Controls.Add(this.m_StartTimeLabel);
            this.m_DateTimeFiltersGroupBox.Controls.Add(this.m_LatestDateTimePicker);
            this.m_DateTimeFiltersGroupBox.Controls.Add(this.m_EarliestDateTimePicker);
            this.m_DateTimeFiltersGroupBox.Location = new System.Drawing.Point(600, 8);
            this.m_DateTimeFiltersGroupBox.Margin = new System.Windows.Forms.Padding(4);
            this.m_DateTimeFiltersGroupBox.Name = "m_DateTimeFiltersGroupBox";
            this.m_DateTimeFiltersGroupBox.Padding = new System.Windows.Forms.Padding(4);
            this.m_DateTimeFiltersGroupBox.Size = new System.Drawing.Size(330, 133);
            this.m_DateTimeFiltersGroupBox.TabIndex = 11;
            this.m_DateTimeFiltersGroupBox.TabStop = false;
            this.m_DateTimeFiltersGroupBox.Text = "Data/Time Filtering";
            // 
            // m_ResetLatestDateTimePickerButton
            // 
            this.m_ResetLatestDateTimePickerButton.Location = new System.Drawing.Point(200, 87);
            this.m_ResetLatestDateTimePickerButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ResetLatestDateTimePickerButton.Name = "m_ResetLatestDateTimePickerButton";
            this.m_ResetLatestDateTimePickerButton.Size = new System.Drawing.Size(112, 23);
            this.m_ResetLatestDateTimePickerButton.TabIndex = 5;
            this.m_ResetLatestDateTimePickerButton.Text = "Reset";
            this.m_ResetLatestDateTimePickerButton.UseVisualStyleBackColor = true;
            this.m_ResetLatestDateTimePickerButton.Click += new System.EventHandler(this.m_ResetLatestDateTimePickerButton_Click);
            // 
            // m_ResetEarliestDateTimePickerButton
            // 
            this.m_ResetEarliestDateTimePickerButton.Location = new System.Drawing.Point(200, 41);
            this.m_ResetEarliestDateTimePickerButton.Margin = new System.Windows.Forms.Padding(4);
            this.m_ResetEarliestDateTimePickerButton.Name = "m_ResetEarliestDateTimePickerButton";
            this.m_ResetEarliestDateTimePickerButton.Size = new System.Drawing.Size(112, 23);
            this.m_ResetEarliestDateTimePickerButton.TabIndex = 4;
            this.m_ResetEarliestDateTimePickerButton.Text = "Reset";
            this.m_ResetEarliestDateTimePickerButton.UseVisualStyleBackColor = true;
            this.m_ResetEarliestDateTimePickerButton.Click += new System.EventHandler(this.m_ResetEarlestDateTimePickerButton_Click);
            // 
            // m_LatestDateTimePickerLabel
            // 
            this.m_LatestDateTimePickerLabel.AutoSize = true;
            this.m_LatestDateTimePickerLabel.Location = new System.Drawing.Point(21, 68);
            this.m_LatestDateTimePickerLabel.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.m_LatestDateTimePickerLabel.Name = "m_LatestDateTimePickerLabel";
            this.m_LatestDateTimePickerLabel.Size = new System.Drawing.Size(38, 15);
            this.m_LatestDateTimePickerLabel.TabIndex = 3;
            this.m_LatestDateTimePickerLabel.Text = "Latest";
            // 
            // m_StartTimeLabel
            // 
            this.m_StartTimeLabel.AutoSize = true;
            this.m_StartTimeLabel.Location = new System.Drawing.Point(21, 23);
            this.m_StartTimeLabel.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.m_StartTimeLabel.Name = "m_StartTimeLabel";
            this.m_StartTimeLabel.Size = new System.Drawing.Size(44, 15);
            this.m_StartTimeLabel.TabIndex = 2;
            this.m_StartTimeLabel.Text = "Earliest";
            // 
            // m_LatestDateTimePicker
            // 
            this.m_LatestDateTimePicker.CustomFormat = " yyyy/mm/dd @ HH:mm UTC";
            this.m_LatestDateTimePicker.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.m_LatestDateTimePicker.Location = new System.Drawing.Point(21, 85);
            this.m_LatestDateTimePicker.Margin = new System.Windows.Forms.Padding(4);
            this.m_LatestDateTimePicker.Name = "m_LatestDateTimePicker";
            this.m_LatestDateTimePicker.Size = new System.Drawing.Size(171, 23);
            this.m_LatestDateTimePicker.TabIndex = 1;
            this.m_LatestDateTimePicker.ValueChanged += new System.EventHandler(this.m_LatestDateTimePicker_ValueChanged);
            // 
            // m_EarliestDateTimePicker
            // 
            this.m_EarliestDateTimePicker.Checked = false;
            this.m_EarliestDateTimePicker.CustomFormat = " yyyy/mm/dd @ HH:mm UTC";
            this.m_EarliestDateTimePicker.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.m_EarliestDateTimePicker.Location = new System.Drawing.Point(21, 41);
            this.m_EarliestDateTimePicker.Margin = new System.Windows.Forms.Padding(4);
            this.m_EarliestDateTimePicker.Name = "m_EarliestDateTimePicker";
            this.m_EarliestDateTimePicker.Size = new System.Drawing.Size(171, 23);
            this.m_EarliestDateTimePicker.TabIndex = 0;
            this.m_EarliestDateTimePicker.ValueChanged += new System.EventHandler(this.m_EarliestDateTimePicker_ValueChanged);
            // 
            // m_AuditEventViewTabControl
            // 
            this.m_AuditEventViewTabControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_AuditEventViewTabControl.Controls.Add(this.m_TreeViewTabPage);
            this.m_AuditEventViewTabControl.Controls.Add(this.m_FlatViewTabPage);
            this.m_AuditEventViewTabControl.Location = new System.Drawing.Point(12, 12);
            this.m_AuditEventViewTabControl.Name = "m_AuditEventViewTabControl";
            this.m_AuditEventViewTabControl.SelectedIndex = 0;
            this.m_AuditEventViewTabControl.Size = new System.Drawing.Size(947, 284);
            this.m_AuditEventViewTabControl.TabIndex = 12;
            this.m_AuditEventViewTabControl.SelectedIndexChanged += new System.EventHandler(this.m_AuditEventViewTabControl_SelectedIndexChanged);
            // 
            // m_TreeViewTabPage
            // 
            this.m_TreeViewTabPage.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.m_TreeViewTabPage.Controls.Add(this.m_CurrentBranchNodeTextBox);
            this.m_TreeViewTabPage.Controls.Add(this.m_BackButton);
            this.m_TreeViewTabPage.Controls.Add(this.m_AuditEventsListView);
            this.m_TreeViewTabPage.Controls.Add(this.m_AuditEventPropertiesTextBox);
            this.m_TreeViewTabPage.Location = new System.Drawing.Point(4, 24);
            this.m_TreeViewTabPage.Name = "m_TreeViewTabPage";
            this.m_TreeViewTabPage.Padding = new System.Windows.Forms.Padding(3);
            this.m_TreeViewTabPage.Size = new System.Drawing.Size(939, 256);
            this.m_TreeViewTabPage.TabIndex = 0;
            this.m_TreeViewTabPage.Text = "Tree View Browsing";
            // 
            // m_FlatViewTabPage
            // 
            this.m_FlatViewTabPage.Controls.Add(this.m_FlatViewPropertiesTextBox);
            this.m_FlatViewTabPage.Controls.Add(this.m_FlatViewListView);
            this.m_FlatViewTabPage.Controls.Add(this.m_EventTypeFiltersGroupBox);
            this.m_FlatViewTabPage.Controls.Add(this.m_DateTimeFiltersGroupBox);
            this.m_FlatViewTabPage.Location = new System.Drawing.Point(4, 24);
            this.m_FlatViewTabPage.Name = "m_FlatViewTabPage";
            this.m_FlatViewTabPage.Padding = new System.Windows.Forms.Padding(3);
            this.m_FlatViewTabPage.Size = new System.Drawing.Size(939, 256);
            this.m_FlatViewTabPage.TabIndex = 1;
            this.m_FlatViewTabPage.Text = "Flat View Filtered Browsing";
            this.m_FlatViewTabPage.UseVisualStyleBackColor = true;
            // 
            // m_FlatViewPropertiesTextBox
            // 
            this.m_FlatViewPropertiesTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_FlatViewPropertiesTextBox.BackColor = System.Drawing.SystemColors.Info;
            this.m_FlatViewPropertiesTextBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_FlatViewPropertiesTextBox.Location = new System.Drawing.Point(503, 149);
            this.m_FlatViewPropertiesTextBox.Margin = new System.Windows.Forms.Padding(4);
            this.m_FlatViewPropertiesTextBox.MaxLength = 65535;
            this.m_FlatViewPropertiesTextBox.Multiline = true;
            this.m_FlatViewPropertiesTextBox.Name = "m_FlatViewPropertiesTextBox";
            this.m_FlatViewPropertiesTextBox.ReadOnly = true;
            this.m_FlatViewPropertiesTextBox.Size = new System.Drawing.Size(427, 100);
            this.m_FlatViewPropertiesTextBox.TabIndex = 13;
            this.m_FlatViewPropertiesTextBox.WordWrap = false;
            // 
            // m_FlatViewListView
            // 
            this.m_FlatViewListView.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.m_FlatViewListView.BackColor = System.Drawing.SystemColors.Info;
            this.m_FlatViewListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.m_DateTimeInFlatListViewColumnHeader,
            this.m_EventNameInFlatListViewColumnHeader});
            this.m_FlatViewListView.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_FlatViewListView.FullRowSelect = true;
            this.m_FlatViewListView.HideSelection = false;
            this.m_FlatViewListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem2});
            this.m_FlatViewListView.Location = new System.Drawing.Point(9, 148);
            this.m_FlatViewListView.Margin = new System.Windows.Forms.Padding(4);
            this.m_FlatViewListView.MultiSelect = false;
            this.m_FlatViewListView.Name = "m_FlatViewListView";
            this.m_FlatViewListView.Size = new System.Drawing.Size(486, 103);
            this.m_FlatViewListView.Sorting = System.Windows.Forms.SortOrder.Descending;
            this.m_FlatViewListView.TabIndex = 12;
            this.m_FlatViewListView.UseCompatibleStateImageBehavior = false;
            this.m_FlatViewListView.View = System.Windows.Forms.View.Details;
            this.m_FlatViewListView.SelectedIndexChanged += new System.EventHandler(this.m_FlatViewListView_SelectedIndexChanged);
            this.m_FlatViewListView.DoubleClick += new System.EventHandler(this.m_FlatViewListView_DoubleClick);
            // 
            // m_DateTimeInFlatListViewColumnHeader
            // 
            this.m_DateTimeInFlatListViewColumnHeader.Text = "Date and Time (GMT)";
            this.m_DateTimeInFlatListViewColumnHeader.Width = 238;
            // 
            // m_EventNameInFlatListViewColumnHeader
            // 
            this.m_EventNameInFlatListViewColumnHeader.Text = "Type";
            this.m_EventNameInFlatListViewColumnHeader.Width = 221;
            // 
            // m_StatusStrip
            // 
            this.m_StatusStrip.Font = new System.Drawing.Font("Consolas", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_StatusStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.m_StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_TimeStripStatusLabel,
            this.m_MessageStripStatusLabel,
            this.m_ErrorStripStatusLabel});
            this.m_StatusStrip.Location = new System.Drawing.Point(0, 332);
            this.m_StatusStrip.Name = "m_StatusStrip";
            this.m_StatusStrip.Padding = new System.Windows.Forms.Padding(2, 0, 14, 0);
            this.m_StatusStrip.Size = new System.Drawing.Size(971, 22);
            this.m_StatusStrip.TabIndex = 13;
            // 
            // m_TimeStripStatusLabel
            // 
            this.m_TimeStripStatusLabel.AutoSize = false;
            this.m_TimeStripStatusLabel.ForeColor = System.Drawing.Color.Green;
            this.m_TimeStripStatusLabel.Name = "m_TimeStripStatusLabel";
            this.m_TimeStripStatusLabel.Size = new System.Drawing.Size(300, 17);
            this.m_TimeStripStatusLabel.Text = "September 22nd, 2021 @ 22:22:22 GMT";
            this.m_TimeStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_MessageStripStatusLabel
            // 
            this.m_MessageStripStatusLabel.AutoSize = false;
            this.m_MessageStripStatusLabel.ForeColor = System.Drawing.SystemColors.HotTrack;
            this.m_MessageStripStatusLabel.Name = "m_MessageStripStatusLabel";
            this.m_MessageStripStatusLabel.Size = new System.Drawing.Size(450, 17);
            this.m_MessageStripStatusLabel.Text = "Right click to select context menu!";
            this.m_MessageStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // m_ErrorStripStatusLabel
            // 
            this.m_ErrorStripStatusLabel.AutoSize = false;
            this.m_ErrorStripStatusLabel.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_ErrorStripStatusLabel.ForeColor = System.Drawing.Color.Red;
            this.m_ErrorStripStatusLabel.IsLink = true;
            this.m_ErrorStripStatusLabel.LinkColor = System.Drawing.Color.Red;
            this.m_ErrorStripStatusLabel.Name = "m_ErrorStripStatusLabel";
            this.m_ErrorStripStatusLabel.Size = new System.Drawing.Size(200, 17);
            this.m_ErrorStripStatusLabel.Text = "Click to view Exceptions!";
            this.m_ErrorStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.m_ErrorStripStatusLabel.Click += new System.EventHandler(this.m_ErrorStripStatusLabel_Click);
            // 
            // m_StatusBarRefreshTimer
            // 
            this.m_StatusBarRefreshTimer.Enabled = true;
            this.m_StatusBarRefreshTimer.Tick += new System.EventHandler(this.m_StatusBarRefreshTimer_Tick);
            // 
            // AuditEventsViewer
            // 
            this.AcceptButton = this.m_RefreshButton;
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.BackColor = System.Drawing.SystemColors.Control;
            this.CancelButton = this.m_ExitButton;
            this.ClientSize = new System.Drawing.Size(971, 354);
            this.Controls.Add(this.m_StatusStrip);
            this.Controls.Add(this.m_AuditEventViewTabControl);
            this.Controls.Add(this.m_ExitButton);
            this.Controls.Add(this.m_RefreshButton);
            this.DoubleBuffered = true;
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.MinimumSize = new System.Drawing.Size(987, 393);
            this.Name = "AuditEventsViewer";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "SAIL Audit Viewer";
            this.Load += new System.EventHandler(this.AuditEventsViewer_Load);
            this.m_EventTypeFiltersGroupBox.ResumeLayout(false);
            this.m_EventTypeFiltersGroupBox.PerformLayout();
            this.m_TreeViewContextMenu.ResumeLayout(false);
            this.m_DateTimeFiltersGroupBox.ResumeLayout(false);
            this.m_DateTimeFiltersGroupBox.PerformLayout();
            this.m_AuditEventViewTabControl.ResumeLayout(false);
            this.m_TreeViewTabPage.ResumeLayout(false);
            this.m_TreeViewTabPage.PerformLayout();
            this.m_FlatViewTabPage.ResumeLayout(false);
            this.m_FlatViewTabPage.PerformLayout();
            this.m_StatusStrip.ResumeLayout(false);
            this.m_StatusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button m_RefreshButton;
        private System.Windows.Forms.Button m_ExitButton;
        private System.Windows.Forms.GroupBox m_EventTypeFiltersGroupBox;
        private System.Windows.Forms.Button m_BackButton;
        private System.Windows.Forms.TextBox m_CurrentBranchNodeTextBox;
        private System.Windows.Forms.ListView m_AuditEventsListView;
        private System.Windows.Forms.ColumnHeader m_SequenceNumberColumnHeader;
        private System.Windows.Forms.ColumnHeader m_EpochDateColumnHeader;
        private System.Windows.Forms.ColumnHeader m_EpochTimeColumnHeader;
        private System.Windows.Forms.ColumnHeader m_EventTypeColumnHeader;
        private System.Windows.Forms.TextBox m_AuditEventPropertiesTextBox;
        private System.Windows.Forms.GroupBox m_DateTimeFiltersGroupBox;
        private System.Windows.Forms.Button m_ResetLatestDateTimePickerButton;
        private System.Windows.Forms.Button m_ResetEarliestDateTimePickerButton;
        private System.Windows.Forms.Label m_LatestDateTimePickerLabel;
        private System.Windows.Forms.Label m_StartTimeLabel;
        private System.Windows.Forms.DateTimePicker m_LatestDateTimePicker;
        private System.Windows.Forms.DateTimePicker m_EarliestDateTimePicker;
        private System.Windows.Forms.TabControl m_AuditEventViewTabControl;
        private System.Windows.Forms.TabPage m_TreeViewTabPage;
        private System.Windows.Forms.TabPage m_FlatViewTabPage;
        private System.Windows.Forms.ListView m_FlatViewListView;
        private System.Windows.Forms.RadioButton m_ViewJobEngineActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewAllActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewPullDataActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewPushDataActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewPushFnActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewExecFnActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewDataConnectorActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewVmActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewDatasetActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewDigitalContractActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewAuditLogAccessesActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewUserOrganizationActivityRadioButton;
        private System.Windows.Forms.RadioButton m_ViewAccessesRadioButton;
        private System.Windows.Forms.RadioButton m_ViewInformationalRadioButton;
        private System.Windows.Forms.RadioButton m_ViewWarningsRadioButton;
        private System.Windows.Forms.RadioButton m_ViewErrorsRadioButton;
        private System.Windows.Forms.ContextMenuStrip m_TreeViewContextMenu;
        private System.Windows.Forms.ToolStripMenuItem m_CopyCurrentAuditEventPropertiesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem m_ViewAdditionalDataDetailsToolStripMenuItem;
        private System.Windows.Forms.ColumnHeader m_DateTimeInFlatListViewColumnHeader;
        private System.Windows.Forms.ColumnHeader m_EventNameInFlatListViewColumnHeader;
        private System.Windows.Forms.StatusStrip m_StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel m_TimeStripStatusLabel;
        private System.Windows.Forms.ToolStripStatusLabel m_MessageStripStatusLabel;
        private System.Windows.Forms.ToolStripStatusLabel m_ErrorStripStatusLabel;
        private System.Windows.Forms.Timer m_StatusBarRefreshTimer;
        private System.Windows.Forms.TextBox m_FlatViewPropertiesTextBox;
    }
}