using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Proxy
{
    public class BadClient
    {
        public void Execute()
        {
            WithRateLimit service = new WithRateLimit(new BitcoinAPI());

            for (int i = 0; i < 21; i++)
            {
                try
                {
                    Console.WriteLine(service.GetValueInUSD());
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }

                Thread.Sleep(100);
            }
        }
    }
}
