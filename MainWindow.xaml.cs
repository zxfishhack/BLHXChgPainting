using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media.Imaging;
using Microsoft.Win32;

namespace BLHXChgPainting
{
    public partial class MainWindow : Window
    {
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
            op.Filter = "资源文件|*_enc_tex";
            op.Title = "打开painting目录下的_enc_tex文件...";
            if (op.ShowDialog() == true)
            {
                Vm.BundleFile = op.FileName;
                LoadImageFromBundle();
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

        private void Process(object sender, RoutedEventArgs e)
        {
            if (MessageBox.Show(Vm.ProcessTips, "继续吗", MessageBoxButton.YesNo, MessageBoxImage.Warning) ==
                MessageBoxResult.Yes)
            {
                //TODO：处理BUDLE替换
            }
        }

        private void LoadImageFromBundle()
        {
            var bufSize = 0;
            if (!AssetTool.GetImageInfo(Vm.BundleFile, ref bufSize)) return;
            var buf = Marshal.AllocHGlobal(bufSize);
            var image = new byte[bufSize];
            if (!AssetTool.LoadImageFromBundle(Vm.BundleFile, buf)) return;
            Marshal.Copy(buf, image, 0, bufSize);
            var img = new BitmapImage();
            img.BeginInit();
            img.StreamSource = new MemoryStream(image);
            img.EndInit();
            Vm.Img = img;
        }

        private void SaveOri(object sender, RoutedEventArgs e)
        {
            var bufSize = 0;
            if (!AssetTool.GetImageInfo(Vm.BundleFile, ref bufSize)) return;
            var buf = Marshal.AllocHGlobal(bufSize);
            var image = new byte[bufSize];
            if (!AssetTool.LoadImageFromBundle(Vm.BundleFile, buf)) return;
            Marshal.Copy(buf, image, 0, bufSize);
            var op = new SaveFileDialog
            {
                Filter = "png文件|*.png",
                Title = "保存图片..."
            };
            if (op.ShowDialog() != true) return;
            var f = new FileStream(op.FileName, FileMode.Create);
            f.Write(image, 0, bufSize);
        }
    }
}
