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
    public partial class ExceptionsDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public ExceptionsDialog()
        {
            InitializeComponent();
            m_ExceptionsCount = 0;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="e"></param>
        protected override void OnShown(
            EventArgs e
            )
        {
            // Add new exceptions to the dialog
            bool oneMoreException = false;
            do
            {
                string newException = ApiInterop.GetNextException();
                if (0 < newException.Length)
                {
                    // Increate the internal count of exceptions. This is addressed
                    // externally
                    m_ExceptionsCount++;
                    // Add a separator
                    m_ExceptionsListBox.Items.Add("------------------------------------------------------------------------------------------");
                    // We want to do a split string on the incoming string, but we need
                    // to make sure there is only ONE split symbol
                    newException = newException.Replace("\r\n", "\n");
                    newException = newException.Replace("\n\r", "\n");
                    newException = newException.Replace("\r", "\n");
                    // Now, let's split the string
                    string[] singleStrings = newException.Split('\n');
                    foreach (string oneSingleString in singleStrings)
                    {
                        m_ExceptionsListBox.Items.Add(oneSingleString);
                    }
                    // Try getting another exception, so continue looping
                    oneMoreException = true;
                }
                else
                {
                    oneMoreException = false;
                }
            }
            while (true == oneMoreException);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_ClearExceptionsButton_Click(
            object sender,
            EventArgs e
            )
        {
            m_ExceptionsListBox.Items.Clear();
            m_ExceptionsCount = 0;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_CloseExceptionsDialogButton_Click(
            object sender,
            EventArgs e
            )
        {
            this.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public uint GetExceptionsCount()
        {
            return m_ExceptionsCount;
        }

        private uint m_ExceptionsCount;
    }
}