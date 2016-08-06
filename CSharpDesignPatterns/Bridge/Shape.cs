using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Bridge
{
    public abstract class Shape
    {
        protected DrawAPI m_implementor;

        public Shape(DrawAPI implementor)
        {
            m_implementor = implementor;
        }

        public abstract void Draw();
    }
}
