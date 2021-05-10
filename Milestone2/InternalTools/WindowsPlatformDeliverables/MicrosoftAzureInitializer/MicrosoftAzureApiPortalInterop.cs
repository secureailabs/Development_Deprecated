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
    /// <param name="c_szSubscriptionIdentifier"></param>
    /// <param name="c_szResourceGroup"></param>
    /// <param name="c_szVirtualMachineIdentifier"></param>
    /// <param name="c_szPublicIpSpecification"></param>
    /// <param name="c_szNetworkInterfaceSpecification"></param>
    /// <param name="c_szVirtualMachineSpecification"></param>
    /// <returns></returns>
    [DllImport("MicrosoftAzureApiFunctions.dll", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string DeployVirtualMachineAndWait(string c_szSubscriptionIdentifier, string c_szResourceGroup, string c_szVirtualMachineIdentifier, string confidentialVirtualMachineSpecification, string c_szLocation);

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