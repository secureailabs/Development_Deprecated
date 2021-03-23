using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DataSetSpecification
{
    public partial class UserControlTableColumn : UserControl
    {
        public string m_columnGuid = (new Guid()).ToString();
        public byte m_dataType = 0;
        public string m_columnName = "";
        public string m_columnDescription = "Column Description";
        public string m_columnKeyword = "Column keyword";
        public byte m_sensitivityLevel = 0;

        public UserControlTableColumn()
        {
            InitializeComponent();
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            string stringDataType = comboBox1.Text;
            if ("String" == stringDataType)
            {
                m_dataType = 1;
            }
            else if ("Float" == stringDataType)
            {
                m_dataType = 2;
            }
            else if ("Integer" == stringDataType)
            {
                m_dataType = 3;
            }
            else if ("Boolean" == stringDataType)
            {
                m_dataType = 4;
            }
        }

        private void trackBar1_Scroll_1(object sender, EventArgs e)
        {
            TrackBar tr = (TrackBar)sender;
            TrackBar parentTrackBar = (TrackBar)tr.Parent.Parent.Parent.Controls.Find("trackBar1", true)[0];
            if (trackBar1.Value < parentTrackBar.Value)
            {
                trackBar1.Value = parentTrackBar.Value;
            }
            m_sensitivityLevel = (Byte)trackBar1.Value;
        }

        public void label1_setvalue(string columnName)
        {
            label1.Text = columnName;
            m_columnName = columnName;
        }

        public StructuredBuffer GetColumnStructuredBuffer()
        {
            StructuredBuffer oStructuredBuffer = new StructuredBuffer();
            oStructuredBuffer.PutString("Guid", m_columnGuid);
            oStructuredBuffer.PutInt32("DataType", m_dataType);
            oStructuredBuffer.PutString("Name", m_columnName);
            oStructuredBuffer.PutString("Description", m_columnDescription);
            oStructuredBuffer.PutString("Keywords", m_columnKeyword);
            oStructuredBuffer.PutInt32("PrivacyLevel", m_sensitivityLevel);

            return oStructuredBuffer;
        }
    }
}
