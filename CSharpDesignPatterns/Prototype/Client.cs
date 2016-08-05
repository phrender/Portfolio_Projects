using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Prototype
{
    class Client
    {
        public Prototype CreateProduct(Prototype prototype)
        {
            Prototype product = prototype.Clone();
            
            return product;
        }
    }
}
