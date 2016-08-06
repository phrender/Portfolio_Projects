using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Decorator
{
    public class QuatroStagioni : PizzaElement
    {
        public override decimal GetPrice()
        {
            return 9m;
        }
    }
}
