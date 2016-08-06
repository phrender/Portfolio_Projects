using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Abstract_Factory
{
    class AfricaFactory : ContinentFactory
    {
        public override Herbivore CreateHerbivore()
        {
            return new Zebra();
        }

        public override Carnivore CreateCarnivore()
        {
            return new Lion();
        }
    }
}
