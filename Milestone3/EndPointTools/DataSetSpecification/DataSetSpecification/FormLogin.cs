using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
using System.IO;
using System.Runtime.InteropServices;
using RestSharp;
using Newtonsoft.Json;

namespace DataSetSpecification
{
    public partial class FormLogin : Form
    {
        public DataSetInfo m_container;
        public string m_userName;
        public string m_password;
        public string m_authorizationTokken = "";
        public string m_strWebPortalUrl = "";
        public bool m_isDatasetToBeRegistered = false;

        public FormLogin()
        {
            InitializeComponent();
            textBox1.Text = "nadams@mghl.com";
            textBoxWebPortalUrl.Text = "137.116.90.145";
            textBox2.Text = "";
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if ("" == textBox1.Text)
            {
                loginStatus.Text = "Invalid UserEmail!!";
            }
            else if ("" == textBox2.Text)
            {
                loginStatus.Text = "Invalid Password!!";
            }
            else 
            {
                try
                {
                    m_strWebPortalUrl = textBoxWebPortalUrl.Text;
                    var client = new RestClient("https://"+ m_strWebPortalUrl + ":6200/SAIL/AuthenticationManager/User/Login?Email=" + textBox1.Text + "&Password=" + textBox2.Text)
                    {
                        // ************* VERY VERY IMPORTANT ********************************************/
                        // TODO: this is temporary to ignore the certificate validation for the rest call 
                        // but should be removed in production.
                        RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => true
                    };
                    var request = new RestRequest(Method.POST);
                    IRestResponse response = client.Execute(request);
                    dynamic responseJsonObject = JsonConvert.DeserializeObject(response.Content);
                    if ("201" == responseJsonObject.Status.ToString())
                    {
                        loginStatus.Text = "Login Success!!";
                        m_authorizationTokken = responseJsonObject.Eosb;
                        m_isDatasetToBeRegistered = checkBoxDatasetRegister.Checked;
                        Thread.Sleep(500);
                        this.Hide();
                        client = null;
                        m_container.GetAuth();
                    }
                    else
                    {
                        loginStatus.Text = "Login Fail. Try Again!!";
                    }
                }
                catch (Exception oException)
                {
                    loginStatus.Text = oException.Message;
                }
            }
        }
        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            m_userName = textBox1.Text;
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {
            m_password = textBox2.Text;
        }
    }
}
