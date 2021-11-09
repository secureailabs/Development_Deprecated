using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace DataSetSpecification
{
    public class StructuredBuffer
    {
        private const string DllFilePath = @".\StructuredBufferDLL.dll";

        // Call the StructuredBuffer Constructor with no parameters
        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static IntPtr SBLibStructuredBuffer();

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static IntPtr SBLibStructuredBufferFromByteBuffer(IntPtr pByteBuffer, UInt32 SizeInBytes);

        // This calls the StructuredBuffer destructor
        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBlibDestructorStructuredBuffer(IntPtr poStructuredBuffer);

        // Add a string key-value pair to the StructuredBuffer 
        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibPutString(IntPtr poStructuredBuffer, IntPtr ElementName, IntPtr ElementValue);

        // Add a string key-value pair to the StructuredBuffer 
        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibPutBuffer(IntPtr poStructuredBuffer, IntPtr ElementName, UInt64[] array, UInt32 SizeInBytes);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibPutByte(IntPtr poStructuredBuffer, IntPtr key, Byte value);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibPutGuid(IntPtr poStructuredBuffer, IntPtr key, Byte[] guidRawPtr);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibPutStructuredBuffer(IntPtr poStructuredBuffer, IntPtr key, IntPtr poToPutStructuredBuffer);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibPutUnsignedInt64(IntPtr poStructuredBuffer, IntPtr key, UInt64 value);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibPutUnsignedInt32(IntPtr poStructuredBuffer, IntPtr key, UInt32 value);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibPutInt64(IntPtr poStructuredBuffer, IntPtr key, Int64 value);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibPutInt32(IntPtr poStructuredBuffer, IntPtr key, Int32 value);

        // Get a string value of the key from the StructuredBuffer 
        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static void SBLibGetString(IntPtr poStructuredBuffer, IntPtr ElementName, IntPtr ElementValue, int ElementValueLength);

        // Get a string length of value of the key from the StructuredBuffer 
        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static int SBLibGetStringLength(IntPtr poStructuredBuffer, IntPtr ElementName);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static int SBLibGetSerializedBufferRawDataSizeInBytes(IntPtr poStructuredBuffer);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static IntPtr SBLibGetSerializedBufferRawDataPtr(IntPtr poStructuredBuffer);

        [DllImport(DllFilePath, CallingConvention = CallingConvention.Cdecl)]
        private extern static UInt64 SBLibGet64BitHash(IntPtr poStructuredBuffer);

        public IntPtr m_poStructuredBuffer;

        // This the interface to the StructuredBuffer to replicate the C++ class behaviour
        public StructuredBuffer()
        {
            m_poStructuredBuffer = SBLibStructuredBuffer();
        }

        public StructuredBuffer(Byte[] structuredBufferByteArray, Int32 unSizeOfSerializedBuffer)
        {
            IntPtr unmanagedBuffer = Marshal.AllocHGlobal(unSizeOfSerializedBuffer);
            Marshal.Copy(structuredBufferByteArray, 0, unmanagedBuffer, unSizeOfSerializedBuffer);
            m_poStructuredBuffer = SBLibStructuredBufferFromByteBuffer(unmanagedBuffer, (uint)unSizeOfSerializedBuffer);
        }

        ~StructuredBuffer()
        {
            //SBlibDestructorStructuredBuffer(m_poStructuredBuffer);
        }

        public string GetString(string key)
        {
            int outputStringLength = SBLibGetStringLength(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key));
            IntPtr unmanagedBuffer = Marshal.AllocHGlobal(outputStringLength);
            SBLibGetString(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), unmanagedBuffer, outputStringLength+1);
            string value = Marshal.PtrToStringAnsi(unmanagedBuffer);
            Marshal.FreeHGlobal(unmanagedBuffer);
            return value;
        }

        public void PutString(string key, string value)
        {
            SBLibPutString(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), Marshal.StringToCoTaskMemAnsi(value));
        }

        public void PutUnsignedInt64(string key, UInt64 value)
        {
            SBLibPutUnsignedInt64(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), value);
        }

        public void PutUnsignedInt32(string key, UInt32 value)
        {
            SBLibPutUnsignedInt32(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), value);
        }

        public void PutByte(string key, Byte value)
        {
            SBLibPutByte(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), value);
        }

        public void PutGuid(string key, Guid value)
        {
            SBLibPutGuid(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), value.ToByteArray());
        }
        public void PutStructuredBuffer(string key, StructuredBuffer value)
        {
            SBLibPutStructuredBuffer(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), value.m_poStructuredBuffer);
        }

        public void PutInt64(string key, Int64 value)
        {
            SBLibPutInt64(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), value);
        }

        public void PutInt32(string key, Int32 value)
        {
            SBLibPutInt32(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), value);
        }

        public void PutUInt64Buffer(string key, UInt64[] array, uint unSizeInBytes)
        {
            SBLibPutBuffer(m_poStructuredBuffer, Marshal.StringToCoTaskMemAnsi(key), array, unSizeInBytes*8);
        }

        public int GetSerializedBufferRawDataSizeInBytes()
        {
            return SBLibGetSerializedBufferRawDataSizeInBytes(m_poStructuredBuffer);
        }

        public IntPtr GetSerializedBufferRawDataPtr()
        {
            return SBLibGetSerializedBufferRawDataPtr(m_poStructuredBuffer);
        }

        public UInt64 Get64BitHash()
        {
            return SBLibGet64BitHash(m_poStructuredBuffer);
        }
    }
}
