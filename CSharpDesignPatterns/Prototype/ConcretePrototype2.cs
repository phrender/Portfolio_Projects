using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Prototype
{
    class ConcretePrototype2 : Prototype
    {
        public override Prototype Clone()
        {
            Console.WriteLine("Created prototype2");
            return (Prototype)this.MemberwiseClone();
        }
    }
}
