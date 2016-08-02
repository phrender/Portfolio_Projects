using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CSharpDesignPatterns.Abstract_Factory;

namespace CSharpDesignPatterns
{
    class Program
    {
        static void Main(string[] args)
        {
            #region Abstract Factory pattern

            ContinentFactory factory1 = new AfricaFactory();
            Ecosystem ecosystem1 = new Ecosystem(factory1);
            ecosystem1.Run();

            ContinentFactory factory2 = new AustraliaFactory();
            Ecosystem ecosystem2 = new Ecosystem(factory2);
            ecosystem2.Run();

            #endregion

        }
    }
}
