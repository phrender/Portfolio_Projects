using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Abstract_Factory
{
    class Dingo : Carnivore
    {
        public override void Eat(Herbivore herbivore)
        {
            Console.WriteLine("{0} eats {1}", this.GetType().Name, herbivore.GetType().Name);
        }
    }
}
