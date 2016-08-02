using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Abstract_Factory
{
    class AustraliaFactory : ContinentFactory
    {
        public override Herbivore CreateHerbivore()
        {
            return new Kangaroo();
        }

        public override Carnivore CreateCarnivore()
        {
            return new Dingo();
        }
    }
}
