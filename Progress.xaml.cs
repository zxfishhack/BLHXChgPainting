using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;


namespace BLHXChgPainting
{
    /// <summary>
    /// Interaction logic for Progress.xaml
    /// </summary>
    public partial class Progress : Window
    {
        public Progress()
        {
            InitializeComponent();
        }

        private Task _exportTask = null;
        private int _counter = 0;
        private bool _isStop = false;

        private static Task RecursCreateDirectory(string inputBase, string outputBase)
        {
            return Task.Run(() =>
            {
                var dirInfo = new DirectoryInfo(inputBase);
                var subDirectories = dirInfo.GetDirectories();
                var taskes = new Task[subDirectories.Length];
                for (var i = 0; i < subDirectories.Length; i++)
                {
                    Directory.CreateDirectory(outputBase + "\\" + subDirectories[i].Name);
                    taskes[i] = RecursCreateDirectory(subDirectories[i].FullName, outputBase + "\\" + dirInfo.Name);
                }
                Task.WaitAll(taskes);
            });
        }

        private void ProcessFile(string input, string outputPrefix)
        {
            ExportProgress.Value++;
            var lst = AssetTool.GetTextureList(input);
            if (lst == IntPtr.Zero) return;
            var lstAnsi = Marshal.PtrToStringAnsi(lst);
            if (lstAnsi == null) return;
            var texLst = lstAnsi.Split(',').ToList();
            texLst.Remove("UISprite");
            foreach (var tex in texLst)
            {
                var size = 0;
                if (AssetTool.GetImageInfo(input, ref size, tex))
                {
                    var ptr = Marshal.AllocHGlobal(size);
                    if (AssetTool.LoadImageFromBundle(input, ptr, tex))
                    {
                        try
                        {
                            using (var outputFile = new FileStream(texLst.Count == 1 ? $"{outputPrefix}.png" : $"{outputPrefix}_{tex}.png", FileMode.Create))
                            {
                                var buf = new byte[size];
                                Marshal.Copy(ptr, buf, 0, size);
                                outputFile.Write(buf, 0, buf.Length);
                            }
                        }
                        catch
                        {
                            // ignored
                        }
                    }
                    Marshal.FreeHGlobal(ptr);
                }
            }
        }

        private void RecursProcess(string inputBase, string outputBase)
        {
            var dirInfo = new DirectoryInfo(inputBase);
            var info = dirInfo.GetFiles("*.*");
            foreach (var file in info)
            {
                if (_isStop) return;
                ProcessFile(file.FullName, file.FullName.Replace(inputBase, outputBase));
            }
            var subDirectories = dirInfo.GetDirectories();
            foreach (var dir in subDirectories)
            {
                if (_isStop) return;
                RecursProcess(dir.FullName, $"{outputBase}\\{dir.Name}");
            }
        }

        private Task CountFiles(string dir)
        {
            return Task.Run(() =>
            {
                var dirInfo = new DirectoryInfo(dir);
                var subDirectories = dirInfo.GetDirectories();
                var taskes = new Task[subDirectories.Length];
                for (var i = 0; i < subDirectories.Length; i++)
                {
                    taskes[i] = CountFiles(subDirectories[i].FullName);
                }
                var info = dirInfo.GetFiles("*.*");
                Interlocked.Add(ref _counter, info.Length);
                Task.WaitAll(taskes.ToArray());
            });
        }

        public void BeginExport(string inputBase, string outputBase)
        {
            _exportTask = Task.Run(() =>
            {
                var counters = CountFiles(inputBase);
                // 先建立所有文件夹
                var creators = RecursCreateDirectory(inputBase, outputBase);
                Task.WaitAll(counters, creators);
                ExportProgress.Total = _counter;
                // 开始处理任务
                RecursProcess(inputBase, outputBase);
                if (_isStop) return;
                MessageBox.Show("导出完成。", "提示");
                Dispatcher.Invoke(Close);
            });
        }

        public void CancelExport()
        {
            _isStop = true;
            _exportTask?.Wait();
        }

        private void Progress_OnClosing(object sender, CancelEventArgs e)
        {
            if (ExportProgress.Value == ExportProgress.Total) return;
            if (MessageBox.Show("确定要取消操作吗？", "提示", MessageBoxButton.YesNo, MessageBoxImage.Information) ==
                MessageBoxResult.No)
            {
                e.Cancel = true;
                return;
            }
            CancelExport();
        }

        private void ButtonBase_OnClick(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
