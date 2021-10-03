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
using Microsoft.Win32;

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

            textBox1.Text = "";
            textBoxWebPortalUrl.Text = "";
            textBox2.Text = "";

            // Load default settings from the registry of they exist
            RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
            string[] registryKeyValues = registryKey.GetValueNames();
            if (true == registryKeyValues.Contains("DefaultSailWebApiPortalIpAddress"))
            {
                textBoxWebPortalUrl.Text = registryKey.GetValue("DefaultSailWebApiPortalIpAddress").ToString();
            }
            if (true == registryKeyValues.Contains("DefaultSailWebApiPortalUsername"))
            {
                textBox1.Text = registryKey.GetValue("DefaultSailWebApiPortalUsername").ToString();
            }
            registryKey.Close();

            if ("" != textBoxWebPortalUrl.Text)
            {
                if ("" != textBox1.Text)
                {
                    textBox2.Select();
                }
                else
                {
                    textBox1.Select();
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            bool fLoginSuccess = false;
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
                    // Persist some of the settings to the registry to make it easier to restart the
                    // application later.
                    RegistryKey registryKey = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\SAIL");
                    registryKey.SetValue("DefaultSailWebApiPortalIpAddress", textBoxWebPortalUrl.Text);
                    registryKey.SetValue("DefaultSailWebApiPortalUsername", textBox1.Text);
                    registryKey.Close();

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
                        fLoginSuccess = true;
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

                if (true == fLoginSuccess)
                {
                    m_container.GetAuth();
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
