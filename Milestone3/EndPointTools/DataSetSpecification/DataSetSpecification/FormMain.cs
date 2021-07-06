using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DataSetSpecification
{
    public partial class FormMain : Form
    {
        public int m_nNumberOfTables = 0;
        private int m_nTableID = 0;
        public FormMain()
        {
            InitializeComponent();
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
            DataSetInfo oFullDataInfo = new DataSetInfo
            {
                m_firstPage = this
            };
            PanelDataSetInfo.Controls.Add(oFullDataInfo);
        }

        private void ButtonAddNewDataSet_Click(object sender, EventArgs e)
        {
            TableInfo newTable = new TableInfo();
            Label oDataSetLabel = (Label)newTable.Controls.Find("DataSetID", true)[0];
            oDataSetLabel.Text = oDataSetLabel.Text + " " + (m_nTableID++).ToString();
            FlowLayoutPanelTables.Controls.Add(newTable);
            Label rowlabel = (Label)PanelDataSetInfo.Controls.Find("numTables", true)[0];
            rowlabel.Text = (++m_nNumberOfTables).ToString();
        }

    }
}
