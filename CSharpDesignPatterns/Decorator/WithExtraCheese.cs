using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Decorator
{
    class WithExtraCheese : PizzaTopping
    {
        public WithExtraCheese(PizzaElement component) : base(component)
        {
        }

        public override decimal GetPrice()
        {
            decimal basePrice = base.GetPrice();
            return basePrice + 1m;
        }
    }
}
