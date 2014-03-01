using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace FrogLiesGui
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            this.tabControl1.Controls.Remove(this.tabWebInfo);
            addAData("Header", "Content-Type: multipart/form-data; boundary=---------------------------28251299466151");
            addAData( "Raw",  "-----------------------------28251299466151\\r\\n" );
            addAData( "Raw",  "Content-Disposition: form-data; name=\"file\"; filename=\"" );
            addAData( "String",  "Filename" );
            addAData( "Raw",  "\"\\r\\n" );
            addAData( "Raw",  "Content-Type: " );
            addAData( "String",  "Mimetype" );
            addAData( "Raw",  "\\r\\n\\r\\n" );
            addAData( "Data",  "FileData" );
            addAData( "Raw",  "\\r\\n-----------------------------28251299466151\\r\\n" );
            addAData( "Raw",  "Content-Disposition: form-data; name=\"password\"\\r\\n\\r\\n" );
            addAData( "String",  "Password" );
            addAData( "Raw",  "\\r\\n-----------------------------28251299466151\\r\\n" );
            addAData( "Raw",  "Content-Disposition: form-data; name=\"owner\"\\r\\n\\r\\n" );
            addAData( "String",  "Username" );
            addAData( "Raw",  "\\r\\n-----------------------------28251299466151--\\r\\n" );

        }

        private void tabUserInfo_Click(object sender, EventArgs e)
        {

        }

        private void dataGridView1_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }

        private void button13_Click(object sender, EventArgs e)
        {
            colorDialog1.ShowDialog();
            colorPanel.BackColor = colorDialog1.Color;
        }

        private void textBox14_TextChanged(object sender, EventArgs e)
        {
            try
            {
                double t = Convert.ToDouble(AlphaText.Text);
                AlphaText.BackColor = Color.White;
            }
            catch (Exception ee)
            {
                AlphaText.BackColor = Color.Red;
            }
        }

        private void AlphaText_Leave(object sender, EventArgs e)
        {
            try
            {
                double t = Convert.ToDouble(AlphaText.Text);
                if (t < 0)
                    t = 0;
                if (t > 1)
                    t = 1;
                AlphaText.Text = t.ToString("N3");
                AlphaText.BackColor = Color.White;
            }
            catch (Exception ee)
            {
                AlphaText.BackColor = Color.Red;
            }
        }

        private void tabGeneral_MouseClick(object sender, MouseEventArgs e)
        {
            AlphaText_Leave(sender, null);
        }
        private List<System.Windows.Forms.Button> RemoveButtons = new List<System.Windows.Forms.Button>();
        private List<System.Windows.Forms.ComboBox> ComboBoxes = new List<System.Windows.Forms.ComboBox>();
        private List<System.Windows.Forms.TextBox> TextBoxes = new List<System.Windows.Forms.TextBox>();


        private void addAData(String type, String value)
        {
            System.Windows.Forms.ComboBox ComboType1 = new System.Windows.Forms.ComboBox();
            System.Windows.Forms.TextBox TextBoxData1 = new System.Windows.Forms.TextBox();
            System.Windows.Forms.Button removeData1 = new System.Windows.Forms.Button();


            // 
            // ComboType1
            // 
            ComboType1.FormattingEnabled = true;
            ComboType1.Items.AddRange(new object[] {
            "Raw",
            "String",
            "Data",
            "Number",
            "Header"});
            ComboType1.Location = new System.Drawing.Point(8, 3+29*TextBoxes.Count);
            ComboType1.Name = "ComboType1";
            ComboType1.Size = new System.Drawing.Size(62, 21);
            ComboType1.TabIndex = 0;
            ComboType1.Text = type;
            // 
            // TextBoxData1
            // 
            TextBoxData1.Location = new System.Drawing.Point(76, 4 + 29 * TextBoxes.Count);
            TextBoxData1.Name = "TextBoxData1";
            TextBoxData1.Size = new System.Drawing.Size(476, 20);
            TextBoxData1.TabIndex = 1;
            TextBoxData1.Text = value;
            // 
            // removeData1
            // 
            removeData1.Location = new System.Drawing.Point(558, 4 + 29 * TextBoxes.Count);
            removeData1.Name = "removeData"+TextBoxes.Count.ToString();
            removeData1.Size = new System.Drawing.Size(58, 23);
            removeData1.TabIndex = 2;
            removeData1.Text = "Remove";
            removeData1.UseVisualStyleBackColor = true;
            removeData1.Click += new System.EventHandler(this.buttonrem_Click);

            TextBoxes.Add(TextBoxData1);
            ComboBoxes.Add(ComboType1);
            RemoveButtons.Add(removeData1);
            this.scrollpanel.Controls.Add(TextBoxData1);
            this.scrollpanel.Controls.Add(ComboType1);
            this.scrollpanel.Controls.Add(removeData1);
            this.addData.Top += 29;
            this.scrollpanel.Height = this.addData.Top + 32;

            if (this.scrollpanel.Height > this.panel2.Height )
            {
                this.vScrollBar1.Enabled = true;
                this.vScrollBar1.Maximum = (this.scrollpanel.Height - this.panel2.Height + 60);
                this.vScrollBar1.Value = this.scrollpanel.Height - this.panel2.Height;
                this.vScrollBar1.LargeChange = 60;
                this.vScrollBar1.SmallChange = 20;

                this.scrollpanel.Top = -vScrollBar1.Value;
            }
            else
            {
                this.vScrollBar1.Enabled = false;
                this.vScrollBar1.Maximum = 0;
            }
        }

        private void button14_Click(object sender, EventArgs e)
        {
            addAData("Raw", "");

        }
        private void buttonrem_Click(object sender, EventArgs e)
        {
            System.Windows.Forms.Button b = (System.Windows.Forms.Button)sender;
            String t = b.Name.Substring(10);
            int v = Convert.ToInt32(t);
            System.Console.WriteLine(t);
            scrollpanel.Controls.Remove(TextBoxes[v]);
            scrollpanel.Controls.Remove(ComboBoxes[v]);
            scrollpanel.Controls.Remove(RemoveButtons[v]);
            for (int i = v; i < TextBoxes.Count-1; ++i)
            {
                TextBoxes[i] = TextBoxes[i + 1];
                TextBoxes[i].Top -= 29;

                ComboBoxes[i] = ComboBoxes[i + 1];
                ComboBoxes[i].Top -= 29;

                RemoveButtons[i] = RemoveButtons[i + 1];
                RemoveButtons[i].Top -= 29;
                RemoveButtons[i].Name = "removeData" + i.ToString();
            }
            TextBoxes.RemoveAt(TextBoxes.Count - 1);
            ComboBoxes.RemoveAt(ComboBoxes.Count - 1);
            RemoveButtons.RemoveAt(RemoveButtons.Count - 1);

            this.addData.Top -= 29;
            this.scrollpanel.Height = this.addData.Top + 32;
            if (this.scrollpanel.Height > this.panel2.Height)
            {
                this.vScrollBar1.Enabled = true;
                this.vScrollBar1.Maximum = (this.scrollpanel.Height - this.panel2.Height + 60);
                this.vScrollBar1.LargeChange = 60;
                this.vScrollBar1.SmallChange = 20;
                if (this.scrollpanel.Top + this.scrollpanel.Height < panel2.Height)
                {
                    this.scrollpanel.Top = panel2.Height - this.scrollpanel.Height;
                }
                
            }
            else
            {
                this.vScrollBar1.Enabled = false;
                this.vScrollBar1.Maximum = 0;
            }

        }


        private void vScrollBar1_Scroll(object sender, ScrollEventArgs e)
        {
            this.scrollpanel.Top = -vScrollBar1.Value;
        }

        private void panel2_MouseWheel(object sender, MouseEventArgs e)
        {
            if (tabControl1.SelectedTab == tabWebInfo)
            {
                int d = e.Delta / 4;
                if (d == 0)
                    d = e.Delta;
                int tmp = vScrollBar1.Value - d;
                if (tmp > vScrollBar1.Maximum - vScrollBar1.LargeChange)
                    tmp = vScrollBar1.Maximum - vScrollBar1.LargeChange;
                if (tmp < vScrollBar1.Minimum)
                    tmp = vScrollBar1.Minimum;

                vScrollBar1.Value = tmp;
                vScrollBar1_Scroll(sender, null);
            }
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox1.Checked)
            {
                this.tabControl1.Controls.Add(this.tabWebInfo);
            }
            else
            {
                this.tabControl1.Controls.Remove(this.tabWebInfo);
            }
        }

        private void tabGeneral_Click(object sender, EventArgs e)
        {

        }

        private void button9_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "Wav files|*.wav";
            openFileDialog1.FileName = "";
            if( openFileDialog1.ShowDialog() == DialogResult.OK )
                SoundStartup.Text = openFileDialog1.FileName;
        }

        private void button10_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "Wav files|*.wav";
            openFileDialog1.FileName = "";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
                SoundUpload.Text = openFileDialog1.FileName;
        }

        private void button11_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "Wav files|*.wav";
            openFileDialog1.FileName = "";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
                SoundFinishUpload.Text = openFileDialog1.FileName;
        }

        private void button12_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "Wav files|*.wav";
            openFileDialog1.FileName = "";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
                SoundError.Text = openFileDialog1.FileName;
        }

        private void checkBox6_CheckedChanged(object sender, EventArgs e)
        {
            //GroupSelectionColor.Enabled = checkBox6.Checked;
        }

        private void checkBox12_CheckedChanged(object sender, EventArgs e)
        {
            LocalSaves.Enabled = checkBox12.Checked;
        }

        private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
    }
}
