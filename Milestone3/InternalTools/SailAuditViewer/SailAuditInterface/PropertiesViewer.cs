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
    public partial class PropertiesViewer : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public PropertiesViewer()
        {
            InitializeComponent();
        }

        /// <summary>
        ///
        /// </summary>
        public string Properties
        {
            get
            {
                return m_PropertiesTextBox.Text;
            }

            set
            {
                m_PropertiesTextBox.Text = value;
            }

        }
    }
}
