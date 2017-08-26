using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Imaging;

namespace BLHXChgPainting
{
    class Vm : INotifyPropertyChanged
    {
        private string _hashesPath = "";
        private string _bundlePath = "";
        private string _pngFile = "";
        private BitmapImage _img = null;
        private BitmapImage _pngImg = null;
        private int _curSel = 0;

        public BitmapImage Img
        {
            get
            {
                return _img;
            }
            set
            {
                _img = value;
                OnPropertyChanged("Img");
                OnPropertyChanged("ImgDesc");
            }
        }

        public BitmapImage PngImg
        {
            get { return _pngImg; }
            set
            {
                _pngImg = value;
                OnPropertyChanged("PngImg");
                OnPropertyChanged("ImgDesc");
            }
        }

        public string ProcessTips { get; } = "注意，用以替换的png文件需自行处理透明通道，并且保证与原资源中的图片大小一致。";

        public string ImgDesc
        {
            get
            {
                string str = null;
                if (_img != null)
                {
                    str += $"原图片大小:{_img.PixelWidth}x{_img.PixelHeight}";
                }
                if (_pngImg != null)
                {
                    if (str != null)
                    {
                        str += ", ";
                    }
                    str += $"准备替换的图片大小:{_pngImg.PixelWidth}x{_pngImg.PixelHeight}";
                }
            return str;
            }
        }

        public string HashesPath
        {
            get { return _hashesPath; }
            set
            {
                _hashesPath = value;
                OnPropertyChanged("HashesPath");
            }
        }

        public string BundleFile
        {
            get { return _bundlePath; }
            set
            {
                _bundlePath = value;
                OnPropertyChanged("BundleFile");
            }
        }

        public string PngFile
        {
            get { return _pngFile; }
            set
            {
                _pngFile = value;
                OnPropertyChanged("PngFile");
            }
        }

        public int CurSel
        {
            get { return _curSel; }
            set
            {
                _curSel = value;
                OnPropertyChanged("CurSel");
            }
        }

        public ObservableCollection<string> TextureList { get; } = new ObservableCollection<string>();

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
