using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CSharpDesignPatterns.Abstract_Factory;
using CSharpDesignPatterns.Builder;
using CSharpDesignPatterns.Factory_Method;

namespace CSharpDesignPatterns
{
    class Program
    {
        static void Main(string[] args)
        {
            #region Abstract Factory pattern

            //ContinentFactory factory1 = new AfricaFactory();
            //Ecosystem ecosystem1 = new Ecosystem(factory1);
            //ecosystem1.Run();

            //ContinentFactory factory2 = new AustraliaFactory();
            //Ecosystem ecosystem2 = new Ecosystem(factory2);
            //ecosystem2.Run();

            #endregion

            #region Builder pattern

            //MenuDirector director = new MenuDirector();

            //MenuBuilder builder1 = new BurgerMenuBuilder();
            //director.Construct(builder1);
            //Menu menu1 = builder1.GetResult();
            //Console.WriteLine("Burger menu: {0}", menu1);

            //MenuBuilder builder2 = new KidsMenuBuilder();
            //director.Construct(builder2);
            //Menu menu2 = builder2.GetResult();
            //Console.WriteLine("Kids menu: {0}", menu2);

            //MenuBuilder builder3 = new SaladMenuBuilder();
            //director.Construct(builder3);
            //Menu menu3 = builder3.GetResult();
            //Console.WriteLine("Salad menu: {0}", menu3);

            #endregion

            #region Factory Method pattern

            //ToyCreator creator = null;

            //Console.WriteLine("Creating a firetruck toy.");
            //creator = new FiretruckCreator();
            //creator.MakeToy();
            //Console.WriteLine();

            //Console.WriteLine("Creating a superman toy.");
            //creator = new SupermanCreator();
            //creator.MakeToy();
            //Console.WriteLine();

            //Console.WriteLine("Creating a duck toy.");
            //creator = new DuckCreator();
            //creator.MakeToy();
            //Console.WriteLine();

            #endregion

        }
    }
}
