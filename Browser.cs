using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Media.Imaging;

namespace BLHXChgPainting
{
    internal class Node
    {
        public string Name { get; set; }
        public bool IsDir { get; set; }
    }

    internal class DirectoryNode : Node
    {
        public ObservableCollection<Node> Childrens { get; } = new ObservableCollection<Node>();
    }

    internal class FileNode : Node
    {
        public string FullName { get; set; }
    }

    class Browser : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private string _dirPath = "";
        private int _curSel = 0;
        private BitmapImage _img = null;

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

        public string DirPath
        {
            get { return _dirPath; }
            set
            {
                _dirPath = value;
                OnPropertyChanged("DirPath");
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

        public bool ListLoading { get; set; } = false;

        public ObservableCollection<string> TextureList { get; } = new ObservableCollection<string>();

        public ObservableCollection<Node> Root { get; } = new ObservableCollection<Node>();

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
