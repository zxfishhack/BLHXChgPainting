using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace BLHXChgPainting
{
    class AssetTool
    {
        [DllImport("AssetsToolsWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetImageInfo(string fileName, ref int bufSize);

        [DllImport("AssetsToolsWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LoadImageFromBundle(string fileName, IntPtr image);

        [DllImport("AssetsToolsWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool ReplaceImageFile(string fileName, string pngFile);
    }
}
