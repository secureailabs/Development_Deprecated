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
    public partial class PropertyView : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public PropertyView()
        {
            InitializeComponent();
        }

        /// <summary>
        /// 
        /// </summary>
        public string Property
        {
            get
            {
                return m_PropertyTextBox.Text;
            }

            set
            {
                m_PropertyTextBox.Text = value;
            }
        }
    }
}
