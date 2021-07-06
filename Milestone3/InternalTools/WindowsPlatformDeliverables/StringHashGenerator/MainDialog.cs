using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace StringTo64BitHashGenerator
{
    public partial class MainDialog : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public MainDialog()
        {
            InitializeComponent();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>

        private void m_TimeRefreshTimer_Tick(
            object sender,
            EventArgs e
            )
        {
            // Update the time
            m_TimeStripStatusLabel.Text = System.DateTime.Now.ToString("MMMM d, yyyy @ HH:mm:ss");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_TextToConvertTextBox_TextChanged(
            object sender,
            EventArgs e
            )
        {
            m_64BitHashTextBox.Text = string.Format("0x{0:X16}", ApiInterop.Get64BitHashOfString(m_TextToConvertTextBox.Text, m_CaseSensitiveCheckBox.Checked));
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_CaseSensitiveCheckBox_CheckedChanged(
            object sender,
            EventArgs e
            )
        {
            if (0 < m_TextToConvertTextBox.Text.Length)
            {
                m_64BitHashTextBox.Text = string.Format("0x{0:X16}", ApiInterop.Get64BitHashOfString(m_TextToConvertTextBox.Text, m_CaseSensitiveCheckBox.Checked));
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_PastLinkLabel_LinkClicked(
            object sender,
            EventArgs e
            )
        {
            m_TextToConvertTextBox.Text = Clipboard.GetText();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void m_Copy64BitHashLinkLabel_LinkClicked(
            object sender,
            EventArgs e
            )
        {
            Clipboard.SetText(m_64BitHashTextBox.Text);
        }
    }
}
