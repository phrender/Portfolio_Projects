using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Facade.Subsystem
{
    public class Stock : Equity
    {
        public Stock(string strTickerSymbol) : base(strTickerSymbol)
        {
        }

        public override void Buy(decimal shares)
        {
            // Always succeeds
        }

        public override void Sell(decimal shares)
        {
            throw new NotImplementedException();
        }

        public override decimal GetPrice()
        {
            return 10m; // All shares const $10
        }
    }
}
