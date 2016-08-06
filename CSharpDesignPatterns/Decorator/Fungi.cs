using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Decorator
{
    public class Fungi : PizzaElement
    {
        public override decimal GetPrice()
        {
            return 10m;
        }
    }
}
