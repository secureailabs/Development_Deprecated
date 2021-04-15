using System;
using System.Collections.Generic;
using System.Text;

namespace MicrosoftAzureVirtualMachineInitializer
{
    public class MicrosoftAzureVirtualMachine
    {
        /// <summary>
        /// 
        /// </summary>
        /// <param name="clusterIdentifier"></param>
        /// <param name="digitalContractIdentifier"></param>
        /// <param name="datasetIdentifier"></param>
        /// <param name="ipAddress"></param>
        /// <param name=""></param>
        public MicrosoftAzureVirtualMachine(
            string clusterIdentifier,
            string digitalContractIdentifier,
            string datasetIdentifier,
            string datasetFilename,
            string sailWebApiPortalIpAddress,
            string ipAddress
            )
        {
            // First we try to load the dataset since
            if (false == System.IO.File.Exists(datasetFilename))
            {
                throw new InvalidOperationException("Data file does not exist");
            }

            m_DatasetFilename = datasetFilename;
            m_Dataset = System.IO.File.ReadAllBytes(datasetFilename);
            m_DatasetIdentifier = datasetIdentifier;
            m_VirtualMachineIdentifier = System.Guid.NewGuid().ToString("D").ToUpper();
            m_ClusterIdentifier = clusterIdentifier;
            m_RootOfTrustDomainIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
            m_ComputationalDomainIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
            m_DataConnectorDomainIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
            m_DigitalContractIdentifier = digitalContractIdentifier.ToUpper();
            m_VirtualMachineIpAddress = ipAddress;
            m_SailWebApiPortalIpAddress = sailWebApiPortalIpAddress;
            m_VirtualMachineStatus = "Not Started";
            m_VirtualMachineUsername = "saildeveloper";
            m_VirtualMachinePassword = "Iw2btin2AC+beRl&dir!";
            m_VirtualMachineComputerName = "SailSecureVm";
        }

        /// <summary>
        /// Default constructor
        /// </summary>
        public MicrosoftAzureVirtualMachine(
            string clusterIdentifier,
            string digitalContractIdentifier,
            string datasetIdentifier,
            string datasetFilename,
            string sailWebApiPortalIpAddress,
            string subscriptionIdentifier,
            string resourceGroup,
            string location,
            string virtualNetwork,
            string networkSecurityGroup,
            string baseMachineName,
            string virtualMachineSize
            )
        {
            // First we try to load the dataset since
            if (false == System.IO.File.Exists(datasetFilename))
            {
                throw new InvalidOperationException("Data file does not exist");
            }

            m_DatasetFilename = datasetFilename;
            m_Dataset = System.IO.File.ReadAllBytes(datasetFilename);
            m_DatasetIdentifier = datasetIdentifier;
            m_VirtualMachineIdentifier = System.Guid.NewGuid().ToString("D").ToUpper();
            m_ClusterIdentifier = clusterIdentifier;
            m_RootOfTrustDomainIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
            m_ComputationalDomainIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
            m_DataConnectorDomainIdentifier = System.Guid.NewGuid().ToString("B").ToUpper();
            m_DigitalContractIdentifier = digitalContractIdentifier.ToUpper();
            m_VirtualMachineIpAddress = "000.000.000.000";
            m_SailWebApiPortalIpAddress = sailWebApiPortalIpAddress;
            m_VirtualMachineStatus = "Not Started";
            m_SubscriptionIdentifier = subscriptionIdentifier;
            m_ResourceGroup = resourceGroup;
            m_Location = location;
            m_VirtualNetwork = virtualNetwork;
            m_NetworkSecurityGroup = networkSecurityGroup;
            m_BaseMachineName = baseMachineName;
            m_VirtualMachineSize = virtualMachineSize;
            m_VirtualMachineUsername = "saildeveloper";
            m_VirtualMachinePassword = "Iw2btin2AC+beRl&dir!";
            m_VirtualMachineComputerName = "SailSecureVm";
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureSubscriptionIdentifier
        {
            get
            {
                return m_SubscriptionIdentifier;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureResourceGroup
        {
            get
            {
                return m_ResourceGroup;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureLocation
        {
            get
            {
                return m_Location;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureVirtualNetwork
        {
            get
            {
                return m_VirtualNetwork;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureNetworkSecurityGroup
        {
            get
            {
                return m_NetworkSecurityGroup;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureBaseMachineName
        {
            get
            {
                return m_BaseMachineName;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string AzureVirtualMachineSize
        {
            get
            {
                return m_VirtualMachineSize;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string VirtualMachineIdentifier
        {
            get
            {
                return m_VirtualMachineIdentifier;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string ClusterIdentifier
        {
            get
            {
                return m_ClusterIdentifier;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string DigitalContractIdentifier
        {
            get
            {
                return m_DigitalContractIdentifier;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string DatasetIdentifier
        {
            get
            {
                return m_DatasetIdentifier;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string RootOfTrustDomainIdentifier
        {
            get
            {
                return m_RootOfTrustDomainIdentifier;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string ComputationalDomainIdentifier
        {
            get
            {
                return m_ComputationalDomainIdentifier;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string DataConnectorDomainIdentifier
        {
            get
            {
                return m_DataConnectorDomainIdentifier;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string SailWebApiPortalIpAddress
        {
            get
            {
                return m_SailWebApiPortalIpAddress;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string DatasetFilename
        {
            get
            {
                return m_DatasetFilename;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public byte[] Dataset
        {
            get
            {
                return m_Dataset;
            }
        }

        public string IpAddress
        {
            get
            {
                return m_VirtualMachineIpAddress;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public string VirtualMachineStatus
        {
            get
            {
                return m_VirtualMachineStatus;
            }
        }

        /// <summary>
        /// Call this method to effectively provision the virtual machine within Microsoft Azure and once it
        /// is running, initialize it with the SAIL initialization data
        /// </summary>
        /// <returns></returns>
        public void ProvisionAndInitialize()
        {
            m_VirtualMachineStatus = "Starting...";
            m_ProvisionAndInitializationThread = new System.Threading.Thread(this.ProvisionAndInitializeThread);
            m_ProvisionAndInitializationThread.Start();
        }

        /// <summary>
        /// 
        /// </summary>
        private void ProvisionAndInitializeThread()
        {
            // Create a virtual machine in azure
            if ("000.000.000.000" == m_VirtualMachineIpAddress)
            {
                // Create a public ip address
                string publicIpSpecification = this.FillTemplate(global::MicrosoftAzureInitializer.Properties.Resources.ResourceManager.GetString("PublicIpJson"));
                string networkInterfaceSpecification = this.FillTemplate(global::MicrosoftAzureInitializer.Properties.Resources.ResourceManager.GetString("NetworkInterfaceJson"));
                string virtualMachineSpecification = this.FillTemplate(global::MicrosoftAzureInitializer.Properties.Resources.ResourceManager.GetString("VirtualMachineFromImageJson"));
                // Do some specification substitutions ups that are above and beyond the FillTemplate() substitutions
                publicIpSpecification = publicIpSpecification.Replace("{{DnsLabel}}", ("sail" + m_VirtualMachineIdentifier).ToLower());
                networkInterfaceSpecification = networkInterfaceSpecification.Replace("{{Name}}", (m_VirtualMachineIdentifier + "-nic").ToLower());
                networkInterfaceSpecification = networkInterfaceSpecification.Replace("{{IpAddressId}}", (m_VirtualMachineIdentifier + "-ip").ToLower());
                virtualMachineSpecification = virtualMachineSpecification.Replace("{{OsDiskName}}", (m_VirtualMachineIdentifier + "-disk").ToLower());
                virtualMachineSpecification = virtualMachineSpecification.Replace("{{NetworkInterface}}", (m_VirtualMachineIdentifier + "-nic").ToLower());
                virtualMachineSpecification = virtualMachineSpecification.Replace("{{ImageName}}", m_BaseMachineName);
                virtualMachineSpecification = virtualMachineSpecification.Replace("{{VmSize}}", m_VirtualMachineSize);
                virtualMachineSpecification = virtualMachineSpecification.Replace("{{Password}}", m_VirtualMachinePassword);
                virtualMachineSpecification = virtualMachineSpecification.Replace("{{Username}}", m_VirtualMachineUsername);
                virtualMachineSpecification = virtualMachineSpecification.Replace("{{ComputerName}}", m_VirtualMachineComputerName);
                // Provision the virtual machine on Microsoft Azure and get it's IP address
                m_VirtualMachineIpAddress = MicrosoftAzureApiPortalInterop.ProvisionVirtualMachineAndWait(m_SubscriptionIdentifier, m_ResourceGroup, VirtualMachineIdentifier, publicIpSpecification, networkInterfaceSpecification, virtualMachineSpecification);
                m_VirtualMachineStatus = "Installing...";
                // Upload the binaries to the virtual machine
                byte[] installationPackage = System.IO.File.ReadAllBytes("SecureComputationalVirtualMachine.binaries");
                string installationPackageBase64Encoded = System.Convert.ToBase64String(installationPackage);
                if (true == SailWebApiPortalInterop.UploadInstallationPackageToVirtualMachine(m_VirtualMachineIpAddress, installationPackageBase64Encoded))
                {
                    m_VirtualMachineStatus = "Initialization...";
                }
                System.Threading.Thread.Sleep(10);
            }
            else
            {
                m_VirtualMachineStatus = "Initialization...";
            }
            // Now we need to initialize the virtual machine
            string datasetBase64Encoded = System.Convert.ToBase64String(m_Dataset);
            SailWebApiPortalInterop.UploadInitializationParametersToVirtualMachine("Some nice name for the virtual machine", m_VirtualMachineIpAddress, m_VirtualMachineIdentifier, m_ClusterIdentifier, m_DigitalContractIdentifier, m_DatasetIdentifier, m_RootOfTrustDomainIdentifier, m_ComputationalDomainIdentifier, m_DataConnectorDomainIdentifier, m_SailWebApiPortalIpAddress, datasetBase64Encoded);
            m_VirtualMachineStatus = "Ready...";
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="unfilledTemplate"></param>
        /// <returns></returns>
        private string FillTemplate(
            string unfilledTemplate
            )
        {
            string filledTemplate = unfilledTemplate.Replace("{{SubscriptionId}}", m_SubscriptionIdentifier);
            filledTemplate = filledTemplate.Replace("{{ResourceGroup}}", m_ResourceGroup);
            filledTemplate = filledTemplate.Replace("{{Location}}", m_Location);
            filledTemplate = filledTemplate.Replace("{{VirtualNetwork}}", m_VirtualNetwork);
            filledTemplate = filledTemplate.Replace("{{NetworkSecurityGroup}}", m_NetworkSecurityGroup);

            return filledTemplate;
        }

        // Private data members
        private string m_SubscriptionIdentifier;
        private string m_ResourceGroup;
        private string m_Location;
        private string m_VirtualNetwork;
        private string m_NetworkSecurityGroup;
        private string m_BaseMachineName;
        private string m_VirtualMachineSize;
        private string m_VirtualMachineUsername;
        private string m_VirtualMachinePassword;
        private string m_VirtualMachineComputerName;
        private string m_VirtualMachineIdentifier;
        private string m_ClusterIdentifier;
        private string m_DigitalContractIdentifier;
        private string m_DatasetIdentifier;
        private string m_RootOfTrustDomainIdentifier;
        private string m_ComputationalDomainIdentifier;
        private string m_DataConnectorDomainIdentifier;
        private string m_SailWebApiPortalIpAddress;
        private string m_DatasetFilename;
        private byte[] m_Dataset;
        private string m_VirtualMachineIpAddress;
        private string m_VirtualMachineStatus;
        private System.Threading.Thread m_ProvisionAndInitializationThread;
    }
}
