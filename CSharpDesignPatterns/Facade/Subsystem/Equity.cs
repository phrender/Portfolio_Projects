using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Facade.Subsystem
{
    public abstract class Equity
    {

        private string m_strTickerSymbol = string.Empty;

        public Equity (string strTickerSymbol)
        {
            m_strTickerSymbol = strTickerSymbol;
        }

        public abstract void Buy (decimal shares);
        public abstract void Sell (decimal shares);
        public abstract decimal GetPrice();

    }
}
