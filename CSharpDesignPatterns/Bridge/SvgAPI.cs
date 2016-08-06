using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Bridge
{
    public class SvgAPI : DrawAPI
    {
        public override void DrawLine(int iX1, int iY1, int iX2, int iY2)
        {
            Console.WriteLine(@"    <line x1={0} y1={1} x2={2} y2={3}/>", iX1, iY1, iX2, iY2);
        }
    }
}
