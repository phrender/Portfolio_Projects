using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Builder
{
    abstract class MenuBuilder
    {
        public abstract void BuildBurgerOrSalad();
        public abstract void BuildFries();
        public abstract void BuildDessert();
        public abstract void BuildDrink();
        public abstract void BuildToy();


        public abstract Menu GetResult();
    }
}
