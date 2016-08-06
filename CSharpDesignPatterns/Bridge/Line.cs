using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Bridge
{
    public class Line : Shape
    {
        public Line(DrawAPI implementor) : base(implementor)
        {
        }

        public override void Draw()
        {
            m_implementor.DrawLine(0, 0, 100, 100);
        }
    }
}
