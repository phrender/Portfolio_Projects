using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Bridge
{
    public class Square : Shape
    {
        public Square(DrawAPI implementor) : base(implementor)
        {
        }

        public override void Draw()
        {
            m_implementor.DrawLine(0, 0, 100, 0);
            m_implementor.DrawLine(100, 0, 100, 100);
            m_implementor.DrawLine(100, 100, 0, 100);
            m_implementor.DrawLine(0, 100, 0, 0);
        }
    }
}
