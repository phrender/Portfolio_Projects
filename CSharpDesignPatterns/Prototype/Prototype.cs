using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Prototype
{
    abstract class Prototype
    {
        public abstract Prototype Clone();
    }
}
