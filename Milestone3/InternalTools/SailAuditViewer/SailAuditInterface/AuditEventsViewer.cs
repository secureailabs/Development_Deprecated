using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SailAuditInterface
{
    public partial class AuditEventsViewer : Form
    {
        public AuditEventsViewer()
        {
            InitializeComponent();
            // Initialize the ExceptionsDialog
            m_ExceptionsDialog = new ExceptionsDialog();
            // Make sure we start up with all of the audit events loaded up
            ApiInterop.DesignateAuditEventsUsingParentIdentifier("{00000000-0000-0000-0000-000000000000}");
            m_EarliestDateTimePicker.Value = DateTime.Parse("01/01/1970");
            m_LatestDateTimePicker.Value = DateTime.Now;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void AuditEventsViewer_Load(
            object sender,
            EventArgs e
            )
        {
            
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_BranchesTreeView_AfterSelect(
            object sender,
            TreeViewEventArgs e
            )
        {
            if (1 == m_AuditEventsListView.SelectedItems.Count)
            {
                uint sequenceNumber = uint.Parse(m_AuditEventsListView.SelectedItems[0].Text);
                string auditEventIdentifier = ApiInterop.GetDesignatedAuditEventIdentifierBySequenceNumber(sequenceNumber);
                string textToPrint = ApiInterop.GetAuditEventGeneralDetails(auditEventIdentifier);
                m_AuditEventPropertiesTextBox.Text = textToPrint.Replace(",", ",\r\n");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_RefreshButton_Click(
            object sender,
            EventArgs e
            )
        {
            // Do the first initial audit events load
            ReloadAllAuditEventsDialog reloadAllAuditEventsDialog = new ReloadAllAuditEventsDialog();
            reloadAllAuditEventsDialog.ShowDialog();
            if (m_AuditEventViewTabControl.SelectedTab == m_TreeViewTabPage)
            {
                this.RedrawDesignatedNodes();
            }
            else
            {
                this.RedrawFlatView();
            }            
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_CloseButton_Click(
            object sender,
            EventArgs e
            )
        {
            this.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        private void ReloadDesignatedNodes(
            string parentIdentifier
            )
        {
            ApiInterop.DesignateAuditEventsUsingParentIdentifier(parentIdentifier);
            // Because we probably have a whole new set of audit events, let's make sure
            // the m_AuditEventsListView selestion is reset
            m_AuditEventsListView.SelectedItems.Clear();
            // Now redraw the nodes
            this.RedrawDesignatedNodes();
        }

        /// <summary>
        /// 
        /// </summary>
        private void RedrawFlatView()
        {
            // Redraw the ListView
            m_FlatViewListView.BeginUpdate();
            // Clear the items from the m_AuditEventsListView
            m_FlatViewListView.Items.Clear();
            m_FlatViewPropertiesTextBox.Text = "";
            // Loop through all of the events
            uint numberOfAuditEvents = ApiInterop.GetAllLoadedAuditEventsCount();
            for (uint index = 0; index < numberOfAuditEvents; ++index)
            {
                // Get the audit event identifier
                string auditEventIdentifier = ApiInterop.GetAuditEventIdentifierByIndex(index);
                // Get and format the millisecond time and date string. This is formatted such
                // that the list view can sort on that value
                if (false == this.IsAuditEventFilteredOut(auditEventIdentifier))
                {
                    string[] listViewItems = new string[3];
                    listViewItems[0] = ApiInterop.GetAuditEventEpochTimestampInMillisecondsGmt(auditEventIdentifier);
                    listViewItems[1] = ApiInterop.GetAuditEventType(auditEventIdentifier);
                    listViewItems[2] = "No details";

                    ListViewItem listViewItem = new ListViewItem(listViewItems);
                    listViewItem.Tag = auditEventIdentifier;
                    m_FlatViewListView.Items.Add(listViewItem);
                }
                m_FlatViewListView.EndUpdate();
            }
        }

        /// <summary>
        /// 
        /// </summary>
        private void RedrawDesignatedNodes()
        {
            int selectedSequenceNumber = 0;
            // Tract the sequence of the event currently selected
            if (1 == m_AuditEventsListView.SelectedItems.Count)
            {
                selectedSequenceNumber = int.Parse(m_AuditEventsListView.SelectedItems[0].Text);
            }
            // Redraw the ListView
            m_AuditEventsListView.BeginUpdate();
            // Clear the items from the m_AuditEventsListView
            m_AuditEventsListView.Items.Clear();
            // Now delete any existing nodes in the tree view.
            for (uint unIndex = 0; unIndex < ApiInterop.GetDesignatedAuditEventsCount(); ++unIndex)
            {
                string eventIdentifier = ApiInterop.GetDesignatedAuditEventIdentifierBySequenceNumber(unIndex);
                string[] listViewItems = new string[4];

                listViewItems[0] = unIndex.ToString();
                listViewItems[1] = ApiInterop.GetAuditEventDateGmt(eventIdentifier);
                listViewItems[2] = ApiInterop.GetAuditEventTimeWithMillisecondsGmt(eventIdentifier);
                listViewItems[3] = ApiInterop.GetAuditEventType(eventIdentifier);

                ListViewItem listViewItem = new ListViewItem(listViewItems);
                m_AuditEventsListView.Items.Add(listViewItem);
            }
            m_AuditEventsListView.EndUpdate();
            // Now we redraw the branch node path
            string currentAuditEventsParentIdentifier = ApiInterop.GetDesignatedAuditEventsParentIdentifier();
            if ("{00000000-0000-0000-0000-000000000000}" == currentAuditEventsParentIdentifier)
            {
                m_CurrentBranchNodeTextBox.Text = "/";
            }
            else
            {
                string currentParentEventIdentifier = currentAuditEventsParentIdentifier;
                Stack<string> nodeStack = new Stack<string>();
                nodeStack.Push(currentParentEventIdentifier);
                do
                {
                    currentParentEventIdentifier = ApiInterop.GetAuditEventParentIdentifier(currentParentEventIdentifier);
                    if ("{00000000-0000-0000-0000-000000000000}" != currentParentEventIdentifier)
                    {
                        nodeStack.Push(currentParentEventIdentifier);
                    }
                }
                while (("{00000000-0000-0000-0000-000000000000}" != currentParentEventIdentifier) && ("" != currentParentEventIdentifier));

                string newBranchNode = "";
                do
                {
                    string poppedIdentifier = nodeStack.Pop();
                    newBranchNode += "/";
                    newBranchNode += poppedIdentifier;
                }
                while (0 < nodeStack.Count);

                m_CurrentBranchNodeTextBox.Text = newBranchNode;
            }
            // Now we redraw the properties text box
            if (0 != m_AuditEventsListView.Items.Count)
            {
                m_AuditEventsListView.SelectedIndices.Add(selectedSequenceNumber);
                string auditEventIdentifier = ApiInterop.GetDesignatedAuditEventIdentifierBySequenceNumber((uint)selectedSequenceNumber);
                string textToCopy = ApiInterop.GetAuditEventGeneralDetails(auditEventIdentifier);
                textToCopy = textToCopy.Remove(0, 1);
                textToCopy = textToCopy.Remove(textToCopy.Length - 1, 1);
                m_AuditEventPropertiesTextBox.Text = textToCopy.Replace(",", ",\r\n");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="eventIdentifier"></param>
        /// <returns></returns>
        private bool IsAuditEventFilteredOut(
            string eventIdentifier
            )
        {
            bool isFiltered = false;

            if (false == m_ViewAllActivityRadioButton.Checked)
            {
                string auditEventType = ApiInterop.GetAuditEventType(eventIdentifier);
                isFiltered = true;
                if (true == m_ViewVmActivityRadioButton.Checked)
                {
                    if (("VM_STARTED" == auditEventType) || ("VM_INITIALIZED" == auditEventType) || ("VM_READY" == auditEventType) || ("VM_SHUTDOWN" == auditEventType))
                    {
                        isFiltered = false;
                    }
                }
                else if ((true == m_ViewDatasetActivityRadioButton.Checked) && ("VM_DATASET_UPLOADED" == auditEventType))
                {
                    isFiltered = false;
                }
                else if (true == m_ViewAccessesRadioButton.Checked)
                {
                    if (("CONNECT_SUCCESS" == auditEventType) || ("CONNECT_FAILURE" == auditEventType))
                    {
                        isFiltered = false;
                    }
                }
                else if ((true == m_ViewPullDataActivityRadioButton.Checked) && ("PULL_DATA" == auditEventType))
                {
                    isFiltered = false;
                }
                else if ((true == m_ViewPushDataActivityRadioButton.Checked) && ("PUSH_DATA" == auditEventType))
                {
                    isFiltered = false;
                }
                else if ((true == m_ViewPushFnActivityRadioButton.Checked) && ("PUSH_FN" == auditEventType))
                {
                    isFiltered = false;
                }
                else if ((true == m_ViewExecFnActivityRadioButton.Checked) && ("RUN_FN" == auditEventType))
                {
                    isFiltered = false;
                }
                else if ((true == m_ViewJobEngineActivityRadioButton.Checked) && ("CHECK_JOB" == auditEventType))
                {
                    isFiltered = false;
                }
            }

            return isFiltered;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_EarliestDateTimePicker_ValueChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawDesignatedNodes();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_LatestDateTimePicker_ValueChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawDesignatedNodes();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ResetEarlestDateTimePickerButton_Click(
            object sender,
            EventArgs e
            )
        {
            m_EarliestDateTimePicker.Value = DateTime.Parse("01/01/1970");
            this.RedrawDesignatedNodes();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ResetLatestDateTimePickerButton_Click(
            object sender,
            EventArgs e
            )
        {
            m_LatestDateTimePicker.Value = DateTime.Now;
            this.RedrawDesignatedNodes();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_BackButton_Click(
            object sender,
            EventArgs e
            )
        {
            // Make sure that the current selection within the listview is clear since
            // we are switching list views
            m_AuditEventsListView.SelectedIndices.Clear();
            // Get the current audit events parent identifier
            string currentAuditEventsParentIdentifier = ApiInterop.GetDesignatedAuditEventsParentIdentifier();
            if ("{00000000-0000-0000-0000-000000000000}" == currentAuditEventsParentIdentifier)
            {
                m_CurrentBranchNodeTextBox.Text = "/";
            }
            else
            {
                string newParentEventIdentifier = ApiInterop.GetAuditEventParentIdentifier(currentAuditEventsParentIdentifier);
                if ("{00000000-0000-0000-0000-000000000000}" == newParentEventIdentifier)
                {
                    m_CurrentBranchNodeTextBox.Text = "/";
                }
                else if ("" == newParentEventIdentifier)
                {
                    m_CurrentBranchNodeTextBox.Text = "/";
                }
                else
                {
                    string currentParentEventIdentifier = newParentEventIdentifier;
                    Stack<string> nodeStack = new Stack<string>();
                    nodeStack.Push(currentParentEventIdentifier);
                    do
                    {
                        currentParentEventIdentifier = ApiInterop.GetAuditEventParentIdentifier(currentParentEventIdentifier);
                        if (("{00000000-0000-0000-0000-000000000000}" != currentParentEventIdentifier) && ("" != currentParentEventIdentifier))
                        {
                            nodeStack.Push(currentParentEventIdentifier);
                        }
                    }
                    while (("{00000000-0000-0000-0000-000000000000}" != currentParentEventIdentifier) && ("" != currentParentEventIdentifier));

                    string newBranchNode = "";
                    do
                    {
                        string poppedIdentifier = nodeStack.Pop();
                        newBranchNode += "/";
                        newBranchNode += poppedIdentifier;
                    }
                    while (0 < nodeStack.Count);
                }
                this.ReloadDesignatedNodes(newParentEventIdentifier);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_CopyAllButton_Click(
            object sender,
            EventArgs e
            )
        {
            if (1 == m_AuditEventsListView.SelectedItems.Count)
            {
                uint sequenceNumber = uint.Parse(m_AuditEventsListView.SelectedItems[0].Text);
                string auditEventIdentifier = ApiInterop.GetDesignatedAuditEventIdentifierBySequenceNumber(sequenceNumber);
                string textToCopy = ApiInterop.GetAuditEventGeneralDetails(auditEventIdentifier);
                textToCopy = textToCopy.Remove(0, 1);
                textToCopy = textToCopy.Remove(textToCopy.Length - 1, 1);
                Clipboard.SetText(textToCopy.Replace("\",\"", "\",\r\n\""));
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_CopyEventDataButton_Click(
            object sender,
            EventArgs e
            )
        {
            if (1 == m_AuditEventsListView.SelectedItems.Count)
            {
                uint sequenceNumber = uint.Parse(m_AuditEventsListView.SelectedItems[0].Text);
                string auditEventIdentifier = ApiInterop.GetDesignatedAuditEventIdentifierBySequenceNumber(sequenceNumber);
                string textToCopy = ApiInterop.GetAuditEventAdditionalDataDetails(auditEventIdentifier);
                if ("None" != textToCopy)
                {
                    textToCopy = textToCopy.Remove(0, 1);
                    textToCopy = textToCopy.Remove(textToCopy.Length - 1, 1);
                    Clipboard.SetText(textToCopy.Replace("\",\"", "\",\r\n\""));
                }
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_AuditEventsListView_SelectedIndexChanged(
            object sender,
            EventArgs e
            )
        {
            if (1 == m_AuditEventsListView.SelectedItems.Count)
            {
                uint sequenceNumber = uint.Parse(m_AuditEventsListView.SelectedItems[0].Text);
                string auditEventIdentifier = ApiInterop.GetDesignatedAuditEventIdentifierBySequenceNumber(sequenceNumber);
                string textToPrint = ApiInterop.GetAuditEventGeneralDetails(auditEventIdentifier);
                textToPrint = textToPrint.Remove(0, 1);
                textToPrint = textToPrint.Remove(textToPrint.Length - 1, 1);
                m_AuditEventPropertiesTextBox.Text = textToPrint.Replace(",", ",\r\n"); ;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_AuditEventsListView_DoubleClick(
            object sender,
            EventArgs e
            )
        {
            if (1 == m_AuditEventsListView.SelectedItems.Count)
            {
                uint sequenceNumber = uint.Parse(m_AuditEventsListView.SelectedItems[0].Text);
                string auditEventIdentifier = ApiInterop.GetDesignatedAuditEventIdentifierBySequenceNumber(sequenceNumber);
                // Make sure that the current selection within the listview is clear since
                // we are switching list views
                m_AuditEventsListView.SelectedIndices.Clear();
                m_AuditEventPropertiesTextBox.Clear();
                // Reload the nodes
                this.ReloadDesignatedNodes(auditEventIdentifier);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_FlatViewListView_DoubleClick(
            object sender,
            EventArgs e
            )
        {
            if (1 == m_FlatViewListView.SelectedItems.Count)
            {
                string auditEventIdentifier = m_FlatViewListView.SelectedItems[0].Tag.ToString();
                int sequenceNumber = (int) ApiInterop.GetAuditEventSequenceNumber(auditEventIdentifier);
                string parentIdentifier = ApiInterop.GetAuditEventParentIdentifier(auditEventIdentifier);
                m_AuditEventsListView.SelectedIndices.Clear();
                m_AuditEventPropertiesTextBox.Clear();
                // Reload the nodes
                this.ReloadDesignatedNodes(parentIdentifier);
                m_AuditEventsListView.SelectedIndices.Add(sequenceNumber);
                m_AuditEventViewTabControl.SelectedTab = m_TreeViewTabPage;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_AuditEventsListView_MouseEnter(
            object sender,
            EventArgs e
            )
        {
            m_MessageStripStatusLabel.Text = "Double click items to drill down...";
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_AuditEventPropertiesTextBox_MouseEnter(
            object sender,
            EventArgs e
            )
        {
            m_MessageStripStatusLabel.Text = "Right click to see context menu...";
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_RefreshButton_MouseEnter(
            object sender,
            EventArgs e
            )
        {
            m_MessageStripStatusLabel.Text = "Click REFRESH to reload all of the events from SAIL SaaS..";
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_BackButton_MouseEnter(
            object sender,
            EventArgs e
            )
        {
            m_MessageStripStatusLabel.Text = "Click back button to return to parent node...";
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Generic_MouseLeave(
            object sender,
            EventArgs e
            )
        {
            m_MessageStripStatusLabel.Text = "";
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewErrorsRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewWarningsRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewInformationalRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewAccessesRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewYserOragnizationActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewAuditLogAccessesActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewDigitalContractActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewDatasetActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewVmActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewDataConnectorActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewJobEngineActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewPushFnActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewExecFnActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewPushDataActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewPullDataActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewDeleteDataActivityRadioButton_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            this.RedrawFlatView();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ErrorStripStatusLabel_Click(
            object sender,
            EventArgs e
            )
        {
            m_ExceptionsDialog.ShowDialog(this);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_StatusBarRefreshTimer_Tick(
            object sender,
            EventArgs e
            )
        {
            // Update the time
            m_TimeStripStatusLabel.Text = System.DateTime.UtcNow.ToString("MMMM d, yyyy @ HH:mm:ss") + " GMT";
            // Update the exceptions link if it exists
            if ((0 < ApiInterop.GetExceptionCount()) || (0 < m_ExceptionsDialog.GetExceptionsCount()))
            {
                m_ErrorStripStatusLabel.Text = "Click to view exceptions!";
            }
            else
            {
                m_ErrorStripStatusLabel.Text = "";
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_CopyCurrentAuditEventPropertiesToolStripMenuItem_Click(
            object sender,
            EventArgs e
            )
        {
            Clipboard.SetText(m_AuditEventPropertiesTextBox.Text);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ViewAdditionalDataDetailsToolStripMenuItem_Click(
            object sender,
            EventArgs e
            )
        {
            uint sequenceNumber = uint.Parse(m_AuditEventsListView.SelectedItems[0].Text);
            string auditEventIdentifier = ApiInterop.GetDesignatedAuditEventIdentifierBySequenceNumber(sequenceNumber);
            PropertyView propertyView = new PropertyView();
            propertyView.Property = ApiInterop.GetAuditEventAdditionalDataDetails(auditEventIdentifier);
            propertyView.ShowDialog();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="e"></param>
        private void m_AuditEventViewTabControl_SelectedIndexChanged(
            object sender,
            EventArgs e
            )
        {
            if (m_AuditEventViewTabControl.SelectedTab == m_TreeViewTabPage)
            {
                this.RedrawDesignatedNodes();
            }
            else
            {
                this.RedrawFlatView();
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_FlatViewListView_SelectedIndexChanged(
            object sender,
            EventArgs e
            )
        {
            if (1 == m_FlatViewListView.SelectedItems.Count)
            {
                string auditEventIdentifier = m_FlatViewListView.SelectedItems[0].Tag.ToString();
                string textToPrint = ApiInterop.GetAuditEventGeneralDetails(auditEventIdentifier);
                textToPrint = textToPrint.Remove(0, 1);
                textToPrint = textToPrint.Remove(textToPrint.Length - 1, 1);
                m_FlatViewPropertiesTextBox.Text = textToPrint.Replace(",", ",\r\n"); ;
            }
        }

        private ExceptionsDialog m_ExceptionsDialog;
    }
}
