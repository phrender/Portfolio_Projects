using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Factory_Method
{
    abstract class Toy
    {
        public abstract int GetPowderQuantity();
        public abstract TimeSpan GetHeatingTime();
        public abstract string GetColor();
    }
}
