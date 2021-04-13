using System;
using System.Runtime.InteropServices;

public class MicrosoftAzureApiPortalInterop
{
    /// <summary>
    /// 
    /// </summary>
    /// <param name="applicationIdentifier"></param>
    /// <param name="secret"></param>
    /// <param name="tenantIdentifier"></param>
    /// <returns></returns>
    [DllImport("MicrosoftAzureApiFunctions.dll", CallingConvention = CallingConvention.Cdecl)]
    static extern public bool LoginToMicrosoftAzureApiPortal(string applicationIdentifier, string secret, string tenantIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="subscriptionIdentifier"></param>
    /// <param name="resourceGroup"></param>
    /// <param name="virtualMachineIdentifier"></param>
    /// <param name="publicIpSpecification"></param>
    /// <param name="networkInterfaceSpecification"></param>
    /// <param name="virtualMachineSpecification"></param>
    /// <returns></returns>
    [DllImport("MicrosoftAzureApiFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string ProvisionVirtualMachineAndWait(string subscriptionIdentifier, string resourceGroup, string virtualMachineIdentifier, string publicIpSpecification, string networkInterfaceSpecification, string virtualMachineSpecification);

    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    [DllImport("MicrosoftAzureApiFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public uint GetExceptionCount();

    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    [DllImport("MicrosoftAzureApiFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetNextException();
}