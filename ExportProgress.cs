using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace BLHXChgPainting
{
    internal class ExportProgress : INotifyPropertyChanged
    {
        private int _total = 1;
        private int _value = 0;

        public int Total
        {
            get { return _total; }
            set
            {
                _total = value;
                OnPropertyChanged("Total");
                OnPropertyChanged("Verbose");
            }
        }

        public int Value
        {
            get { return _value; }
            set
            {
                _value = value;
                OnPropertyChanged("Value");
                OnPropertyChanged("Verbose");
            }
        }

        public string Verbose => $"{_value} / {_total}";

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
