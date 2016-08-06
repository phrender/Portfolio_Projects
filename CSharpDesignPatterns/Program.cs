using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

// Pattern locations
using CSharpDesignPatterns.Abstract_Factory;
using CSharpDesignPatterns.Builder;
using CSharpDesignPatterns.Factory_Method;
using CSharpDesignPatterns.Prototype;
using CSharpDesignPatterns.Singelton;
using CSharpDesignPatterns.Adapter;
using CSharpDesignPatterns.Bridge;
using CSharpDesignPatterns.Composite;
using CSharpDesignPatterns.Decorator;
using CSharpDesignPatterns.Facade;

namespace CSharpDesignPatterns
{
    class Program
    {
        static void Main(string[] args)
        {
            /*
             * Samples using the different patterns included in this project.
             */

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

            #region Prototype pattern

            //Client client = new Client();

            //ConcretePrototype1 prototype1 = new ConcretePrototype1();
            //Prototype.Prototype product1 = client.CreateProduct(prototype1);

            //ConcretePrototype2 prototype2 = new ConcretePrototype2();
            //Prototype.Prototype product2 = client.CreateProduct(prototype2);

            #endregion

            #region Singelton pattern

            //Logger log = Logger.Instance;

            //log.Log(string.Format("Logging started at {0}", DateTime.Now));
            //log.Log("Hello logger!");
            //log.Log(string.Format("Logging ended at {0}", DateTime.Now));

            #endregion

            #region Adapter pattern

            //StockApp app = new StockApp();
            //app.ShowStockHistory("APPL");

            #endregion

            #region Bridge pattern

            //Console.WriteLine("Line drawn using OpenGL commands.");
            //Shape line = new Line(new OpenGLAPI());
            //line.Draw();
            //Console.WriteLine();

            //Console.WriteLine("Rectangle drawn using SVG commands.");
            //Shape rectangle = new Rectangle(new SvgAPI());
            //rectangle.Draw();
            //Console.WriteLine();

            //Console.WriteLine("Square drawn using OpenGL commands.");
            //Shape square = new Square(new OpenGLAPI());
            //square.Draw();
            //Console.WriteLine();

            #endregion

            #region Composite pattern

            //Html html = new Html();
            //Body body = new Body();
            //P p = new P();
            //Text text = new Text("Hello world\n");
            //p.AddHtmlChild(text);
            //body.AddHtmlChild(p);
            //html.AddHtmlChild(body);

            //Console.WriteLine(html.Render());

            #endregion

            #region Decorator

            //PizzaElement pizza = null;

            //pizza = new WithExtraHam(new Margarita());
            //decimal price = pizza.GetPrice();
            //Console.WriteLine("Margarita + ham: {0}", price);

            //pizza = new WithExtraHam(new WithExtraCheese(new Fungi()));
            //price = pizza.GetPrice();
            //Console.WriteLine("Fungi + ham, cheese: {0}", price);

            #endregion

            #region Facade

            MutualFund mutualFund = new MutualFund(12349876);
            mutualFund.Buy("GOOG", 50);

            #endregion

        }
    }
}
