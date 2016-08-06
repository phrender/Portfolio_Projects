using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Bridge
{
    public abstract class DrawAPI
    {
        public abstract void DrawLine(int iX1, int iY1, int iX2, int iY2);
    }
}
