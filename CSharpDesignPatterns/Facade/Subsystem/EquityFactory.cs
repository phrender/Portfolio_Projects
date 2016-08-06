using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Facade.Subsystem
{
    public class EquityFactory
    {
        private static EquityFactory m_instance;

        public static EquityFactory GetInstance()
        {
            if (m_instance == null)
            {
                m_instance = new EquityFactory();
            }

            return m_instance;
        }

        public Equity CreateEquity(string strTickerSymbol)
        {
            return new Stock(strTickerSymbol);
        }
    }
}
