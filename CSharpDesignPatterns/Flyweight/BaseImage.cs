using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Flyweight
{
    abstract class BaseImage
    {
        public abstract void Display(int iX, int iY, int iWidth, int iHeight);
    }
}
