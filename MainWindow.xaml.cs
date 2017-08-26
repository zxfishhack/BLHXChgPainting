using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;
using Microsoft.Win32;

namespace BLHXChgPainting
{
    public partial class MainWindow : Window
    {
        private bool loadList = false;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void SelectHashes(object sender, RoutedEventArgs e)
        {
            var op = new OpenFileDialog();
            op.Filter = "Hashes.csv|hashes.csv";
            op.Title = "打开Hashes.csv文件...";
            if (op.ShowDialog() == true)
            {
                Vm.HashesPath = op.FileName;
            }
        }

        private void SelectBundle(object sender, RoutedEventArgs e)
        {
            var op = new OpenFileDialog();
            op.Filter = "资源文件|*.*";
            op.Title = "打开painting目录下的资源文件...";
            if (op.ShowDialog() == true)
            {
                if (LoadImageFromBundle(op.FileName))
                {
                    MessageBox.Show("加载成功。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
                    Vm.BundleFile = op.FileName;
                }
            }
        }

        private void SelectPngFile(object sender, RoutedEventArgs e)
        {
            var op = new OpenFileDialog
            {
                Filter = "png文件|*.png",
                Title = "打开要替换的png文件..."
            };
            if (op.ShowDialog() != true) return;
            Vm.PngFile = op.FileName;
            var img = new BitmapImage(new Uri(Vm.PngFile));
            Vm.PngImg = img;
        }

        private readonly Random _random = new Random((int)DateTime.Now.ToBinary());

        private class HashesItem
        {
            public string AssetName { get; set; }
            public int AssetSize { get; set; }
            public string AssetMd5 { get; set; }
        }

        private List<HashesItem> ReadHashesCsv()
        {
            var lst = new List<HashesItem>();
            using (var stream = new StreamReader(Vm.HashesPath))
            {
                string line;
                while ((line = stream.ReadLine()) != null)
                {
                    var row = line.Split(',');
                    lst.Add(new HashesItem
                    {
                        AssetName = row[0],
                        AssetSize = int.Parse(row[1]),
                        AssetMd5 = row[2]
                    });
                }         
            }
            return lst;
        }

        private bool WriteHashesCsv(List<HashesItem> lst)
        {
            try
            {
                using (var stream = new StreamWriter(Vm.HashesPath))
                {
                    lst.ForEach(item =>
                    {
                        stream.WriteLine("{0},{1},{2}", item.AssetName, item.AssetSize, item.AssetMd5);
                    });
                }
                return true;
            }
            catch(Exception e)
            {
                MessageBox.Show(e.Message);
                return false;
            }
            
        }

        private static async Task<string> Md5Value(string fileName)
        {
            MD5 md5 = new MD5CryptoServiceProvider();
            if (!File.Exists(fileName))
                return string.Empty;
            var fs = new FileStream(fileName, FileMode.Open, FileAccess.Read);
            var md5Ch = await Task.Run(() => md5.ComputeHash(fs));
            fs.Close();
            md5.Clear();
            var strMd5 = md5Ch.Aggregate("", (current, b) => current + b.ToString("x").PadLeft(2, '0'));
            return strMd5.ToLower();
        }

        private async void Process(object sender, RoutedEventArgs e)
        {
#if (DEBUG)
            var debugOutput = "d:\\temp\\debug_output";
            File.Copy(Vm.BundleFile, debugOutput, true);
            if (!AssetTool.ReplaceImageFile(debugOutput, Vm.PngFile, Vm.TextureList[Vm.CurSel]))
            {
                MessageBox.Show("修改文件出错。", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            else
            {
                MessageBox.Show("修改文件成功。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            }
#else
            int idx;
            if (Vm.BundleFile == "" || Vm.HashesPath == "" || Vm.PngFile == "")
            {
                var errMsg = new List<string>
                {
                    "你在逗我？",
                    "搞笑吧？",
                    "作死？"
                };
                idx = _random.Next(0, errMsg.Count);
                MessageBox.Show(errMsg[idx], errMsg[idx], MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
            if (Vm.TextureList.Count == 0 || Vm.CurSel >= Vm.TextureList.Count)
            {
                return;
            }
            if (MessageBox.Show(Vm.ProcessTips, "继续吗?", MessageBoxButton.YesNo, MessageBoxImage.Warning) !=
                MessageBoxResult.Yes) return;
            var assetList = ReadHashesCsv();
            idx = assetList.FindIndex(item => Vm.BundleFile.EndsWith(item.AssetName.Replace("/", "\\")));
            if (idx < 0)
            {
                MessageBox.Show("请保持游戏原始目录结构。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }
            var backupError = false;
            try
            {
                File.Copy(Vm.BundleFile, Vm.BundleFile + ".bak", true);
                File.Copy(Vm.HashesPath, Vm.HashesPath + ".bak", true);
            }
            catch
            {
                backupError = true;
                if (MessageBox.Show("备份文件出错，是否继续？", "继续吗?", MessageBoxButton.YesNo, MessageBoxImage.Warning) !=
                    MessageBoxResult.Yes) return;
            }
            if (!AssetTool.ReplaceImageFile(Vm.BundleFile, Vm.PngFile, Vm.TextureList[Vm.CurSel]))
            {
                MessageBox.Show("修改文件出错。", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
                try
                {
                    File.Copy(Vm.BundleFile + ".bak", Vm.BundleFile, true);
                    File.Copy(Vm.HashesPath + ".bak", Vm.HashesPath, true);
                }
                catch
                {
                    MessageBox.Show("恢复备份文件出错，祝福你。", ">_<", MessageBoxButton.OK, MessageBoxImage.Warning);
                }
            }
            else
            {
                assetList[idx].AssetMd5 = await Md5Value(Vm.BundleFile);
                try
                {
                    using (var f = File.Open(Vm.BundleFile, FileMode.Open))
                    {
                        assetList[idx].AssetSize = (int) f.Length;
                    }
                }
                catch
                {
                    // ignored
                }
                if (WriteHashesCsv(assetList))
                {
                    MessageBox.Show("替换成功。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
                    return;
                }
                if (backupError)
                {
                    MessageBox.Show("修改Hashes.csv出错，请手工修改Hashes.csv或重试。", "错误", MessageBoxButton.OK,
                        MessageBoxImage.Error);
                }
                else
                {
                    if (MessageBox.Show("修改Hashes.csv出错，是否保留修改后的资源文件？（若保留，则需手工修改Hashes.csv）", "提示",
                            MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes) return;
                    //不保留
                    try
                    {
                        File.Copy(Vm.BundleFile + ".bak", Vm.BundleFile, true);
                        File.Copy(Vm.HashesPath + ".bak", Vm.HashesPath, true);
                    }
                    catch
                    {
                        MessageBox.Show("恢复备份文件出错，祝福你。", ">_<", MessageBoxButton.OK, MessageBoxImage.Warning);
                    }
                }
            }
#endif
        }

        private static byte[] GetImageFromBundle(string bundleFile, string textureName)
        {
            var bufSize = 0;
            if (!AssetTool.GetImageInfo(bundleFile, ref bufSize, textureName)) return null;
            var buf = Marshal.AllocHGlobal(bufSize);
            var image = new byte[bufSize];
            if (!AssetTool.LoadImageFromBundle(bundleFile, buf, textureName))
            {
                Marshal.FreeHGlobal(buf);
                return null;
            }
            Marshal.Copy(buf, image, 0, bufSize);
            return image;
        }

        private bool LoadImageFromBundle(string bundleFile, string textureFile)
        {
            var image = GetImageFromBundle(bundleFile, Vm.TextureList[Vm.CurSel]);
            if (image == null)
            {
                MessageBox.Show("加载图片失败。", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }
            try
            {
                var img = new BitmapImage();
                img.BeginInit();
                img.StreamSource = new MemoryStream(image);
                img.EndInit();
                Vm.Img = img;
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message, "发生错误", MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }
            return true;
        }

        private bool LoadImageFromBundle(string bundleFile)
        {
            loadList = true;
            Vm.TextureList.Clear();
            var lst = AssetTool.GetTextureList(bundleFile);
            if (lst == IntPtr.Zero)
            {
                MessageBox.Show("加载图片失败。", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }

            var texLst = Marshal.PtrToStringAnsi(lst);
            if (texLst == null)
            {
                MessageBox.Show("加载图片失败。", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }
            foreach (var s in texLst.Split(','))
            {
                Vm.TextureList.Add(s);
            }
            Vm.CurSel = 0;
            loadList = false;
            return LoadImageFromBundle(bundleFile, Vm.TextureList[Vm.CurSel]);
        }

        private void SaveOri(object sender, RoutedEventArgs re)
        {
            var image = GetImageFromBundle(Vm.BundleFile, Vm.TextureList[Vm.CurSel]);
            if (image == null)
            {
                MessageBox.Show("加载图片失败。", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
            var op = new SaveFileDialog
            {
                Filter = "png文件|*.png",
                Title = "保存图片..."
            };
            if (op.ShowDialog() != true) return;
            try
            {
                using (var f = new FileStream(op.FileName, FileMode.Create))
                {
                    f.Write(image, 0, image.Length);
                }
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message, "发生错误", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void SelctTexture(object sender, SelectionChangedEventArgs e)
        {
            if (Vm.CurSel < 0 || Vm.CurSel >= Vm.TextureList.Count || loadList) return;
            LoadImageFromBundle(Vm.BundleFile, Vm.TextureList[Vm.CurSel]);
        }
    }
}
