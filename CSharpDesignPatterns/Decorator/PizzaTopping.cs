using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Decorator
{
    public abstract class PizzaTopping : PizzaElement
    {
        protected PizzaElement m_component;

        protected PizzaTopping(PizzaElement component)
        {
            m_component = component;
        }

        public override decimal GetPrice()
        {
            return m_component.GetPrice();
        }
    }
}
