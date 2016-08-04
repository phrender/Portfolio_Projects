using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Factory_Method
{
    class FiretruckCreator : ToyCreator
    {
        protected override Toy CreateToy()
        {
            return new Firetruck();
        }
    }
}
