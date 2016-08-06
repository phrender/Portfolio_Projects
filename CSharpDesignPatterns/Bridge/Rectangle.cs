using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Bridge
{
    public class Rectangle : Shape
    {
        public Rectangle(DrawAPI implementor) : base(implementor)
        {
        }

        public override void Draw()
        {
            m_implementor.DrawLine(0, 0, 100, 0);
            m_implementor.DrawLine(100, 0, 100, 50);
            m_implementor.DrawLine(100, 50, 0, 50);
            m_implementor.DrawLine(0, 50, 0, 0);
        }
    }
}
