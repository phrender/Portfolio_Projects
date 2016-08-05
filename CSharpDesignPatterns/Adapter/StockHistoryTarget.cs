using System;
using System.Data;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Adapter
{
    public abstract class StockHistoryTarget
    {
        public abstract DataTable GetStockPrices(string strTicker);
    }
}
