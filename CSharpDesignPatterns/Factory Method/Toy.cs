using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Factory_Method
{
    abstract class Toy
    {
        public abstract int GetPowderQuintity();
        public abstract TimeSpan GetHeatingTime();
        public abstract Color GetColor();
    }
}
