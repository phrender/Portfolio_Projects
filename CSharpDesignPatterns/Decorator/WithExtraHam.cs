using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Decorator
{
    public class WithExtraHam : PizzaTopping
    {
        public WithExtraHam(PizzaElement component) : base(component)
        {
        }

        public override decimal GetPrice()
        {
            decimal basePrice = base.GetPrice();
            return basePrice + 1.50m;
        }
    }
}
