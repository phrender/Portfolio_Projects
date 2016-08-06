using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CSharpBehaviouralDesignPatterns.ChainOfResponsibility;

namespace CSharpBehaviouralDesignPatterns
{
    class Program
    {
        static void Main(string[] args)
        {
            #region Chain of Responsibility

            POSystem poSystem = new POSystem();

            poSystem.ProcessRequest(1000);
            poSystem.ProcessRequest(20000);
            poSystem.ProcessRequest(100000);
            poSystem.ProcessRequest(300000);

            #endregion
        }
    }
}
