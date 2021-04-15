using System;
using System.Runtime.InteropServices;

public class ApiInterop
{
    /// <summary>
    /// 
    /// </summary>
    /// <param name="value"></param>
    /// <param name="caseSensitive"></param>
    /// <returns></returns>
    [DllImport("64BitHashFunctions.dll", CallingConvention = CallingConvention.Cdecl)]
    static extern public ulong Get64BitHashOfString(string value, bool caseSensitive);

}