using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Abstract_Factory
{
    abstract class Carnivore
    {
        public abstract void Eat(Herbivore herbivore);
    }
}
