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
        public static extern IntPtr GetTextureList(string fileName);

        [DllImport("AssetsToolsWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetImageInfo(string fileName, ref int bufSize, string textureName);

        [DllImport("AssetsToolsWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LoadImageFromBundle(string fileName, IntPtr image, string textureName);

        [DllImport("AssetsToolsWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool ReplaceImageFile(string fileName, string pngFile, string textureName);
    }
}
