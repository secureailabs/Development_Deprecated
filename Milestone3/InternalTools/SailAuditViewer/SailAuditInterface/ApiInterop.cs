using System;
using System.Runtime.InteropServices;

public class ApiInterop
{
    /// <summary>
    /// 
    /// </summary>
    /// <param name="ipAddressOfSailWebApiPortal"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public bool SetSailWebApiPortalIpAddress(string ipAddressOfSailWebApiPortal);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="username"></param>
    /// <param name="password"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public bool Login(string username, string password);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="parentIdentifier"></param>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public void ReloadLoadAllAuditEvents(string parentIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="parentIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public bool DesignateAuditEventsUsingParentIdentifier(string parentIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetDesignatedAuditEventsParentIdentifier();

    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public uint GetDesignatedAuditEventsCount();

    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public uint GetAllLoadedAuditEventsCount();

    /// <summary>
    /// 
    /// </summary>
    /// <param name="sequenceNumber"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetDesignatedAuditEventIdentifierBySequenceNumber(uint sequenceNumber);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="sequenceNumber"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetAuditEventIdentifierByIndex(uint sequenceNumber);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="auditEventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetAuditEventOrganizationalIdentifier(string auditEventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetAuditEventParentIdentifier(string eventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public bool IsBranchNodeEditEvent(string eventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public uint GetAuditEventSequenceNumber(string eventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetAuditEventDateGmt(string eventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetAuditEventTimeWithMillisecondsGmt(string eventIdentifier);
    
    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public ulong GetAuditEventGmtEpochTimestampInMilliseconds(string eventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetAuditEventEpochTimestampInMillisecondsGmt(string eventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetAuditEventType(string eventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetAuditEventGeneralDetails(string eventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="eventIdentifier"></param>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetAuditEventAdditionalDataDetails(string eventIdentifier);

    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    static extern public uint GetExceptionCount();

    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    [return: MarshalAs(UnmanagedType.BStr)]
    static extern public string GetNextException();
}